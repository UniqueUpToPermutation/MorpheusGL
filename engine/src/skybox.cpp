#include <engine/skybox.hpp>

namespace Morpheus {
	Skybox* Skybox::toSkybox() {
		return this;
	}

	void Skybox::prepare(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& eyePosition) {
		mSkyboxShader->bind();
		auto& renderView = mSkyboxShader->renderView();

		renderView.mView.set(view);
		renderView.mProjection.set(projection);
		renderView.mEyePosition.set(eyePosition);
		mSkyboxUniform.set(mSkyboxTexture, mSkyboxSampler);
	}

	void Skybox::init() {
		mSkyboxShader = load<Shader>("internal/skybox.json", this);
		mSkyboxSampler = load<Sampler>(MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC, this);
		addChild(mSkyboxTexture);

		mSkyboxUniform.find(mSkyboxShader, "skybox");
		if (!mSkyboxUniform.valid()) {
			throw std::runtime_error("Skybox: Could not find uniform skybox!");
		}
	}
}