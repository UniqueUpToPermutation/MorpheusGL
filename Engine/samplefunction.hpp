#pragma once

#include "core.hpp"
#include "texture.hpp"

#include <type_traits>
#include <chrono>
#include <random>

#define DEFAULT_KERNEL_SAMPLE_COUNT 100u

namespace Morpheus {
	template <typename VectorType>
	struct ScalarType;

	template <>
	struct ScalarType<glm::vec2> {
		typedef decltype(glm::vec2::x) RESULT;
	};

	template <>
	struct ScalarType<glm::vec3> {
		typedef decltype(glm::vec3::x) RESULT;
	};

	template <>
	struct ScalarType<glm::vec4> {
		typedef decltype(glm::vec4::x) RESULT;
	};

	template <>
	struct ScalarType<glm::vec1> {
		typedef decltype(glm::vec1::x) RESULT;
	};

	template <>
	struct ScalarType<glm::dvec2> {
		typedef decltype(glm::dvec2::x) RESULT;
	};

	template <>
	struct ScalarType<glm::dvec3> {
		typedef decltype(glm::dvec3::x) RESULT;
	};

	template <>
	struct ScalarType<glm::dvec4> {
		typedef decltype(glm::dvec4::x) RESULT;
	};

	template <>
	struct ScalarType<glm::dvec1> {
		typedef decltype(glm::dvec1::x) RESULT;
	};


	template <>
	struct ScalarType<float> {
		typedef float RESULT;
	};

	template <>
	struct ScalarType<double> {
		typedef float RESULT;
	};

	class BilinearInterpolator {
	public:
		constexpr static uint32_t RequestedSampleValues = 4;
		typedef glm::vec2 WeightType;

		constexpr static uint32_t TOP_LEFT = 0;
		constexpr static uint32_t TOP_RIGHT = 1;
		constexpr static uint32_t BOTTOM_LEFT = 2;
		constexpr static uint32_t BOTTOM_RIGHT = 3;

		template <typename ReturnType, typename InputType>
		static void interp(const ReturnType sampleValues[], const uint32_t sampleCount, 
			const WeightType& weights, ReturnType* out) {
			assert(sampleCount == RequestedSampleValues);

			auto top = (1 - weights.x) * sampleValues[TOP_LEFT] + weights.x * sampleValues[TOP_RIGHT];
			auto bottom = (1 - weights.x) * sampleValues[BOTTOM_LEFT] + weights.x * sampleValues[BOTTOM_RIGHT];
			*out = top * (1 - weights.y) + bottom * weights.y;
		}
	}; 

	class BilinearSampler {
	public:
		typedef BilinearInterpolator DefaultInterpolator;
		typedef glm::vec2 WeightType;

		constexpr static uint32_t RequestedSamplePositions = 1;

		template <typename InputType>
		static void computeWeights(const InputType queryPosition,
			const InputType samplePositions[],
			const uint32_t samplePositionCount,
			WeightType* weightsOut) {
			assert(samplePositionCount == RequestedSamplePositions);

			weightsOut->x = static_cast<decltype(weightsOut->x)>(samplePositions[0].x);
			weightsOut->y = static_cast<decltype(weightsOut->y)>(samplePositions[0].y);
		}
	};

	enum class WrapType {
		CLAMP,
		REPEAT
	};

	template <typename OutputType, typename InputType = glm::vec2>
	class RectSurfaceGridStorage;
	template <typename OutputType, typename InputType = glm::vec3>
	class CubemapStorage;

	template <typename ReturnType,
		typename InputType,
		typename StorageType,
		typename SamplerType,
		typename InterpolatorType = typename SamplerType::DefaultInterpolator>
		class SampleFunction;

	template <typename ReturnType>
	using Function2D = SampleFunction<ReturnType, glm::vec2,
		RectSurfaceGridStorage<ReturnType>,
		BilinearSampler>;

	template <typename ReturnType>
	using FunctionSphere = SampleFunction<ReturnType, glm::vec3,
		CubemapStorage<ReturnType>,
		BilinearSampler>;

