#pragma once

#include "core.hpp"
#include "content.hpp"
#include "shader.hpp"

namespace Morpheus {
	/// <summary>
	/// A material is a shader plus overrides on different shader uniforms
	/// </summary>
	class Material {
	private:
		ref<Shader> mShader;
		ShaderUniformAssignments mUniformAssigments;
		ShaderSamplerAssignments mSamplerAssignments;

	public:
		inline ref<Shader> shader() const { return mShader; }
		inline const ShaderUniformAssignments& uniformAssignments() const {
			return mUniformAssigments;
		}
		inline const ShaderSamplerAssignments& samplerAssignments() const {
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
		void unload(ref<void>& ref) override;
		void dispose() override;
	};
}