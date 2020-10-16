#pragma once

#include <string>

#include <engine/halfedge.hpp>
#include <engine/content.hpp>

namespace Assimp {
	class Importer;
}

namespace Morpheus {
	class HalfEdgeGeometry;
	SET_NODE_ENUM(HalfEdgeGeometry, HALF_EDGE_GEOMETRY);

	struct HalfEdgeLoadParameters {
		stype mRelativeJoinEpsilon;
	};

	template <>
	class ContentFactory<HalfEdgeGeometry> : public IContentFactory {
	private:
		Assimp::Importer* mImporter;

	public:
		~ContentFactory();

		HalfEdgeGeometry* loadUnmanaged(const std::string& source, const HalfEdgeLoadParameters& params);
		HalfEdgeGeometry* loadUnmanaged(const std::string& source);

		INodeOwner* load(const std::string& source, Node loadInto) override;
		void unload(INodeOwner* ref) override;

		std::string getContentTypeString() const override;

		ContentFactory();
	};
}