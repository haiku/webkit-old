/*
 * Copyright (C) 2019 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(GRAPHICS_CONTEXT_GL) && USE(ANGLE)
#include "ExtensionsGLANGLE.h"

#include "GraphicsContextGLOpenGL.h"

#include <ANGLE/entry_points_gles_2_0_autogen.h>
#include <ANGLE/entry_points_gles_3_0_autogen.h>
// Skip the inclusion of ANGLE's explicit context entry points for now.
#define GL_ANGLE_explicit_context
#define GL_ANGLE_explicit_context_gles1
typedef void* GLeglContext;
#include <ANGLE/entry_points_gles_ext_autogen.h>

// Note: this file can't be compiled in the same unified source file
// as others which include the system's OpenGL headers.

namespace WebCore {

ExtensionsGLANGLE::ExtensionsGLANGLE(GraphicsContextGLOpenGL* context, bool useIndexedGetString)
    : m_initializedAvailableExtensions(false)
    , m_context(context)
    , m_isNVIDIA(false)
    , m_isAMD(false)
    , m_isIntel(false)
    , m_isImagination(false)
    , m_requiresBuiltInFunctionEmulation(false)
    , m_requiresRestrictedMaximumTextureSize(false)
    , m_useIndexedGetString(useIndexedGetString)
{
    // FIXME: ideally, remove this initialization altogether. ANGLE
    // subsumes the responsibility for graphics driver workarounds.
    m_vendor = String(reinterpret_cast<const char*>(gl::GetString(GL_VENDOR)));
    m_renderer = String(reinterpret_cast<const char*>(gl::GetString(GL_RENDERER)));

    Vector<String> vendorComponents = m_vendor.convertToASCIILowercase().split(' ');
    if (vendorComponents.contains("nvidia"))
        m_isNVIDIA = true;
    if (vendorComponents.contains("ati") || vendorComponents.contains("amd"))
        m_isAMD = true;
    if (vendorComponents.contains("intel"))
        m_isIntel = true;
    if (vendorComponents.contains("imagination"))
        m_isImagination = true;
}

ExtensionsGLANGLE::~ExtensionsGLANGLE() = default;

bool ExtensionsGLANGLE::supports(const String& name)
{
    if (!m_initializedAvailableExtensions) {
        if (!m_context->makeContextCurrent())
            return false;
        initializeAvailableExtensions();
    }
    return supportsExtension(name);
}

void ExtensionsGLANGLE::ensureEnabled(const String& name)
{
    // Enable support in ANGLE (if not enabled already).
    if (m_requestableExtensions.contains(name) && !m_enabledExtensions.contains(name)) {
        if (!m_context->makeContextCurrent())
            return;
        gl::RequestExtensionANGLE(name.ascii().data());
        m_enabledExtensions.add(name);

        if (name == "GL_CHROMIUM_color_buffer_float_rgba"_s)
            m_webglColorBufferFloatRGBA = true;
        else if (name == "GL_CHROMIUM_color_buffer_float_rgb"_s)
            m_webglColorBufferFloatRGB = true;
    }
}

bool ExtensionsGLANGLE::isEnabled(const String& name)
{
    return m_availableExtensions.contains(name) || m_enabledExtensions.contains(name);
}

int ExtensionsGLANGLE::getGraphicsResetStatusARB()
{
    return GraphicsContextGL::NO_ERROR;
}

String ExtensionsGLANGLE::getTranslatedShaderSourceANGLE(PlatformGLObject shader)
{
    if (!m_context->makeContextCurrent())
        return String();

    int sourceLength = m_context->getShaderi(shader, GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE);
    if (!sourceLength)
        return emptyString();
    Vector<GLchar> name(sourceLength); // GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE includes null termination.
    int returnedLength = 0;
    gl::GetTranslatedShaderSourceANGLE(shader, sourceLength, &returnedLength, name.data());
    if (!returnedLength)
        return emptyString();
    // returnedLength does not include the null terminator.
    ASSERT(returnedLength == sourceLength - 1);
    return String(name.data(), returnedLength);
}

void ExtensionsGLANGLE::initializeAvailableExtensions()
{
    if (m_useIndexedGetString) {
        GLint numExtensions = 0;
        gl::GetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        for (GLint i = 0; i < numExtensions; ++i)
            m_availableExtensions.add(gl::GetStringi(GL_EXTENSIONS, i));
        gl::GetIntegerv(GL_NUM_REQUESTABLE_EXTENSIONS_ANGLE, &numExtensions);
        for (GLint i = 0; i < numExtensions; ++i)
            m_requestableExtensions.add(gl::GetStringi(GL_REQUESTABLE_EXTENSIONS_ANGLE, i));
    } else {
        String extensionsString = getExtensions();
        for (auto& extension : extensionsString.split(' '))
            m_availableExtensions.add(extension);
        extensionsString = String(reinterpret_cast<const char*>(gl::GetString(GL_REQUESTABLE_EXTENSIONS_ANGLE)));
        for (auto& extension : extensionsString.split(' '))
            m_requestableExtensions.add(extension);
    }
    m_initializedAvailableExtensions = true;
}

void ExtensionsGLANGLE::blitFramebuffer(long srcX0, long srcY0, long srcX1, long srcY1, long dstX0, long dstY0, long dstX1, long dstY1, unsigned long mask, unsigned long filter)
{
    // FIXME: consider adding support for APPLE_framebuffer_multisample.
    if (!m_context->makeContextCurrent())
        return;

    gl::BlitFramebufferANGLE(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void ExtensionsGLANGLE::renderbufferStorageMultisample(unsigned long target, unsigned long samples, unsigned long internalformat, unsigned long width, unsigned long height)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::RenderbufferStorageMultisampleANGLE(target, samples, internalformat, width, height);
}

PlatformGLObject ExtensionsGLANGLE::createVertexArrayOES()
{
    if (!m_context->makeContextCurrent())
        return 0;

    GLuint array = 0;
    gl::GenVertexArraysOES(1, &array);
    return array;
}

void ExtensionsGLANGLE::deleteVertexArrayOES(PlatformGLObject array)
{
    if (!array)
        return;

    if (!m_context->makeContextCurrent())
        return;

    gl::DeleteVertexArraysOES(1, &array);
}

GCGLboolean ExtensionsGLANGLE::isVertexArrayOES(PlatformGLObject array)
{
    if (!array)
        return GL_FALSE;

    if (!m_context->makeContextCurrent())
        return GL_FALSE;

    return gl::IsVertexArrayOES(array);
}

void ExtensionsGLANGLE::bindVertexArrayOES(PlatformGLObject array)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::BindVertexArrayOES(array);
}

void ExtensionsGLANGLE::insertEventMarkerEXT(const String&)
{
    // FIXME: implement this function and add GL_EXT_debug_marker in supports().
    return;
}

void ExtensionsGLANGLE::pushGroupMarkerEXT(const String&)
{
    // FIXME: implement this function and add GL_EXT_debug_marker in supports().
    return;
}

void ExtensionsGLANGLE::popGroupMarkerEXT(void)
{
    // FIXME: implement this function and add GL_EXT_debug_marker in supports().
    return;
}

bool ExtensionsGLANGLE::supportsExtension(const String& name)
{
    return m_availableExtensions.contains(name) || m_requestableExtensions.contains(name);
}

void ExtensionsGLANGLE::drawBuffersEXT(GCGLSpan<const GCGLenum> bufs)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::DrawBuffersEXT(bufs.bufSize, bufs.data);
}

void ExtensionsGLANGLE::drawArraysInstanced(GCGLenum mode, GCGLint first, GCGLsizei count, GCGLsizei primcount)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::DrawArraysInstancedANGLE(mode, first, count, primcount);
}

void ExtensionsGLANGLE::drawElementsInstanced(GCGLenum mode, GCGLsizei count, GCGLenum type, long long offset, GCGLsizei primcount)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::DrawElementsInstancedANGLE(mode, count, type, reinterpret_cast<GLvoid*>(static_cast<intptr_t>(offset)), primcount);
}

void ExtensionsGLANGLE::vertexAttribDivisor(GCGLuint index, GCGLuint divisor)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::VertexAttribDivisorANGLE(index, divisor);
}

String ExtensionsGLANGLE::getExtensions()
{
    return String(reinterpret_cast<const char*>(gl::GetString(GL_EXTENSIONS)));
}

GCGLenum ExtensionsGLANGLE::adjustWebGL1TextureInternalFormat(GCGLenum internalformat, GCGLenum format, GCGLenum type)
{
    // The implementation of WEBGL_color_buffer_float for WebGL 1.0 / ES 2.0 requires a sized
    // internal format. Adjust it if necessary at this lowest level.
    if (type == GL_FLOAT) {
        if (m_webglColorBufferFloatRGBA && format == GL_RGBA && internalformat == GL_RGBA)
            return GL_RGBA32F;
        if (m_webglColorBufferFloatRGB && format == GL_RGB && internalformat == GL_RGB)
            return GL_RGB32F;
    }
    return internalformat;
}

// GL_ANGLE_robust_client_memory
void ExtensionsGLANGLE::readPixelsRobustANGLE(int x, int y, GCGLsizei width, GCGLsizei height, GCGLenum format, GCGLenum type, GCGLsizei bufSize, GCGLsizei *length, GCGLsizei *columns, GCGLsizei *rows, void *pixels)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::ReadPixelsRobustANGLE(x, y, width, height, format, type, bufSize, length, columns, rows, pixels);
}

void ExtensionsGLANGLE::texParameterfvRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, const GCGLfloat *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::TexParameterfvRobustANGLE(target, pname, bufSize, params);
}

void ExtensionsGLANGLE::texParameterivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, const GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::TexParameterivRobustANGLE(target, pname, bufSize, params);
}

void ExtensionsGLANGLE::getQueryivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetQueryivRobustANGLE(target, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getQueryObjectuivRobustANGLE(GCGLuint id, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetQueryObjectuivRobustANGLE(id, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getBufferPointervRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, void **params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetBufferPointervRobustANGLE(target, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getInternalformativRobustANGLE(GCGLenum target, GCGLenum internalformat, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetInternalformativRobustANGLE(target, internalformat, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getVertexAttribIivRobustANGLE(GCGLuint index, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetVertexAttribIivRobustANGLE(index, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getVertexAttribIuivRobustANGLE(GCGLuint index, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetVertexAttribIuivRobustANGLE(index, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getUniformuivRobustANGLE(GCGLuint program, int location, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetUniformuivRobustANGLE(program, location, bufSize, length, params);
}

void ExtensionsGLANGLE::getBufferParameteri64vRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint64 *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetBufferParameteri64vRobustANGLE(target, pname, bufSize, length, reinterpret_cast<GLint64*>(params));
}

void ExtensionsGLANGLE::samplerParameterivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, const GCGLint *param)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::SamplerParameterivRobustANGLE(sampler, pname, bufSize, param);
}

void ExtensionsGLANGLE::samplerParameterfvRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, const GCGLfloat *param)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::SamplerParameterfvRobustANGLE(sampler, pname, bufSize, param);
}

void ExtensionsGLANGLE::getSamplerParameterivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetSamplerParameterivRobustANGLE(sampler, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getSamplerParameterfvRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLfloat *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetSamplerParameterfvRobustANGLE(sampler, pname, bufSize, length, params);
}


void ExtensionsGLANGLE::getFramebufferParameterivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetFramebufferParameterivRobustANGLE(target, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getProgramInterfaceivRobustANGLE(GCGLuint program, GCGLenum programInterface, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetProgramInterfaceivRobustANGLE(program, programInterface, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getBooleani_vRobustANGLE(GCGLenum target, GCGLuint index, GCGLsizei bufSize, GCGLsizei *length, GCGLboolean *data)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetBooleani_vRobustANGLE(target, index, bufSize, length, data);
}

void ExtensionsGLANGLE::getMultisamplefvRobustANGLE(GCGLenum pname, GCGLuint index, GCGLsizei bufSize, GCGLsizei *length, GCGLfloat *val)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetMultisamplefvRobustANGLE(pname, index, bufSize, length, val);
}

void ExtensionsGLANGLE::getTexLevelParameterivRobustANGLE(GCGLenum target, int level, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetTexLevelParameterivRobustANGLE(target, level, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getTexLevelParameterfvRobustANGLE(GCGLenum target, int level, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLfloat *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetTexLevelParameterfvRobustANGLE(target, level, pname, bufSize, length, params);
}


void ExtensionsGLANGLE::getPointervRobustANGLERobustANGLE(GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, void **params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetPointervRobustANGLERobustANGLE(pname, bufSize, length, params);
}

#if PLATFORM(MAC) || PLATFORM(IOS_FAMILY)
static void wipeAlphaChannelFromPixels(int width, int height, unsigned char* pixels)
{
    // We can assume this doesn't overflow because the calling functions
    // use checked arithmetic.
    int totalBytes = width * height * 4;
    for (int i = 0; i < totalBytes; i += 4)
        pixels[i + 3] = 255;
}
#endif

void ExtensionsGLANGLE::readnPixelsRobustANGLE(int x, int y, GCGLsizei width, GCGLsizei height, GCGLenum format, GCGLenum type, GCGLsizei bufSize, GCGLsizei *length, GCGLsizei *columns, GCGLsizei *rows, void *data, bool readingToPixelBufferObject)
{
    if (!m_context->makeContextCurrent())
        return;

    // FIXME: remove the two glFlush calls when the driver bug is fixed, i.e.,
    // all previous rendering calls should be done before reading pixels.
    gl::Flush();
    auto attrs = m_context->contextAttributes();
    GCGLenum framebufferTarget = m_context->m_isForWebGL2 ? GraphicsContextGL::READ_FRAMEBUFFER : GraphicsContextGL::FRAMEBUFFER;
    const GraphicsContextGLOpenGL::GraphicsContextGLState& state = m_context->m_state;
    if (attrs.antialias && state.boundReadFBO == m_context->m_multisampleFBO) {
        m_context->resolveMultisamplingIfNecessary(IntRect(x, y, width, height));
        gl::BindFramebuffer(framebufferTarget, m_context->m_fbo);
        gl::Flush();
    }
    (void) m_context->moveErrorsToSyntheticErrorList();
    gl::ReadnPixelsRobustANGLE(x, y, width, height, format, type, bufSize, length, columns, rows, data);
    GLenum error = gl::GetError();
    if (attrs.antialias && state.boundReadFBO == m_context->m_multisampleFBO)
        gl::BindFramebuffer(framebufferTarget, m_context->m_multisampleFBO);

    if (error) {
        // ANGLE detected a failure during the ReadnPixelsRobustANGLE operation. Surface this in the
        // synthetic error list, and skip the alpha channel fixup below.
        m_context->synthesizeGLError(error);
        return;
    }

#if PLATFORM(MAC) || PLATFORM(IOS_FAMILY)
    if (!readingToPixelBufferObject && !attrs.alpha && (format == GraphicsContextGL::RGBA || format == GraphicsContextGL::BGRA) && (type == GraphicsContextGL::UNSIGNED_BYTE) && (state.boundReadFBO == m_context->m_fbo || (attrs.antialias && state.boundReadFBO == m_context->m_multisampleFBO)))
        wipeAlphaChannelFromPixels(width, height, static_cast<unsigned char*>(data));
#else
    UNUSED_PARAM(readingToPixelBufferObject);
#endif
}

void ExtensionsGLANGLE::getnUniformfvRobustANGLE(GCGLuint program, int location, GCGLsizei bufSize, GCGLsizei *length, GCGLfloat *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetnUniformfvRobustANGLE(program, location, bufSize, length, params);
}

void ExtensionsGLANGLE::getnUniformivRobustANGLE(GCGLuint program, int location, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetnUniformivRobustANGLE(program, location, bufSize, length, params);
}

void ExtensionsGLANGLE::getnUniformuivRobustANGLE(GCGLuint program, int location, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetnUniformuivRobustANGLE(program, location, bufSize, length, params);
}

void ExtensionsGLANGLE::texParameterIivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, const GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::TexParameterIivRobustANGLE(target, pname, bufSize, params);
}

void ExtensionsGLANGLE::texParameterIuivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, const GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::TexParameterIuivRobustANGLE(target, pname, bufSize, params);
}

void ExtensionsGLANGLE::getTexParameterIivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetTexParameterIivRobustANGLE(target, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getTexParameterIuivRobustANGLE(GCGLenum target, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetTexParameterIuivRobustANGLE(target, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::samplerParameterIivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, const GCGLint *param)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::SamplerParameterIivRobustANGLE(sampler, pname, bufSize, param);
}

void ExtensionsGLANGLE::samplerParameterIuivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, const GCGLuint *param)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::SamplerParameterIuivRobustANGLE(sampler, pname, bufSize, param);
}

void ExtensionsGLANGLE::getSamplerParameterIivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetSamplerParameterIivRobustANGLE(sampler, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getSamplerParameterIuivRobustANGLE(GCGLuint sampler, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLuint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetSamplerParameterIuivRobustANGLE(sampler, pname, bufSize, length, params);
}


void ExtensionsGLANGLE::getQueryObjectivRobustANGLE(GCGLuint id, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetQueryObjectivRobustANGLE(id, pname, bufSize, length, params);
}

void ExtensionsGLANGLE::getQueryObjecti64vRobustANGLE(GCGLuint id, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLint64 *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetQueryObjecti64vRobustANGLE(id, pname, bufSize, length, reinterpret_cast<GLint64*>(params));
}

void ExtensionsGLANGLE::getQueryObjectui64vRobustANGLE(GCGLuint id, GCGLenum pname, GCGLsizei bufSize, GCGLsizei *length, GCGLuint64 *params)
{
    if (!m_context->makeContextCurrent())
        return;

    gl::GetQueryObjectui64vRobustANGLE(id, pname, bufSize, length, reinterpret_cast<GLuint64*>(params));
}


} // namespace WebCore

#endif // ENABLE(GRAPHICS_CONTEXT_GL) && USE(ANGLE)
