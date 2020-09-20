#pragma once

#include <engine/core.hpp>
#include <engine/content.hpp>
#include <engine/shader.hpp>

namespace Morpheus {
	
	// A material is a shader plus overrides on different shader uniforms
	class Material {
	private:
		ref<Shader> mShader;
		ShaderUniformAssignments mUniformAssigments;
		ShaderSamplerAssignments mSamplerAssignments;

	public:
		inline ref<Shader> shader() const { return mShader; }
		inline ShaderUniformAssignments& uniformAssignments() {
			return mUniformAssigments;
		}
		inline ShaderSamplerAssignments& samplerAssignments() {
			return mSamplerAssignments;
		}

		friend ref<Material> duplicateRef<Material>(const ref<Material>& a);
		friend class ContentFactory<Material>;
	};
	SET_NODE_ENUM(Material, MATERIAL);

	template <>
	ref<Material> duplicateRef<Material>(const ref<Material>& a);
	template <>
	Node duplicateToNode<Material>(const ref<Material>& a);
	template <>
	Node duplicate<Material>(const Node& a);

	template <>
	class ContentFactory<Material> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void> ref) override;
		void dispose() override;

		std::string getContentTypeString() const override;
	};
}