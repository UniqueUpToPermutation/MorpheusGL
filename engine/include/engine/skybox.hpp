#pragma once

#include <engine/core.hpp>
#include <engine/shader.hpp>

namespace Morpheus {
	class Skybox : public INodeOwner {
	private:
		Texture* mSkyboxTexture;
		Sampler* mSkyboxSampler;
		Shader* mSkyboxShader;
		ShaderUniform<Sampler> mSkyboxUniform;

	public:
		inline Skybox(Texture* skyboxTexture) : INodeOwner(NodeType::SKYBOX), mSkyboxTexture(skyboxTexture) {
			assert(skyboxTexture->textureType() == TextureType::CUBE_MAP);
		}

		void prepare(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& eyePosition);

		Skybox* toSkybox() override;

		void init() override;
	};
	SET_NODE_ENUM(Skybox, SKYBOX);
}