	enum class StorageMode {
		UNALLOCATED,
		ALLOCATED,
		READ,
		WRITE
	};

	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<float>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec2>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec3>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec4>* out);

	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<float>& f);
	void saveCubemapeStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec2>& f);
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec3>& f);
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f);

	void testSaveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f);

	template <typename OutputType, typename InputType>
	class CubemapStorage {
	public:
		typedef glm::uvec2 SizeType;
		typedef glm::ivec3 IndexType;

		constexpr static uint32_t BORDER_TOP = 0;
		constexpr static uint32_t BORDER_BOTTOM = 1;
		constexpr static uint32_t BORDER_LEFT = 2;
		constexpr static uint32_t BORDER_RIGHT = 3;

		constexpr static uint32_t FACE_POSITIVE_X = 0;
		constexpr static uint32_t FACE_NEGATIVE_X = 1;
		constexpr static uint32_t FACE_POSITIVE_Y = 2;
		constexpr static uint32_t FACE_NEGATIVE_Y = 3;
		constexpr static uint32_t FACE_POSITIVE_Z = 4;
		constexpr static uint32_t FACE_NEGATIVE_Z = 5;

		constexpr static uint32_t ALLOCATE_DIMENSION = 2;

		typedef typename ScalarType<InputType>::RESULT scalar_t;

	private:
		std::vector<OutputType> mCubemapData;
		InputType mScale;
		InputType mScaleInv;
		SizeType mGridSize;
		StorageMode mStorageMode;

		void transitionToRead() {
			struct Boundary {
				uint32_t mFace;
				uint32_t mSide;
				bool bFlip;
			};

			Boundary borderPairs[6][4];
			uint64_t edgeOffsets[4];
			int64_t edgeStrides[4];
			uint64_t faceOffsets[6];
			uint64_t srcEdgeOffsets[4];
			uint64_t srcEdgeEndOffsets[4];

			size_t interiorSize = mGridSize.x - 2;

			// Create the edge map
			borderPairs[FACE_POSITIVE_X][BORDER_TOP] = { FACE_POSITIVE_Y, BORDER_RIGHT, true };
			borderPairs[FACE_POSITIVE_X][BORDER_BOTTOM] = { FACE_NEGATIVE_Y, BORDER_RIGHT, false };
			borderPairs[FACE_POSITIVE_X][BORDER_LEFT] = { FACE_POSITIVE_Z, BORDER_RIGHT, false };
			borderPairs[FACE_POSITIVE_X][BORDER_RIGHT] = { FACE_NEGATIVE_Z, BORDER_LEFT, false };

			borderPairs[FACE_NEGATIVE_X][BORDER_TOP] = { FACE_POSITIVE_Y, BORDER_LEFT, false };
			borderPairs[FACE_NEGATIVE_X][BORDER_BOTTOM] = { FACE_NEGATIVE_Y, BORDER_LEFT, true };
			borderPairs[FACE_NEGATIVE_X][BORDER_LEFT] = { FACE_NEGATIVE_Z, BORDER_RIGHT, false };
			borderPairs[FACE_NEGATIVE_X][BORDER_RIGHT] = { FACE_POSITIVE_Z, BORDER_LEFT, false };

			borderPairs[FACE_POSITIVE_Y][BORDER_TOP] = { FACE_NEGATIVE_Z, BORDER_TOP, true };
			borderPairs[FACE_POSITIVE_Y][BORDER_BOTTOM] = { FACE_POSITIVE_Z, BORDER_TOP, false };
			borderPairs[FACE_POSITIVE_Y][BORDER_LEFT] = { FACE_NEGATIVE_X, BORDER_TOP, false };
			borderPairs[FACE_POSITIVE_Y][BORDER_RIGHT] = { FACE_POSITIVE_X, BORDER_TOP, true};

			borderPairs[FACE_NEGATIVE_Y][BORDER_TOP] = { FACE_POSITIVE_Z, BORDER_BOTTOM, false };
			borderPairs[FACE_NEGATIVE_Y][BORDER_BOTTOM] = { FACE_NEGATIVE_Z, BORDER_BOTTOM, true };
			borderPairs[FACE_NEGATIVE_Y][BORDER_LEFT] = { FACE_NEGATIVE_X, BORDER_BOTTOM, true, };
			borderPairs[FACE_NEGATIVE_Y][BORDER_RIGHT] = { FACE_POSITIVE_X, BORDER_BOTTOM, false };

			borderPairs[FACE_POSITIVE_Z][BORDER_TOP] = { FACE_POSITIVE_Y, BORDER_BOTTOM, false };
			borderPairs[FACE_POSITIVE_Z][BORDER_BOTTOM] = { FACE_NEGATIVE_Y, BORDER_TOP, false };
			borderPairs[FACE_POSITIVE_Z][BORDER_LEFT] = { FACE_NEGATIVE_X, BORDER_RIGHT, false };
			borderPairs[FACE_POSITIVE_Z][BORDER_RIGHT] = { FACE_POSITIVE_X, BORDER_LEFT, false };

			borderPairs[FACE_NEGATIVE_Z][BORDER_TOP] = { FACE_POSITIVE_Y, BORDER_TOP, true };
			borderPairs[FACE_NEGATIVE_Z][BORDER_BOTTOM] = { FACE_NEGATIVE_Y, BORDER_BOTTOM, true };
			borderPairs[FACE_NEGATIVE_Z][BORDER_LEFT] = { FACE_POSITIVE_X, BORDER_RIGHT, false };
			borderPairs[FACE_NEGATIVE_Z][BORDER_RIGHT] = { FACE_NEGATIVE_X, BORDER_LEFT, false };

#ifdef _DEBUG
			for (uint32_t face = 0; face < 6; ++face) {
				for (uint32_t side = 0; side < 4; ++side) {
					auto& res = borderPairs[face][side];
					auto& other = borderPairs[res.mFace][res.mSide];
					assert(other.mFace == face && other.mSide == side && other.bFlip == res.bFlip);
				}
			}
#endif

			edgeOffsets[BORDER_TOP] = 0;
			edgeOffsets[BORDER_BOTTOM] = (size_t)(mGridSize.y - 1) * (size_t)mGridSize.x;
			edgeOffsets[BORDER_LEFT] = 0;
			edgeOffsets[BORDER_RIGHT] = (size_t)mGridSize.x - 1;

			for (uint32_t face = 0; face < 6; ++face) {
				faceOffsets[face] = face * (size_t)mGridSize.x * (size_t)mGridSize.y;
			}

			edgeStrides[BORDER_TOP] = 1;
			edgeStrides[BORDER_BOTTOM] = 1;
			edgeStrides[BORDER_LEFT] = (size_t)mGridSize.x;
			edgeStrides[BORDER_RIGHT] = (size_t)mGridSize.x;

			srcEdgeOffsets[BORDER_TOP] = (size_t)mGridSize.x;
			srcEdgeOffsets[BORDER_BOTTOM] = (size_t)mGridSize.x * (size_t)(mGridSize.y - 2);
			srcEdgeOffsets[BORDER_LEFT] = 1;
			srcEdgeOffsets[BORDER_RIGHT] = (size_t)mGridSize.x - 2;

			srcEdgeEndOffsets[BORDER_TOP] = 2 * (size_t)(mGridSize.x) - 1;
			srcEdgeEndOffsets[BORDER_BOTTOM] = (size_t)mGridSize.x * (size_t)(mGridSize.y - 1) - 1;
			srcEdgeEndOffsets[BORDER_LEFT] = (size_t)mGridSize.x * (size_t)(mGridSize.y - 1) + 1;
			srcEdgeEndOffsets[BORDER_RIGHT] = (size_t)mGridSize.x * (size_t)mGridSize.y - 2;

			typedef typename ScalarType<OutputType>::RESULT scalar_t;

			for (uint32_t dest_face = 0; dest_face < 6; ++dest_face) {
				// Copy edges
				for (uint32_t dest_edge = 0; dest_edge < 4; ++dest_edge) {
					uint64_t dest_stride = edgeStrides[dest_edge];
					auto copy_dest = &mCubemapData[faceOffsets[dest_face] + edgeOffsets[dest_edge] + dest_stride];
					
					auto& src = borderPairs[dest_face][dest_edge];
					int64_t src_stride = edgeStrides[src.mSide];
					int64_t src_edge_offset;
					if (src.bFlip) {
						src_stride = -src_stride;
						src_edge_offset = srcEdgeEndOffsets[src.mSide];
					}
					else {
						src_edge_offset = srcEdgeOffsets[src.mSide];
					}

					auto copy_src = &mCubemapData[faceOffsets[src.mFace] + src_edge_offset + src_stride];

					for (uint32_t i = 0; i < interiorSize; copy_dest += dest_stride, copy_src += src_stride, ++i)
						*copy_dest = *copy_src;
				}

				// Update corners to average of nearby pixels
				auto face_ptr = &mCubemapData[faceOffsets[dest_face]];
				auto norm_factor = static_cast<scalar_t>(1.0 / 3.0);

				// Top left
				face_ptr[0] = 
					(face_ptr[1] + 
					face_ptr[mGridSize.x] + 
					face_ptr[mGridSize.x + 1]) * norm_factor;
				// Top right
				face_ptr[mGridSize.x - 1] = 
					(face_ptr[mGridSize.x - 2] + 
					face_ptr[2 * mGridSize.x - 2] + 
					face_ptr[2 * mGridSize.x - 1]) * norm_factor;
				// Bottom left
				face_ptr[mGridSize.x * (mGridSize.y - 1)] =
					(face_ptr[mGridSize.x * (mGridSize.y - 1) + 1] +
						face_ptr[mGridSize.x * (mGridSize.y - 2)] +
						face_ptr[mGridSize.x * (mGridSize.y - 2) + 1]) * norm_factor;
				// Bottom right
				face_ptr[mGridSize.x * mGridSize.y - 1] =
					(face_ptr[mGridSize.x * mGridSize.y - 2] +
					face_ptr[mGridSize.x * (mGridSize.y - 1)] +
					face_ptr[mGridSize.x * (mGridSize.y - 1) - 1]) * norm_factor;
			}
		}

	public:

		inline uint32_t width() const {
			return mGridSize.x - 2;
		}

		inline uint32_t height() const {
			return mGridSize.y - 2;
		}

		inline uint32_t depth() const {
			return 6u;
		}
		
		CubemapStorage() : mStorageMode(StorageMode::UNALLOCATED) {
		}

		StorageMode getMode() const {
			return mStorageMode;
		}

		// Must be called before samples are assigned to
		void allocate(const SizeType& gridSize) {
			mGridSize = gridSize;
			// Make room for the padding
			mGridSize.x += 2;
			mGridSize.y += 2;

			mCubemapData.resize(mGridSize.x * mGridSize.y * 6);

			mScale.x = static_cast<decltype(mScale.x)>(1.0 / gridSize.x);
			mScale.y = static_cast<decltype(mScale.y)>(1.0 / gridSize.y);
			mScaleInv.x = static_cast<decltype(mScale.x)>(gridSize.x);
			mScaleInv.y = static_cast<decltype(mScale.y)>(gridSize.y);

			mStorageMode = StorageMode::ALLOCATED;
		}

		void allocate(const uint32_t sizeX, const uint32_t sizeY) {
			allocate(SizeType(sizeX, sizeY));
		}

		struct CubemapUV {
			scalar_t u;
			scalar_t v;
			uint32_t face;
		};

		CubemapUV getUV(const InputType& query) const {
			InputType absQuery = glm::abs(query);
			CubemapUV result;
			// Find correct face
			if (absQuery.x >= absQuery.y && absQuery.x >= absQuery.z) {
				if (query.x >= 0.0) {
					result.face = FACE_POSITIVE_X;
					result.u = query.z / absQuery.x;
					result.v = query.y / absQuery.x;
				}
				else {
					result.face = FACE_NEGATIVE_X;
					result.u = -query.z / absQuery.x;
					result.v = query.y / absQuery.x;
				}
			}
			else if (absQuery.y >= absQuery.x && absQuery.y >= absQuery.z) {
				if (query.y >= 0.0) {
					result.face = FACE_POSITIVE_Y;
					result.u = query.x / absQuery.y;
					result.v = query.z / absQuery.y;
				}
				else {
					result.face = FACE_NEGATIVE_Y;
					result.u = query.x / absQuery.y;
					result.v = -query.z / absQuery.y;
				}
			}
			else if (absQuery.z >= absQuery.x && absQuery.z >= absQuery.y) {
				if (query.z >= 0.0) {
					result.face = FACE_POSITIVE_Z;
					result.u = query.x / absQuery.z;
					result.v = query.y / absQuery.z;
				}
				else {
					result.face = FACE_NEGATIVE_Z;
					result.u = -query.x / absQuery.z;
					result.v = query.y / absQuery.z;
				}
			}

			// Normalize to [0, 1] texture coordinates
			result.u = static_cast<scalar_t>(0.5) * (result.u + static_cast<scalar_t>(1.0));
			result.v = static_cast<scalar_t>(0.5) * (-result.v + static_cast<scalar_t>(1.0));

			assert(result.u >= 0.0 && result.u <= 1.0);
			assert(result.v >= 0.0 && result.v <= 1.0);

			return result;
		}

		void collectSamples(const InputType& query,
			InputType samplePositionsOut[],
			OutputType sampleValuesOut[],
			uint32_t* samplePositionCountOut,
			uint32_t* sampleValueCountOut) const {

			CubemapUV uv = getUV(query);

			// Account for boundary
			scalar_t scaled_u = uv.u * mScaleInv.x + static_cast<scalar_t>(0.5);
			scalar_t scaled_v = uv.v * mScaleInv.y + static_cast<scalar_t>(0.5);

			IndexType indx;
			indx.x = static_cast<decltype(indx.x)>(scaled_u);
			indx.y = static_cast<decltype(indx.y)>(scaled_v);
			indx.z = uv.face;

			sampleValuesOut[BilinearInterpolator::TOP_LEFT] = (*this)(indx.x, indx.y, indx.z);
			sampleValuesOut[BilinearInterpolator::TOP_RIGHT] = (*this)(indx.x + 1, indx.y, indx.z);
			sampleValuesOut[BilinearInterpolator::BOTTOM_LEFT] = (*this)(indx.x, indx.y + 1, indx.z);
			sampleValuesOut[BilinearInterpolator::BOTTOM_RIGHT] = (*this)(indx.x + 1, indx.y + 1, indx.z);

			samplePositionsOut[0].x = scaled_u - std::floor(scaled_u);
			samplePositionsOut[0].y = scaled_v - std::floor(scaled_v);

			*samplePositionCountOut = 1;
			*sampleValueCountOut = 4;
		}

		void transition(StorageMode mode) {
			switch (mode) {
			case StorageMode::UNALLOCATED:
				mCubemapData.clear();
				break;
			case StorageMode::READ:
				transitionToRead();
				break;
			}

			mStorageMode = mode;
		}

		OutputType& operator()(const uint32_t i, const uint32_t j, const uint32_t face) {
			// Account for cubemap border
			return mCubemapData[mGridSize.x * mGridSize.y * face + (i + 1) + (j + 1) * mGridSize.x];
		}

		OutputType& operator()(const IndexType& pos) {
			// Account for cubemap border
			return mCubemapData[mGridSize.x * mGridSize.y * pos.z + (pos.x + 1) + (pos.y + 1) * mGridSize.x];
		}

		const OutputType& operator()(const uint32_t i, const uint32_t j, const uint32_t face) const {
			// Account for cubemap border
			return mCubemapData[mGridSize.x * mGridSize.y * face + (i + 1) + (j + 1) * mGridSize.x];
		}

		const OutputType& operator()(const IndexType& pos) const {
			// Account for cubemap border
			return mCubemapData[mGridSize.x * mGridSize.y * pos.z + (pos.x + 1) + (pos.y + 1) * mGridSize.x];
		}

		InputType getSampleLocation(const IndexType& pos) const {
			InputType result;

			scalar_t u = static_cast<scalar_t>(pos.x);
			scalar_t v = static_cast<scalar_t>(pos.y);

			u += static_cast<scalar_t>(0.5);
			v += static_cast<scalar_t>(0.5);

			u *= mScale.x;
			v *= mScale.y;

			u = u * static_cast<scalar_t>(2.0) - static_cast<scalar_t>(1.0);
			v = -(v * static_cast<scalar_t>(2.0) - static_cast<scalar_t>(1.0));

			switch (pos.z) {
			case FACE_POSITIVE_X:
				result.x = 1.0;
				result.z = u;
				result.y = v;
				break;
			case FACE_NEGATIVE_X:
				result.x = -1.0;
				result.z = -u;
				result.y = v;
				break;
			case FACE_POSITIVE_Y:
				result.x = u;
				result.y = 1.0;
				result.z = v;
				break;
			case FACE_NEGATIVE_Y:
				result.x = u;
				result.y = -1.0;
				result.z = -v;
				break;
			case FACE_POSITIVE_Z:
				result.x = u;
				result.y = v;
				result.z = 1.0;
				break;
			case FACE_NEGATIVE_Z:
				result.x = -u;
				result.y = v;
				result.z = -1.0;
				break;
			default:
				throw std::exception("pos.z is an invalid face!");
				break;
			}

			return glm::normalize(result);
		}

		IndexType getIndexCoords(const uint64_t index) const {
			IndexType result;
			size_t width_ = width();
			size_t height_ = height();
			result.z = static_cast<decltype(result.z)>(index / (width_ * height_));
			uint64_t faceOffset = index - result.z * width_ * height_;
			result.x = static_cast<decltype(result.x)>(faceOffset % width_);
			result.y = static_cast<decltype(result.y)>(faceOffset / width_);

			return result;
		}

		inline InputType getSampleLocation(const uint64_t index) const {
			return getSampleLocation(getIndexCoords(index));
		}

		inline OutputType& operator[](const uint64_t i) {
			return (*this)(getIndexCoords(i));
		}

		inline const OutputType& operator[](const uint64_t i) const {
			return (*this)(getIndexCoords(i));
		}

		inline OutputType& atIndex(const uint64_t i, const uint32_t face) {
			return (*this)(getIndexCoords(face * width() * height() + i));
		}

		inline const OutputType& atIndex(const uint64_t i, const uint32_t face) const {
			return (*this)(getIndexCoords(face * width() * height() + i));
		}

		inline OutputType& atIndex(const uint64_t i) {
			return (*this)(getIndexCoords(i));
		}

		inline const OutputType& atIndex(const uint64_t i) const {
			return (*this)(getIndexCoords(i));
		}

		inline size_t sampleCount() const {
			if (mCubemapData.size() == 0)
				return 0;
			else
				return (mGridSize.x - 2) * (mGridSize.y - 2) * 6;
		}

		void loadpng(const std::string& filename) {
			loadCubemapStorageFromPNG(filename, this);
		}

		void savepng(const std::string& filename) {
			saveCubemapStorageToPNG(filename, *this);
		}

		template <typename ReturnType>
		friend void loadCubemapStorageFromPNGinternal(const std::string& filename, CubemapStorage<ReturnType>* out);
		template <typename ReturnType>
		friend void saveCubemapStorageToPNGinternal(const std::string& filename, const CubemapStorage<ReturnType>& f);

		friend void testSaveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f);
	};

	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<float>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec2>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec3>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec4>* out);

	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<float>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec2>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec3>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec4>& f);

	template <typename OutputType, typename InputType>
	class RectSurfaceGridStorage {
	public:
		typedef glm::uvec2 SizeType;
		typedef glm::ivec2 IndexType;

		constexpr static uint32_t ALLOCATE_DIMENSION = 2;

	private:
		std::vector<OutputType> mGrid;
		InputType mScale;
		InputType mScaleInv;
		InputType mClampBoundsUpper;
		InputType mClampBoundsLower;
		SizeType mGridSize;
		WrapType mWrapType;
		StorageMode mStorageMode;

	public:

		inline uint32_t width() const {
			return mGridSize.x;
		}

		inline uint32_t height() const {
			return mGridSize.y;
		}

		inline uint32_t depth() const {
			return 1;
		}

		RectSurfaceGridStorage() :
			mStorageMode(StorageMode::UNALLOCATED) {
		}

		StorageMode getMode() const {
			return mStorageMode;
		}

		WrapType& wrap() { return mWrapType; }

		void allocate(const SizeType& gridSize) {
			mGridSize = gridSize;
			mScale.x = static_cast<decltype(mScale.x)>(1.0 / mGridSize.x);
			mScale.y = static_cast<decltype(mScale.y)>(1.0 / mGridSize.y);
			mScaleInv.x = static_cast<decltype(mScaleInv.x)>(mGridSize.x);
			mScaleInv.y = static_cast<decltype(mScaleInv.y)>(mGridSize.y);
			mClampBoundsLower = mScale / static_cast<decltype(mScale.x)>(2.0);
			mClampBoundsUpper = InputType(1.0f, 1.0f) - mScale / static_cast <decltype(mScale.x)>(2.0);
			mGrid.resize((size_t)gridSize.x * (size_t)gridSize.y);
		}

		void allocate(const uint32_t sizeX, const uint32_t sizeY) {
			allocate(SizeType(sizeX, sizeY));
		}

		void transition(StorageMode mode) {
			if (mode == StorageMode::UNALLOCATED)
				mGrid.clear();

			mStorageMode = mode;
		}

		OutputType& operator()(const uint32_t i, const uint32_t j) {
			return mGrid[i + j * mGridSize.x];
		}

		OutputType& operator()(const IndexType& pos) {
			return mGrid[pos.x + pos.y * mGridSize.x];
		}

		const OutputType& operator()(const uint32_t i, const uint32_t j) const {
			return mGrid[i + j * mGridSize.x];
		}

		const OutputType& operator()(const IndexType& pos) const {
			return mGrid[pos.x + pos.y * mGridSize.x];
		}

		OutputType& operator[](const uint64_t i) {
			return mGrid[i];
		}

		const OutputType& operator[](const uint64_t i) const {
			return mGrid[i];
		}

		OutputType& atIndex(const uint64_t i) {
			return mGrid[i];
		}

		const OutputType& atIndex(const uint64_t i) const {
			return mGrid[i];
		}

		InputType getSampleLocation(const IndexType& pos) const {
			InputType result;
			result.x = static_cast<decltype(result.x)>((static_cast<decltype(mScale.x)>(pos.x) + 0.5) * mScale.x);
			result.y = static_cast<decltype(result.y)>((static_cast<decltype(mScale.y)>(pos.y) + 0.5) * mScale.y);
			return result;
		}

		IndexType getIndexCoords(const uint64_t index) const {
			IndexType result;
			result.x = static_cast<decltype(result.x)>(index % mGridSize.x);
			result.y = static_cast<decltype(result.y)>(index / mGridSize.x);
			return result;
		}

		InputType getSampleLocation(const uint64_t index) const {
			InputType result;
			IndexType coords = getIndexCoords(index);
			result.x = static_cast<decltype(result.x)>((static_cast<decltype(mScale.x)>(coords.x) + 0.5) * mScale.x);
			result.y = static_cast<decltype(result.y)>((static_cast<decltype(mScale.y)>(coords.y) + 0.5) * mScale.y);
			return result;
		}

		inline size_t sampleCount() const {
			return mGrid.size();
		}

		void collectSamples(const InputType& query, 
			InputType samplePositionsOut[], 
			OutputType sampleValuesOut[],
			uint32_t* samplePositionCountOut, 
			uint32_t* sampleValueCountOut) const {
			IndexType upperLeftGridSample;
			IndexType lowerRightGridSample;
			InputType transformedQuery;

			if (mWrapType == WrapType::CLAMP) {
				auto clampQuery = query;
				clampQuery.x -= mScale.x * static_cast<decltype(mScale.x)>(0.5);
				clampQuery.y -= mScale.y * static_cast<decltype(mScale.y)>(0.5);
				clampQuery = glm::min(glm::max(query, glm::zero<InputType>()), glm::one<InputType>());
				auto scaledClampQuery = clampQuery * mScaleInv;
				transformedQuery = scaledClampQuery;
				upperLeftGridSample.x = static_cast<decltype(upperLeftGridSample.x)>(scaledClampQuery.x);
				upperLeftGridSample.y = static_cast<decltype(upperLeftGridSample.y)>(scaledClampQuery.y);
				lowerRightGridSample.x = upperLeftGridSample.x + 1;
				lowerRightGridSample.y = upperLeftGridSample.y + 1;
				lowerRightGridSample = glm::min(lowerRightGridSample, IndexType(mGridSize.x - 1, mGridSize.y - 1));
			}
			else if (mWrapType == WrapType::REPEAT) {
				auto wrapQuery = query;
				wrapQuery.x -= mScale.x * static_cast<decltype(mScale.x)>(0.5);
				wrapQuery.y -= mScale.y * static_cast<decltype(mScale.y)>(0.5);
				wrapQuery.x = wrapQuery.x - std::floor(wrapQuery.x);
				wrapQuery.y = wrapQuery.y - std::floor(wrapQuery.y);
				auto scaled_mod_query = wrapQuery * mScaleInv;
				transformedQuery = scaled_mod_query;
				upperLeftGridSample.x = static_cast<decltype(upperLeftGridSample.x)>(scaled_mod_query.x) % mGridSize.x;
				upperLeftGridSample.y = static_cast<decltype(upperLeftGridSample.y)>(scaled_mod_query.y) % mGridSize.y;
				lowerRightGridSample.x = (upperLeftGridSample.x + 1) % mGridSize.x;
				lowerRightGridSample.y = (upperLeftGridSample.y + 1) % mGridSize.y;
			}

			samplePositionsOut[0].x = transformedQuery.x - std::floor(transformedQuery.x);
			samplePositionsOut[0].y = transformedQuery.y - std::floor(transformedQuery.y);

			sampleValuesOut[BilinearInterpolator::TOP_LEFT] = (*this)(upperLeftGridSample.x, upperLeftGridSample.y);
			sampleValuesOut[BilinearInterpolator::TOP_RIGHT] = (*this)(lowerRightGridSample.x, upperLeftGridSample.y);
			sampleValuesOut[BilinearInterpolator::BOTTOM_LEFT] = (*this)(upperLeftGridSample.x, lowerRightGridSample.y);
			sampleValuesOut[BilinearInterpolator::BOTTOM_RIGHT] = (*this)(lowerRightGridSample.x, lowerRightGridSample.y);

			*samplePositionCountOut = 1;
			*sampleValueCountOut = 4;
		}

		void loadpng(const std::string& filename) {
			loadRectSurfaceStorageFromPNG(filename, this);
		}

		void savepng(const std::string& filename) {
			saveRectSurfaceStorageToPNG(filename, *this);
		}

		template <typename ReturnType>
		friend void loadRectSurfaceStorageFromPNGinternal(const std::string& filename, RectSurfaceGridStorage<ReturnType>* out);
		template <typename ReturnType>
		friend void saveRectSurfaceStorageToPNGinternal(const std::string& filename, const RectSurfaceGridStorage<ReturnType>& f);
	};

	template <bool success>
	struct AllocateStorageGate;

	template <>
	struct AllocateStorageGate<false> {
		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX) {
			throw std::exception("Storage does not support these dimensions!");
		}

		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY) {
			throw std::exception("Storage does not support these dimensions!");
		}

		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ) {
			throw std::exception("Storage does not support these dimensions!");
		}
	};

	template <>
	struct AllocateStorageGate<true> {
		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX) {
			storage->allocate(sizeX);
		}

		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY) {
			storage->allocate(sizeX, sizeY);
		}

		template <typename StorageType>
		inline static void allocate(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ) {
			storage->allocate(sizeX, sizeY, sizeZ);
		}
	};

	template <typename ReturnType, 
		typename InputType, 
		typename StorageType,
		typename SamplerType,
		typename InterpolatorType>
	class SampleFunction {
	private:
		StorageType mStorage;

	public:
		typedef SampleFunction<ReturnType,
			InputType,
			StorageType,
			SamplerType,
			InterpolatorType> FunctionType;

		typedef ReturnType ReturnType;

		inline StorageType& storage() {
			return mStorage;
		}

		inline void allocate(const typename StorageType::SizeType& size) {
			mStorage.allocate(size);
		}

		inline void allocate(const uint32_t sizeX) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 1;
			AllocateStorageGate<same>::allocate(&mStorage, sizeX);
		}

		inline void allocate(const uint32_t sizeX, const uint32_t sizeY) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 2;
			AllocateStorageGate<same>::allocate(&mStorage, sizeX, sizeY);
		}

		inline void allocate(const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 3;
			AllocateStorageGate<same>::allocate(&mStorage, sizeX, sizeY, sizeZ);
		}

		inline void transition(const StorageMode mode) {
			mStorage.transition(mode);
		}

		ReturnType operator()(const InputType& input) const {
			static_assert(std::is_same_v<typename SamplerType::WeightType, typename InterpolatorType::WeightType>,
				"SamplerType and InterpolatorType do not share the same weight type!");

			InputType samplePositions[SamplerType::RequestedSamplePositions];
			ReturnType sampleValues[InterpolatorType::RequestedSampleValues];
			uint32_t samplePositionsCount;
			uint32_t sampleValuesCount;

			// Collect all samples near the requested input
			mStorage.collectSamples(input, samplePositions, sampleValues, &samplePositionsCount,
				&sampleValuesCount);

			typename SamplerType::WeightType weights;
			ReturnType result;
			
			// Compute the weights of those samples
			SamplerType::template computeWeights<InputType>(input, samplePositions, samplePositionsCount, &weights);
			// Interpolate between values
			InterpolatorType::template interp<ReturnType, InputType>(sampleValues, sampleValuesCount, weights, &result);

			return result;
		}

		inline void savepng(const std::string& filename) {
			mStorage.savepng(filename);
		}

		inline void loadpng(const std::string& filename) {
			mStorage.loadpng(filename);
		}
	};

	template <typename KernelType, typename FuncType, bool weighted = KernelType::HAS_WEIGHTS>
	struct KernelProc;

	template <typename KernelType, typename FuncType>
	struct KernelProc<KernelType, FuncType, false> {
		static void apply(KernelType& kernel, const FuncType& input,
			FuncType* output, uint32_t sampleCount) {
			input->transition(StorageMode::READ);
			output->transition(StorageMode::WRITE);
			auto& storage = output->storage();
			typedef typename ScalarType<typename FuncType::ReturnType>::RESULT NormType;
			auto norm_factor = static_cast<NormType>(1.0) / static_cast<NormType>(sampleCount);
			uint32_t outputSamples = static_cast<uint32_t>(storage.sampleCount());
			for (uint32_t i = 0; i < outputSamples; ++i) {
				auto outputLocation = storage.getSampleLocation(i);
				typename FuncType::ReturnType val = glm::zero<FuncType::ReturnType>();
				for (uint32_t iSample = 0; iSample < sampleCount; ++iSample) {
					auto sampleLocation = kernel.sample(outputLocation);
					val += input(sampleLocation);
				}
				storage[i] = val * norm_factor;
			}
			output->transition(StorageMode::READ);
		}
	};

	template <typename KernelType, typename FuncType>
	struct KernelProc<KernelType, FuncType, true> {
		static void apply(KernelType& kernel, const FuncType& input,
			FuncType* output, uint32_t sampleCount) {
			auto& storage = output->storage();
			output->transition(StorageMode::WRITE);
			input->transition(StorageMode::READ);
			typedef typename ScalarType<typename FuncType::ReturnType>::RESULT NormType;
			auto norm_factor = static_cast<NormType>(1.0) / static_cast<NormType>(sampleCount);
			uint32_t outputSamples = static_cast<uint32_t>(storage.sampleCount());
			for (size_t i = 0; i < outputSamples; ++i) {
				auto outputLocation = storage.getSampleLocation(i);
				typename FuncType::ReturnType val = glm::zero<FuncType::ReturnType>();
				for (uint32_t iSample = 0; iSample < sampleCount; ++iSample) {
					typename KernelType::WeightType weight;
					auto sampleLocation = kernel.sample(outputLocation, &weight);
					val += input(sampleLocation) * weight;
				}
				storage[i] = val * norm_factor;
			}
			output->transition(StorageMode::READ);
		}
	};

	template <typename VectorType = glm::vec2>
	class GaussianKernel2D {
	private:
		std::minstd_rand generator;
		std::normal_distribution<decltype(VectorType::x)> distribution;

	public:
		constexpr static bool HAS_WEIGHTS = false;
		
		typedef decltype(VectorType::x) ScalarType;

		GaussianKernel2D(decltype(VectorType::x) sigma) : 
			generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count()),
			distribution(0.0, sigma) {
		}

		VectorType sample(const VectorType& location) {
			auto sample1 = distribution(generator);
			auto sample2 = distribution(generator);

			return VectorType(location.x + sample1, location.y + sample2);
		}

		template <typename FuncType>
		void apply(const FuncType& input, FuncType* output, uint32_t sampleCount = DEFAULT_KERNEL_SAMPLE_COUNT) {
			KernelProc<GaussianKernel2D, FuncType>::apply(*this, input, output, sampleCount);
		}
	};
}