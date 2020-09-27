#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>

#include <glad/glad.h>

namespace Morpheus {
    struct BlitShaderView {
        ShaderUniform<glm::vec2> mLower;
        ShaderUniform<glm::vec2> mUpper;
        ShaderUniform<Sampler> mBlitTexture;
    };

    GLint makeBlitShaderRaw();
    Shader* makeBlitShader(INodeOwner* parent, BlitShaderView* shaderViewOut);
   	Geometry* makeBlitGeometry(INodeOwner* parent);
}