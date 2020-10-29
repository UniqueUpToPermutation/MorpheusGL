#include <engine/bvh.hpp>

#include <functional>

namespace Morpheus {
	template <typename base_t, bool is_base_t_pointer>
	BoundingBox bvhComputeBoundingBoxProxy(const base_t* obj);
	template <typename base_t, bool is_base_t_pointer>
	glm::vec3 bvhComputeCenter(const base_t* obj);

	template <typename base_t>
	inline BoundingBox bvhComputeBoundingBoxProxy<base_t, true>(const base_t* obj) {
		return (*obj)->computeBoundingBox();
	}

	template <typename base_t>
	inline BoundingBox bvhComputeBoundingBoxProxy<base_t, false>(const base_t* obj) {
		return obj->computeBoundingBox();
	}

	template <typename base_t>
	inline glm::vec3 bvhComputeCenter<base_t, true>(const base_t* obj) {
		return (*obj)->computeCenter();
	}

	template <typename base_t>
	inline glm::vec3 bvhComputeCenter<base_t, false>(const base_t* obj) {
		return obj->computeCenter();
	}

	template <typename base_t, bool is_base_t_pointer>
	void BinaryBVH<base_t, is_base_t_pointer>::build(const std::vector<base_t>& leaves, uint maxLeafSize, BinaryBVHSplitHeuristic heuristic) {
		mLeaves.clear();
		mNodes.clear();
		mBoundingBoxes.clear();
		mCentroids.clear();

		mLeaves = leaves; // Copy over leaves

		// Compute bounding boxes and centroids
		mBoundingBoxes.reserve(leaves.size());
		for (auto& leaf : leaves) {
			BoundingBox bb = bvhComputeBoundingBoxProxy<base_t, is_base_t_pointer>(leaf);
			mBoundingBoxes.emplace_back(bb);

			glm::vec3 centroid = bvhComputeCenter<base_t, is_base_t_pointer>(leaf);
			mCentroids.emplace_back(centroid);
		}

		buildKdTreeRecursive(0, mLeaves.size(), maxLeafSize, heuristic);

		mCentroids.clear();
	}

	template <uint dim>
	bool compareVec(const glm::vec3* x, const glm::vec3* y) {
		return ((*x)[dim] < (*y)[dim]);
	}

	template <typename base_t, bool is_base_t_pointer>
	void BinaryBVH<base_t, is_base_t_pointer>::classifyCentroidSplit(base_t* leavesBegin, base_t* leavesEnd, 
		std::vector<uint8_t>* labels) {
		
		// First determine the extents of the points we have been given
		uint offset = leavesBegin - &mLeaves[0];
		uint count = leavesEnd - leavesBegin;
		
		glm::vec3* centroids = &mCentroids[offset];

		// Compute extents
		BoundingBox bb = BoundingBox::empty();
		for (uint i = 0; i < count; +i) {
			bb.mergeInPlace(centroids[i]);
		}

		glm::vec3 widths = bb.mUpper - bb.mLower;
		float max_width = std::max(widths.x, std::max(widths.y, widths.z));

		float threshold;

		if (max_width == widths.x) {
			threshold = bb.mLower.x + widths.x / 2.0;
		}
		else if (max_width == widths.y) {
			threshold = bb.mLower.y + widths.y / 2.0;
		}
		else {
			threshold = bb.mLower.z + widths.z / 2.0;
		}

		labels->resize(count);
		for (uint i = 0; i < count; ++i) {
			labels[i] = centroids[i] > threshold;
		}
	}

	template <typename base_t, bool is_base_t_pointer>
	void BinaryBVH<base_t, is_base_t_pointer>::classifySurfaceAreaHeuristic(base_t* leavesBegin, base_t* leavesEnd, 
		std::vector<uint8_t>* labels) {

	}

	template <typename base_t, bool is_base_t_pointer>
	int BinaryBVH<base_t, is_base_t_pointer>::buildRecursive(base_t* leavesBegin, base_t* leavesEnd, 
	uint maxLeafSize, BinaryBVHSplitHeuristic heuristic) {

		uint offset = leavesBegin - &mLeaves[0];
		uint leavesCount = leavesEnd - leavesBegin;
		BoundingBox* boundingBoxes = &mBoundingBoxes[offset];
		glm::vec3* centroids = &mCentroids[offset];
		node_t node;

		// Build leaf node
		if (leavesCount <= maxLeafSize) {
			node.bIsLeaf = true;
			node.mData.mLeaf.mLeafStartIdx = offset;
			node.mData.mLeaf.mLeafItemCount = leavesCount;
			
			// Compute the bounding box
			node.mBoundingBox = BoundingBox::empty();
			for (uint i = 0; i < leavesCount; ++i)
				node.mBoundingBox.mergeInPlace(boundingBoxes[i]);

			if (node.mBoundingBox.isEmpty()) {
				return -1; // Empty, we can just leave this out of the tree
			} else {
				// Add this node!
				int id = mNodes.size();
				mNodes.emplace_back(node);
				return id;
			}
		}
		else {
			std::vector<uint8_t> labels;

			// Build internal node
			switch (heuristic) {
			case BinaryBVHSplitHeuristic::KD_CENTROID_SPLIT:
				classifyCentroidSplit(leavesBegin, leavesEnd, &labels);
				break;

			case BinaryBVHSplitHeuristic::KD_SURFACE_AREA_HEURISTIC:
				classifySurfaceAreaHeuristic(leavesBegin, leavesEnd, &labels);
				break;
			}

			// Do a sorting of the items
			int leftIdx = 0;
			int rightIdx = leavesCount - 1;

			while (leftIdx < rightIdx) {
				while (rightIdx >= 0 && labels[rightIdx] == 1) {
					--rightIdx;
				}

				while (leftIdx < leavesCount && labels[leftIdx] == 0) {
					--leftIdx;
				}

				if (leftIdx >= rightIdx)
					break;

				if (labels[leftIdx] == 1) {
					std::swap(leavesBegin[leftIdx], leavesBegin[rightIdx]);
					std::swap(labels[leftIdx], labels[rightIdx]);
					std::swap(boundingBoxes[leftIdx], boundingBoxes[rightIdx]);
					std::swap(centroids[leftIdx], centroids[rightIdx]);
					++leftIdx;
					--rightIdx;
				}
			}

			if (labels[rightIdx] == 0)
				++rightIdx;
			
			int left_node = buildKdTreeRecursive(&leavesBegin[rightIdx], leavesEnd, maxLeafSize, heuristic);
			int right_node = buildKdTreeRecursive(leavesBegin, &leavesBegin[rightIdx], maxLeafSize, heuristic);

			node.bIsLeaf = false;
			node.mData.mInternal.mLeft = left_node;
			node.mData.mInternal.mRight = right_node;
			
			BoundingBox bbLeft = BoundingBox::empty();
			BoundingBox bbRight = BoundingBox::empty();

			if (left_node >= 0)
				bbLeft = mNodes[left_node].mBoundingBox;
			if (right_node >= 0)
				bbRight = mNodes[right_node].mBoundingBox;

			node.mBoundingBox = bbLeft.merge(bbRight);
			
			if (node.mBoundingBox.isEmpty()) {
				return -1; // Empty, we can just leave this out of the tree
			} else {
				// Add this node!
				int id = mNodes.size();
				mNodes.emplace_back(node);
				return id;
			}
		}
	}
}