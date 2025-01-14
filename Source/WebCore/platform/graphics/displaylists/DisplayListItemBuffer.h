/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "DisplayListItemType.h"
#include "SharedBuffer.h"
#include <wtf/FastMalloc.h>
#include <wtf/ObjectIdentifier.h>

namespace WebCore {

class GraphicsContext;

namespace DisplayList {

class DisplayList;

enum ItemBufferIdentifierType { };
using ItemBufferIdentifier = ObjectIdentifier<ItemBufferIdentifierType>;

// An ItemBufferHandle wraps a pointer to a buffer that contains display list item data.
struct ItemBufferHandle {
    ItemBufferIdentifier identifier;
    uint8_t* data { nullptr };
    size_t capacity { 0 };

    operator bool() const { return !!(*this); }
    bool operator!() const { return !data; }
};

using ItemBufferHandles = Vector<ItemBufferHandle, 2>;

// An ItemHandle wraps a pointer to an ItemType followed immediately by an item of that type.
// Each item handle data pointer is aligned to 8 bytes, and the item itself is also aligned
// to 8 bytes. To ensure this, the item type consists of 8 bytes (1 byte for the type and 7
// bytes of padding).
struct ItemHandle {
    uint8_t* data { nullptr };

    void apply(GraphicsContext&);
    WEBCORE_EXPORT void destroy();
    bool isDrawingItem() const { return WebCore::DisplayList::isDrawingItem(type()); }

    operator bool() const { return !!(*this); }
    bool operator!() const { return !data; }

#if !defined(NDEBUG) || !LOG_DISABLED
    CString description() const;
#endif

    ItemType type() const { return static_cast<ItemType>(data[0]); }

    template<typename T> bool is() const { return type() == T::itemType; }
    template<typename T> T& get() const
    {
        ASSERT(is<T>());
        return *reinterpret_cast<T*>(&data[sizeof(uint64_t)]);
    }

    void copyTo(ItemHandle destination) const;
};

class ItemBufferWritingClient {
public:
    virtual ~ItemBufferWritingClient() { }

    virtual ItemBufferHandle createItemBuffer(size_t) = 0;
    virtual RefPtr<SharedBuffer> encodeItem(ItemHandle) const = 0;
};

class ItemBufferReadingClient {
public:
    virtual ~ItemBufferReadingClient() { }

    virtual Optional<ItemHandle> WARN_UNUSED_RETURN decodeItem(const uint8_t* data, size_t dataLength, ItemType, uint8_t* handleLocation) = 0;
};

// An ItemBuffer contains display list item data, and consists of a readwrite ItemBufferHandle (to which display
// list items are appended) as well as a number of readonly ItemBufferHandles. Items are appended to the readwrite
// buffer until all available capacity is exhausted, at which point we will move this writable handle to the list
// of readonly handles.
class ItemBuffer {
    WTF_MAKE_NONCOPYABLE(ItemBuffer); WTF_MAKE_FAST_ALLOCATED;
public:
    friend class DisplayList;

    WEBCORE_EXPORT ItemBuffer();
    WEBCORE_EXPORT ~ItemBuffer();
    ItemBuffer(ItemBufferHandles&&);

    WEBCORE_EXPORT ItemBuffer(ItemBuffer&&);
    WEBCORE_EXPORT ItemBuffer& operator=(ItemBuffer&&);

    size_t sizeInBytes() const
    {
        size_t result = 0;
        for (auto buffer : m_readOnlyBuffers)
            result += buffer.capacity;
        result += m_writtenNumberOfBytes;
        return result;
    }

    void clear();

    bool isEmpty() const
    {
        return !m_writtenNumberOfBytes && m_readOnlyBuffers.isEmpty();
    }

    WEBCORE_EXPORT ItemBufferHandle createItemBuffer(size_t);

    // ItemBuffer::append<T>(...) appends a display list item of type T to the end of the current
    // writable buffer handle; if remaining buffer capacity is insufficient to store the item, a
    // new buffer will be allocated (either by the ItemBufferWritingClient, if set, or by the item
    // buffer itself if there is no client). Items are placed back-to-back in these item buffers,
    // with padding after each item to ensure that all items are aligned to 8 bytes.
    //
    // If a writing client is present and requires custom encoding for the given item type T, the
    // item buffer will ask the client for an opaque SharedBuffer containing encoded data for the
    // item. This encoded data is then appended to the item buffer, with padding to ensure that
    // the start and end of this data are aligned to 8 bytes, if necessary. When consuming encoded
    // item data, a corresponding ItemBufferReadingClient will be required to convert this encoded
    // data back into an item of type T.
    template<typename T, class... Args> void append(Args&&... args)
    {
        static_assert(std::is_trivially_destructible<T>::value || !T::isInlineItem);

        if (!T::isInlineItem && m_writingClient) {
            static uint8_t temporaryItemBuffer[sizeof(uint64_t) + sizeof(T)];
            temporaryItemBuffer[0] = static_cast<uint8_t>(T::itemType);
            new (temporaryItemBuffer + sizeof(uint64_t)) T(std::forward<Args>(args)...);
            appendEncodedData({ temporaryItemBuffer });
            ItemHandle { temporaryItemBuffer }.destroy();
            return;
        }

        swapWritableBufferIfNeeded(paddedSizeOfTypeAndItemInBytes(T::itemType));

        if (!std::is_trivially_destructible<T>::value)
            m_itemsToDestroyInAllocatedBuffers.append({ &m_writableBuffer.data[m_writtenNumberOfBytes] });

        uncheckedAppend<T>(std::forward<Args>(args)...);
    }

    void setClient(ItemBufferWritingClient* client) { m_writingClient = client; }
    void setClient(ItemBufferReadingClient* client) { m_readingClient = client; }

private:
    const ItemBufferHandles& readOnlyBuffers() const { return m_readOnlyBuffers; }
    void forEachItemBuffer(Function<void(const ItemBufferHandle&)>&&) const;

    WEBCORE_EXPORT void swapWritableBufferIfNeeded(size_t numberOfBytes);
    WEBCORE_EXPORT void appendEncodedData(ItemHandle);
    template<typename T, class... Args> void uncheckedAppend(Args&&... args)
    {
        auto* startOfItem = &m_writableBuffer.data[m_writtenNumberOfBytes];
        startOfItem[0] = static_cast<uint8_t>(T::itemType);
        new (startOfItem + sizeof(uint64_t)) T(std::forward<Args>(args)...);
        m_writtenNumberOfBytes += paddedSizeOfTypeAndItemInBytes(T::itemType);
    }

    ItemBufferReadingClient* m_readingClient { nullptr };
    ItemBufferWritingClient* m_writingClient { nullptr };
    Vector<ItemHandle> m_itemsToDestroyInAllocatedBuffers;
    Vector<uint8_t*> m_allocatedBuffers;
    ItemBufferHandles m_readOnlyBuffers;
    ItemBufferHandle m_writableBuffer;
    size_t m_writtenNumberOfBytes { 0 };
};

} // namespace DisplayList
} // namespace WebCore
