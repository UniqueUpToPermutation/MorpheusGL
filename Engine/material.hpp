#pragma once

#include "core.hpp"
#include "content.hpp"
#include "shader.hpp"

namespace Morpheus {
	class IMaterial;
	SET_NODE_TYPE(IMaterial, MATERIAL);

	class IMaterial {
	protected:
		ref<IShader> mShader;

	public:
		virtual ref<IMaterial> copy() const = 0;
	};

	template <>
	class ContentFactory<IMaterial> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
	};
}