#include <engine/blit.hpp>
#include <engine/shader.hpp>
#include <engine/geometry.hpp>

namespace Morpheus {
	Shader* makeBlitShader(INodeOwner* parent, BlitShaderView* shaderViewOut) {
		// Load internal shader resource
        Shader* result = load<Shader>("/internal/blit.json", parent);
        GL_ASSERT;

        shaderViewOut->mLower.find(result, "lower");
        shaderViewOut->mUpper.find(result, "upper");
        shaderViewOut->mBlitTexture.find(result, "blitTexture");

		return result;
    }

	Shader* makeBlitShader(INodeOwner* parent, const std::string& fragmentSrc, 
		BlitShaderView* shaderViewOut, GLSLPreprocessorConfig* overrides) {

		std::vector<ShaderStageSource> sources;
		sources.push_back(ShaderStageSource{ShaderType::VERTEX, "/internal/blit.vert"});
		sources.push_back(ShaderStageSource{ShaderType::FRAGMENT, fragmentSrc});

		auto result = getFactory<Shader>()->make(parent, sources, overrides);

		shaderViewOut->mLower.find(result, "lower");
        shaderViewOut->mUpper.find(result, "upper");
        shaderViewOut->mBlitTexture.find(result, "blitTexture");

		return result;
	}

    Geometry* makeBlitGeometry(INodeOwner* parent) {
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

        Geometry* geo = geoFactory->makeGeometryUnmanaged(vao, vbo, ibo, 
            GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, aabb);

		createContentNode(geo, parent);

		return geo;
    }
}