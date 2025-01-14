/*
 * Copyright (C) 2020 Apple Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEB_RTC)

#include "RTCRtpTransformableFrame.h"

namespace WebCore {

class LibWebRTCRtpTransformableFrame final : public RTCRtpTransformableFrame {
    WTF_MAKE_FAST_ALLOCATED;
public:
    explicit LibWebRTCRtpTransformableFrame(std::unique_ptr<webrtc::TransformableFrameInterface>&&);
    virtual ~LibWebRTCRtpTransformableFrame() = default;

    static std::unique_ptr<webrtc::TransformableFrameInterface> toRTCFrame(LibWebRTCRtpTransformableFrame&&);

private:
    Data data() const final;
    void setData(Data data) final { m_rtcFrame->SetData({ data.data, data.size }); }

    std::unique_ptr<webrtc::TransformableFrameInterface> m_rtcFrame;
};

inline LibWebRTCRtpTransformableFrame::LibWebRTCRtpTransformableFrame(std::unique_ptr<webrtc::TransformableFrameInterface>&& frame)
    : m_rtcFrame(WTFMove(frame))
{
}

inline std::unique_ptr<webrtc::TransformableFrameInterface> LibWebRTCRtpTransformableFrame::toRTCFrame(LibWebRTCRtpTransformableFrame&& frame)
{
    return WTFMove(frame.m_rtcFrame);
}

inline RTCRtpTransformableFrame::Data LibWebRTCRtpTransformableFrame::data() const
{
    auto data = m_rtcFrame->GetData();
    return { data.begin(), data.size() };
}

} // namespace WebCore

#endif // ENABLE(WEB_RTC)
