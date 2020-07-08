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

	public:
		inline ref<Shader> shader() const { return mShader; }
		inline const ShaderUniformAssignments& uniformAssignments() const {
			return mUniformAssigments;
		}

		friend ref<Material> copy(const ref<Material>& a);
		friend class ContentFactory<Material>;
	};
	SET_NODE_TYPE(Material, MATERIAL);

	ref<Material> copy(const ref<Material>& a);

	template <>
	class ContentFactory<Material> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;
	};
}