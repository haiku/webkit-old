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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEB_RTC)

#include "JSDOMPromiseDeferred.h"

namespace WebCore {

class CryptoKey;
class RTCRtpSFrameTransformer;
class RTCRtpTransformBackend;

class RTCRtpSFrameTransform : public RefCounted<RTCRtpSFrameTransform> {
public:
    static Ref<RTCRtpSFrameTransform> create() { return adoptRef(*new RTCRtpSFrameTransform); }
    ~RTCRtpSFrameTransform();

    void setEncryptionKey(CryptoKey&, Optional<uint64_t>, DOMPromiseDeferred<void>&&);

    bool isAttached() const { return m_isAttached; }
    void initializeBackendForReceiver(RTCRtpTransformBackend&);
    void initializeBackendForSender(RTCRtpTransformBackend&);
    void willClearBackend(RTCRtpTransformBackend&);

    WEBCORE_EXPORT uint64_t counterForTesting() const;

private:
    RTCRtpSFrameTransform();

    enum class Side { Sender, Receiver };
    void initializeTransformer(RTCRtpTransformBackend&, Side);

    bool m_isAttached { false };
    Ref<RTCRtpSFrameTransformer> m_transformer;
};

} // namespace WebCore

#endif // ENABLE(WEB_RTC)
