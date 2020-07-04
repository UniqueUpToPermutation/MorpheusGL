#pragma once

#include "core.hpp"
#include "content.hpp"
#include "shader.hpp"

namespace Morpheus {
	class Material;
	SET_NODE_TYPE(Material, MATERIAL);

	class Material {
	protected:
		ref<Shader> mShader;

	public:
		virtual ref<Material> copy() const = 0;
	};

	template <>
	class ContentFactory<Material> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
	};
}