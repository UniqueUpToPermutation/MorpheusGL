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
    Node makeBlitShader(Node parent, ref<Shader>* shaderOut, BlitShaderView* shaderViewOut);
    Node makeBlitGeometry(Node parent, ref<Geometry>* geoOut);
    Node makeBlitShader(NodeHandle parent, ref<Shader>* shaderOut, BlitShaderView* shaderViewOut);
    Node makeBlitGeometry(NodeHandle parent, ref<Geometry>* geoOut);
}