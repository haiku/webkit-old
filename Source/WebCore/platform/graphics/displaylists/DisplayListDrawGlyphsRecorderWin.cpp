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

#include "config.h"
#include "DisplayListDrawGlyphsRecorder.h"

#include "DisplayListItems.h"
#include "DisplayListRecorder.h"
#include "FloatPoint.h"
#include "Font.h"
#include "GlyphBuffer.h"

namespace WebCore {

namespace DisplayList {

DrawGlyphsRecorder::DrawGlyphsRecorder(Recorder& owner, DrawGlyphsDeconstruction)
    : m_owner(owner)
{
}

void DrawGlyphsRecorder::drawGlyphs(const Font& font, const GlyphBuffer& glyphBuffer, unsigned from, unsigned numGlyphs, const FloatPoint& startPoint, FontSmoothingMode smoothingMode)
{
    m_owner.append<DrawGlyphs>(font, glyphBuffer.glyphs(from), glyphBuffer.advances(from), numGlyphs, startPoint, smoothingMode);
}

} // namespace DisplayList

} // namespace WebCore
