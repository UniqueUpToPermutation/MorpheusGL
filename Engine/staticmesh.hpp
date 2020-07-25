#pragma once

#include "content.hpp"
#include "material.hpp"
#include "geometry.hpp"

namespace Morpheus {
	class StaticMesh  {
	public:
		static ref<Geometry> getGeometry(Node meshNode);
		static Node getGeometryNode(Node meshNode);
		static ref<Material> getMaterial(Node meshNode);
		static Node getMaterialNode(Node meshNode);
		static void getParts(Node meshNode, ref<Geometry>* geo_out, ref<Material>* mat_out);
	};
	SET_NODE_ENUM(StaticMesh, STATIC_MESH);

	template <>
	class ContentFactory<StaticMesh> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		Node makeStaticMesh(const Node& material, const Node& geometry,
			ref<StaticMesh>* refOut = nullptr);
		Node makeStaticMesh(const Node& material, const Node& geometry,
			const std::string& source,
			ref<StaticMesh>* refOut = nullptr);
	};
}