#pragma once

#include <string>
#include "halfedge.hpp"

namespace Assimp {
	class Importer;
}

namespace Morpheus {
	class HalfEdgeGeometry;

	struct HalfEdgeLoadParameters {
		stype mRelativeJoinEpsilon;
	};

	class HalfEdgeLoader {
	private:
		Assimp::Importer* mImporter;

	public:
		HalfEdgeGeometry* load(const std::string& source, const HalfEdgeLoadParameters& params);
		HalfEdgeGeometry* load(const std::string& source);

		HalfEdgeLoader();
		~HalfEdgeLoader();
	};
}