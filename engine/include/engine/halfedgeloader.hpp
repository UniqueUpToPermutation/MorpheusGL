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
		HalfEdgeGeometry* loadUnmanaged(const std::string& source, const HalfEdgeLoadParameters& params);
		HalfEdgeGeometry* loadUnmanaged(const std::string& source);

		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		std::string getContentTypeString() const override;

		ContentFactory();
	};
}