/*
 * Copyright (C) 2014-2015 Apple Inc. All rights reserved.
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

#if ENABLE(CSS_SCROLL_SNAP)

#include "LayoutRect.h"
#include "LayoutUnit.h"
#include "ScrollSnapOffsetsInfo.h"
#include "ScrollTypes.h"
#include <wtf/Forward.h>

namespace WebCore {

class HTMLElement;
class RenderBox;
class RenderStyle;
class ScrollableArea;

// Update the snap offsets for this scrollable area, given the RenderBox of the scroll container, the RenderStyle
// which defines the scroll-snap properties, and the viewport rectangle with the origin at the top left of
// the scrolling container's border box.
void updateSnapOffsetsForScrollableArea(ScrollableArea&, const RenderBox& scrollingElementBox, const RenderStyle& scrollingElementStyle, LayoutRect viewportRectInBorderBoxCoordinates);

const unsigned invalidSnapOffsetIndex = UINT_MAX;
WEBCORE_EXPORT LayoutUnit closestSnapOffset(const Vector<LayoutUnit>& snapOffsets, const Vector<ScrollOffsetRange<LayoutUnit>>& snapOffsetRanges, LayoutUnit scrollDestination, float velocity, unsigned& activeSnapIndex);
WEBCORE_EXPORT float closestSnapOffset(const Vector<float>& snapOffsets, const Vector<ScrollOffsetRange<float>>& snapOffsetRanges, float scrollDestination, float velocity, unsigned& activeSnapIndex);

} // namespace WebCore

#endif // ENABLE(CSS_SCROLL_SNAP)
