#include <engine/blit.hpp>
#include <engine/shader.hpp>
#include <engine/geometry.hpp>

static const char blitShaderVert[] = 
    "#version 450 core\n"
    "layout(location = 0) in vec2 position;\n"
    "uniform vec2 lower;\n"
    "uniform vec2 upper;\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "vec2 transformed = lower + position * (upper - lower);\n"
    "gl_Position = vec4(transformed, 0.0, 1.0);\n"
    "uv = position;\n"
    "}";

static const char blitShaderFrag[] = 
    "#version 450 core\n"
    "in vec2 uv;\n"
    "uniform sampler2D blitTexture;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "outColor = texture(blitTexture, uv);\n"
    "}";

namespace Morpheus {
    GLint makeBlitShaderRaw() {
        auto vShader = compileShader(blitShaderVert, ShaderType::VERTEX);
        auto fShader = compileShader(blitShaderFrag, ShaderType::FRAGMENT);

        auto program = glCreateProgram();
        glAttachShader(program, vShader);
        glAttachShader(program, fShader);
        glLinkProgram(program);
        printProgramCompilerOutput(program);
        GL_ASSERT;

        return program;
    }

    Node makeBlitShader(Node parent, ref<Shader>* shaderOut, BlitShaderView* shaderViewOut) {
        auto contentManager = content();
        auto shaderFactory = contentManager->getFactory<Shader>();

        ref<Shader> result = shaderFactory->makeFromGL(makeBlitShaderRaw());
        GL_ASSERT;

        shaderViewOut->mLower.mLoc = glGetUniformLocation(result->id(), "lower");
        shaderViewOut->mUpper.mLoc = glGetUniformLocation(result->id(), "upper");
        shaderViewOut->mBlitTexture.mLoc = glGetUniformLocation(result->id(), "blitTexture");

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

        unsigned short idx[] = { 0, 1, 2, 2, 1, 3 };

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