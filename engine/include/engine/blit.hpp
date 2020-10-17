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

    Shader* makeBlitShader(INodeOwner* parent, BlitShaderView* shaderViewOut);
	Shader* makeBlitShader(INodeOwner* parent, const std::string& fragmentSrc, 
		BlitShaderView* shaderViewOut, GLSLPreprocessorConfig* overrides = nullptr);
   	Geometry* makeBlitGeometry(INodeOwner* parent);
}