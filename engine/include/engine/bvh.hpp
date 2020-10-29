#pragma once

#include <vector>

#include <engine/geobase.hpp>

namespace Morpheus {

	struct BinaryBVHInternalData {
		int mLeft;
		int mRight;
	};

	struct BinaryBVHLeafData {
		int mLeafStartIdx;
		int LeafItemCount;
	};

	union BinaryBVHData {
		BinaryBVHInternalData mInternal;
		BinaryBVHLeafData mLeaf;
	};

	template <typename base_t>
	struct BinaryBVHNode {
		BoundingBox mBoundingBox;
		bool bIsLeaf;
		BinaryBVHData mData;
	};

	enum class BinaryBVHSplitHeuristic {
		KD_CENTROID_SPLIT,
		KD_SURFACE_AREA_HEURISTIC
	};

	template <typename base_t, bool is_base_t_pointer = std::is_pointer_v<base_t>>
	class BinaryBVH {
		typedef BinaryBVHNode<base_t> node_t;

	protected:
		std::vector<base_t> mLeaves;
		std::vector<node_t> mNodes;
		std::vector<BoundingBox> mBoundingBoxes;
		std::vector<glm::vec3> mCentroids;

		int buildRecursive(base_t* leavesBegin, base_t* leavesEnd, uint maxLeafSize, BinaryBVHSplitHeuristic heuristic);

		void classifyCentroidSplit(base_t* leavesBegin, base_t* leavesEnd,
			std::vector<uint8_t>* labels);
		void classifySurfaceAreaHeuristic(base_t* leavesBegin, base_t* leavesEnd,
			std::vector<uint8_t>* labels);

	public:
		void build(const std::vector<base_t>& leaves, uint maxLeafSize, BinaryBVHSplitHeuristic heuristic);
	};
}