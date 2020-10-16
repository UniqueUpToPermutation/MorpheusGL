#pragma once

#include <engine/core.hpp>
#include <engine/content.hpp>
#include <engine/shader.hpp>

namespace Morpheus {
	
	// A material is a shader plus overrides on different shader uniforms
	class Material : public INodeOwner {
	private:
		Shader* mShader;
		ShaderUniformAssignments mUniformAssigments;
		ShaderSamplerAssignments mSamplerAssignments;

	public:
		inline Material() : INodeOwner(NodeType::MATERIAL) {
		}

		Material* toMaterial() override;

		inline Shader* shader() const { return mShader; }
		inline ShaderUniformAssignments& uniformAssignments() {
			return mUniformAssigments;
		}
		inline ShaderSamplerAssignments& samplerAssignments() {
			return mSamplerAssignments;
		}
		inline Material* duplicate();

		friend class ContentFactory<Material>;
	};
	SET_NODE_ENUM(Material, MATERIAL);

	template <>
	class ContentFactory<Material> : public IContentFactory {
	public:
		~ContentFactory();

		INodeOwner* load(const std::string& source, Node loadInto) override;
		void unload(INodeOwner* ref) override;

		std::string getContentTypeString() const override;
	};

	struct DefaultMaterialShaderView {
		ShaderUniform<glm::vec3[]> mEnvironmentDiffuseSH;
		ShaderUniform<Sampler> mEnvironmentSpecular;

		ShaderUniform<Sampler> mAlbedo;
		ShaderUniform<Sampler> mRoughness;
		ShaderUniform<Sampler> mNormal;
		ShaderUniform<Sampler> mDisplacement;

		ShaderUniform<GLboolean> bSampleMetalness;
		ShaderUniform<Sampler> mMetalness;
		ShaderUniform<GLfloat> mMetalnessDefault;

		DefaultMaterialShaderView(Shader* shader);
	};
}