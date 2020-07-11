#pragma once

#include "content.hpp"
#include "material.hpp"
#include "geometry.hpp"

namespace Morpheus {
	class StaticMesh {
	public:
	};
	SET_NODE_TYPE(StaticMesh, STATIC_MESH);

	template <>
	class ContentFactory<StaticMesh> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		Node makeStaticMesh(const Node& material, const Node& geometry,
			ref<StaticMesh>* refOut = nullptr);
	};
}