#include <engine/blit.hpp>
#include <engine/shader.hpp>
#include <engine/geometry.hpp>

static const char blitShaderVert[] = 
R"(
    #version 410 core
    layout(location = 0) in vec2 position;
    uniform vec2 lower;
    uniform vec2 upper;
    out vec2 uv;
    void main() {
    vec2 transformed = lower + position * (upper - lower);
    gl_Position = vec4(transformed, 0.0, 1.0);
        uv = position;
    }
)";

static const char blitShaderFrag[] = 
R"(
    #version 410 core
    in vec2 uv;
    uniform sampler2D blitTexture;
    out vec4 outColor;
    void main() {
        outColor = vec4(texture(blitTexture, uv).rgb, 1.0);
    }
)";

namespace Morpheus {
    GLint makeBlitShaderRaw() {
        auto vShader = compileShader(blitShaderVert, ShaderType::VERTEX);
        auto fShader = compileShader(blitShaderFrag, ShaderType::FRAGMENT);

        auto program = glCreateProgram();
        glAttachShader(program, vShader);
        glAttachShader(program, fShader);
        glLinkProgram(program);
        printProgramLinkerOutput(program);
        GL_ASSERT;

        return program;
    }

    Node makeBlitShader(Node parent, ref<Shader>* shaderOut, BlitShaderView* shaderViewOut) {
        auto contentManager = content();
        auto shaderFactory = contentManager->getFactory<Shader>();

        ref<Shader> result = shaderFactory->makeFromGL(makeBlitShaderRaw());
        GL_ASSERT;

        shaderViewOut->mLower.find(result, "lower");
        shaderViewOut->mUpper.find(result, "upper");
        shaderViewOut->mBlitTexture.find(result, "blitTexture");

        if (shaderOut)
            *shaderOut = result;

        return contentManager->createContentNode(result, parent);
    }

    Node makeBlitShader(NodeHandle parent, ref<Shader>* shaderOut, BlitShaderView* shaderViewOut) {
        return makeBlitShader(graph()->find(parent), shaderOut, shaderViewOut);
    }

    Node makeBlitGeometry(NodeHandle parent, ref<Geometry>* geoOut) {
        return makeBlitGeometry(graph()->find(parent), geoOut);
    }

    Node makeBlitGeometry(Node parent, ref<Geometry>* geoOut) {
        auto contentManager = content();
        auto geoFactory = contentManager->getFactory<Geometry>();

        float verts[] = { 0.0f, 0.0f, 
            1.0f, 0.0f, 
            0.0f, 1.0f, 
            1.0f, 1.0f
        };

        unsigned short idx[] = { 0, 2, 1, 1, 2, 3 };

        GLuint vbo;
        glCreateBuffers(1, &vbo);
        GLuint ibo;
        glCreateBuffers(1, &ibo);

        GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        GL_ASSERT;

        BoundingBox aabb;
        aabb.mLower = glm::vec3(-1.0f, -1.0f, 0.0f);
        aabb.mUpper = glm::vec3(1.0f, 1.0f, 0.0f);

        ref<Geometry> geo = geoFactory->makeGeometryUnmanaged(vao, vbo, ibo, 
            GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, aabb);

        if (geoOut)
            *geoOut = geo;

        return contentManager->createContentNode(geo, parent);
    }
}