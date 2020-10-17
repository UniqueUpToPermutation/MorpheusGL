#pragma once

#include <engine/core.hpp>
#include <engine/texture.hpp>

#include <type_traits>
#include <chrono>
#include <random>
#include <functional>

#include <Eigen/Dense>

#define DEFAULT_KERNEL_SAMPLE_COUNT 100u

namespace Morpheus {
	void writeCubemapSideBaseLevel(Texture* tex, size_t i_side, GLenum format, const std::vector<float>& mData);
	void writeTextureBaseLevel(Texture* tex, GLenum format, const std::vector<float>& mData);

	void readCubemapSideBaseLevel(Texture* tex, size_t i_side, uint32_t element_length, std::vector<float>* mData);
	void readTextureBaseLevel(Texture* tex, uint32_t element_length, std::vector<float>* mData);

	template <typename VecType>
	struct get_scalar_t {
		typedef typename VecType::value_type scalar_t;
	};

	template <>
	struct get_scalar_t<Eigen::MatrixXd> {
		typedef double scalar_t;
	};

	template <>
	struct get_scalar_t<Eigen::MatrixXf> {
		typedef float scalar_t;
	};

	template <>
	struct get_scalar_t<Eigen::VectorXd> {
		typedef double scalar_t;
	};

	template <>
	struct get_scalar_t<Eigen::VectorXf> {
		typedef float scalar_t;
	};

	template <>
	struct get_scalar_t<float> {
		typedef float scalar_t;
	};

	template <>
	struct get_scalar_t<double> {
		typedef double scalar_t;
	};

	template <typename OutputType>
	constexpr size_t getElementLength() {
		return OutputType::length();
	}

	template <>
	constexpr size_t getElementLength<float>() {
		return 1;
	}

	template <>
	constexpr size_t getElementLength<double>() {
		return 1;
	}

	template <typename OutputType>
	bool checkValidFormat(const GLenum format) {
		constexpr size_t element_length = getElementLength<OutputType>();
		if constexpr (element_length == 1) {
			return format == GL_R8 || format == GL_R16 || format == GL_R32F;
		}
		else if constexpr (element_length == 2) {
			return format == GL_RG8 || format == GL_RG16 || format == GL_RG32F;
		}
		else if constexpr (element_length == 3) {
			return format == GL_RGB8 || format == GL_RGB16 || format == GL_RGB32F;
		}
		else if constexpr (element_length == 4) {
			return format == GL_RGBA8 || format == GL_RGBA16 || format == GL_RGBA32F;
		}
	}

	template <typename OutputType>
	constexpr GLenum defaultTextureFormatForFunctionOutput() {
		constexpr size_t element_length = getElementLength<OutputType>();
		if constexpr (element_length == 1) {
			return GL_R8;
		}
		else if constexpr (element_length == 2) {
			return GL_RG8;
		}
		else if constexpr (element_length == 3) {
			return GL_RGB8;
		}
		else if constexpr (element_length == 4) {
			return GL_RGBA8;
		}
	}

	template <typename OutputType>
	inline void funcSampleCopyInto(OutputType* dest, float* src) {
		constexpr size_t element_count = getElementLength<OutputType>();
		for (uint32_t i = 0; i < element_count; ++i, ++src) {
			(*dest)[i] = *src;
		}
	}

	template <>
	inline void funcSampleCopyInto<float>(float* dest, float* src) {
		*dest = *src;
	}

	template <>
	inline void funcSampleCopyInto<double>(double* dest, float* src) {
		*dest = *src;
	}

	template <typename OutputType>
	inline void funcSampleCopyOutOf(float* dest, OutputType* src) {
		constexpr uint32_t element_count = (uint32_t)getElementLength<OutputType>();
		for (uint32_t i = 0; i < element_count; ++i, ++dest) {
			*dest = (*src)[i];
		}
	}

	template <>
	inline void funcSampleCopyOutOf<float>(float* dest, float* src) {
		*dest = *src;
	}

	template <>
	inline void funcSampleCopyOutOf<double>(float* dest, double* src) {
		*dest = (float)*src;
	}
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
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<double>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec2>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec3>* out);
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec4>* out);

	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<float>& f);
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<double>& f);
	void saveCubemapeStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec2>& f);
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec3>& f);
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f);

	template <typename OutputType_, typename InputType_>
	class CubemapStorage {
	public:
		typedef glm::uvec2 SizeType;
		typedef glm::ivec3 IndexType;
		typedef OutputType_ OutputType;
		typedef InputType_ InputType;

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
		constexpr static uint32_t SAMPLE_POSITION_DIMENSION = 3;
		constexpr static bool SUPPORTS_POLAR_GRID = true;

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

		void allocate() {
			mCubemapData.resize(mGridSize.x * mGridSize.y * 6);
			mStorageMode = StorageMode::ALLOCATED;
		}

		void deallocate() {
			mCubemapData.clear();
			mStorageMode = StorageMode::UNALLOCATED;
		}

		// Must be called before samples are assigned to
		void init(const SizeType& gridSize, bool bAllocate = true) {
			mGridSize = gridSize;
			// Make room for the padding
			mGridSize.x += 2;
			mGridSize.y += 2;

			mScale.x = static_cast<decltype(mScale.x)>(1.0 / gridSize.x);
			mScale.y = static_cast<decltype(mScale.y)>(1.0 / gridSize.y);
			mScaleInv.x = static_cast<decltype(mScale.x)>(gridSize.x);
			mScaleInv.y = static_cast<decltype(mScale.y)>(gridSize.y);

			if (bAllocate)
				allocate();
		}

		void init(const uint32_t sizeX, const uint32_t sizeY, bool bAllocate = true) {
			init(SizeType(sizeX, sizeY), bAllocate);
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
					result.u = -query.z / absQuery.x;
					result.v = -query.y / absQuery.x;
				}
				else {
					result.face = FACE_NEGATIVE_X;
					result.u = query.z / absQuery.x;
					result.v = -query.y / absQuery.x;
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
					result.v = -query.y / absQuery.z;
				}
				else {
					result.face = FACE_NEGATIVE_Z;
					result.u = -query.x / absQuery.z;
					result.v = -query.y / absQuery.z;
				}
			}

			// Normalize to [0, 1] texture coordinates
			result.u = static_cast<scalar_t>(0.5) * (result.u + static_cast<scalar_t>(1.0));
			result.v = static_cast<scalar_t>(0.5) * (result.v + static_cast<scalar_t>(1.0));

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

			scalar_t scaled_u = uv.u * mScaleInv.x - static_cast<scalar_t>(0.5);
			scalar_t scaled_v = uv.v * mScaleInv.y - static_cast<scalar_t>(0.5);

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
			default:
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
			v = v * static_cast<scalar_t>(2.0) - static_cast<scalar_t>(1.0);

			switch (pos.z) {
			case FACE_POSITIVE_X:
				result.x = 1.0;
				result.z = -u;
				result.y = -v;
				break;
			case FACE_NEGATIVE_X:
				result.x = -1.0;
				result.z = u;
				result.y = -v;
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
				result.y = -v;
				result.z = 1.0;
				break;
			case FACE_NEGATIVE_Z:
				result.x = -u;
				result.y = -v;
				result.z = -1.0;
				break;
			default:
				throw std::runtime_error("pos.z is an invalid face!");
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

		template <typename VectorType>
		void createGrid(VectorType* outX, VectorType* outY, VectorType* outZ) const {
			size_t size = sampleCount();
			outX->resize(size);
			outY->resize(size);
			outZ->resize(size);

			for (size_t i = 0; i < size; ++i) {
				auto loc = getSampleLocation(i);
				(*outX)(i) = loc.x;
				(*outY)(i) = loc.y;
				(*outZ)(i) = loc.z;
			}
		}

		template <typename VectorType>
		void createPolarGrid(VectorType* outPhi, VectorType* outTheta) const {
			size_t size = sampleCount();
			outPhi->resize(size);
			outTheta->resize(size);

			for (size_t i = 0; i < size; ++i) {
				auto loc = getSampleLocation(i);
				(*outPhi)(i) = std::atan2(loc.y, loc.x);
				(*outTheta)(i) = std::acos(loc.z);
			}
		}

		void writeToTexture(Texture* tex) {
			assert(mStorageMode == StorageMode::READ);
			assert(tex->textureType() == TextureType::CUBE_MAP);
			if (tex->width() != width() || tex->height() != height()) {
				throw std::invalid_argument("Texture has incorrect dimensions!");
			}

			std::vector<float> mData;
			constexpr size_t element_length = getElementLength<OutputType>();
			mData.resize(element_length * (size_t)width() * (size_t)height());

			// Copy data
			uint32_t width_ = width();
			uint32_t height_ = height();
			size_t i_read = 0;
			for (uint32_t i_side = 0; i_side < 6; ++i_side) {
				size_t i_write = 0;

				for (uint32_t y = 0; y < height_; ++y) {
					for (uint32_t x = 0; x < width_; ++x) {
						funcSampleCopyOutOf<OutputType>(&mData[i_write], &(*this)(x, y, i_side));
						i_write += element_length;
					}
				}

				GLenum format;
				if constexpr (element_length == 1) {
					format = GL_RED;
				}
				else if constexpr (element_length == 2) {
					format = GL_RG;
				}
				else if constexpr (element_length == 3) {
					format = GL_RGB;
				}
				else if constexpr (element_length == 4) {
					format = GL_RGBA;
				}

				// Copy data into texture
				writeCubemapSideBaseLevel(tex, i_side, format, mData);
			}
			tex->genMipmaps();
		}

		Texture* toTextureUnmanaged(const GLenum format,
			ContentFactory<Texture>* factory) {
			assert(mStorageMode == StorageMode::READ);
			checkValidFormat<OutputType>(format);
			Texture* tex = factory->makeCubemapUnmanaged(width(), height(), format);
			GL_ASSERT;
			writeToTexture(tex);
			return tex;
		}

		Texture* toTexture(INodeOwner* parent, const GLenum format,
			ContentFactory<Texture>* factory) {
			assert(mStorageMode == StorageMode::READ);
			checkValidFormat<OutputType>(format);
			Texture* tex;
			GL_ASSERT;
			tex = factory->makeCubemap(parent, width(), height(), format);
			GL_ASSERT;
			writeToTexture(tex);
			return tex;
		}

		void fromTexture(Texture* tex) {
			assert(tex->textureType() == TextureType::CUBE_MAP);

			init(tex->width(), tex->height(), true);
			transition(StorageMode::WRITE);

			uint32_t width_ = width();
			uint32_t height_ = height();
			size_t i_read = 0;
			std::vector<float> mData;

			constexpr uint32_t element_length = (uint32_t)getElementLength<OutputType>();

			for (uint32_t i_side = 0; i_side < 6; ++i_side) {
				size_t i_read = 0;

				readCubemapSideBaseLevel(tex, i_side, element_length, &mData);

				for (uint32_t y = 0; y < height_; ++y) {
					for (uint32_t x = 0; x < width_; ++x) {
						funcSampleCopyInto<OutputType>(&(*this)(x, y, i_side), &mData[i_read]);
						i_read += element_length;
					}
				}
			}

			transition(StorageMode::READ);
		}

		void transform(const std::function<OutputType(const OutputType&)>& f) {
			for (auto& sample : mCubemapData) {
				sample = f(sample);
			}
		}

		template <typename ReturnType>
		friend void loadCubemapStorageFromPNGinternal(const std::string& filename, CubemapStorage<ReturnType>* out);
		template <typename ReturnType>
		friend void saveCubemapStorageToPNGinternal(const std::string& filename, const CubemapStorage<ReturnType>& f);
	};

	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<float>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<double>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec2>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec3>* out);
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec4>* out);

	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<float>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<double>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec2>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec3>& f);
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec4>& f);

	template <typename OutputType_, typename InputType_>
	class RectSurfaceGridStorage {
	public:
		typedef glm::uvec2 SizeType;
		typedef glm::ivec2 IndexType;
		typedef OutputType_ OutputType;
		typedef InputType_ InputType;

		constexpr static uint32_t ALLOCATE_DIMENSION = 2;
		constexpr static uint32_t SAMPLE_POSITION_DIMENSION = 2;
		constexpr static bool SUPPORTS_POLAR_GRID = false;

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

		void allocate() {
			mGrid.resize((size_t)mGridSize.x * (size_t)mGridSize.y);
			mStorageMode = StorageMode::ALLOCATED;
		}

		void deallocate() {
			mGrid.clear();
			mStorageMode = StorageMode::UNALLOCATED;
		}

		void init(const SizeType& gridSize, bool bAllocate = true) {
			mGridSize = gridSize;
			mScale.x = static_cast<decltype(mScale.x)>(1.0 / mGridSize.x);
			mScale.y = static_cast<decltype(mScale.y)>(1.0 / mGridSize.y);
			mScaleInv.x = static_cast<decltype(mScaleInv.x)>(mGridSize.x);
			mScaleInv.y = static_cast<decltype(mScaleInv.y)>(mGridSize.y);
			mClampBoundsLower = mScale / static_cast<decltype(mScale.x)>(2.0);
			mClampBoundsUpper = InputType(1.0f, 1.0f) - mScale / static_cast <decltype(mScale.x)>(2.0);
			
			if (bAllocate)
				allocate();
		}

		void init(const uint32_t sizeX, const uint32_t sizeY, bool bAllocate = true) {
			init(SizeType(sizeX, sizeY), bAllocate);
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

		template <typename VectorType>
		void createGrid(VectorType* outX, VectorType* outY) const {
			size_t size = sampleCount();
			outX->resize(size);
			outY->resize(size);

			for (size_t i = 0; i < size; ++i) {
				auto loc = getSampleLocation(i);
				(*outX)(i) = loc.x;
				(*outY)(i) = loc.y;
			}
		}

		void writeToTexture(Texture* tex) {
			assert(mStorageMode == StorageMode::READ);
			assert(tex->textureType() == TextureType::TEXTURE_2D);
			if (tex->width() != width() || tex->height() != height()) {
				throw std::invalid_argument("Texture has incorrect dimensions!");
			}

			constexpr size_t element_length = getElementLength<OutputType>();
			std::vector<float> mData;
			mData.resize(element_length * (size_t)width() * (size_t)height());
			
			// Copy data
			for (size_t i_write = 0, i_read = 0; i_read < mGrid.size(); ++i_read, i_write += element_length) {
				funcSampleCopyOutOf<OutputType>(&mData[i_write], &mGrid[i_read]);
			}

			GLenum format;
			if constexpr (element_length == 1) {
				format = GL_RED;
			}
			else if constexpr (element_length == 2) {
				format = GL_RG;
			}
			else if constexpr (element_length == 3) {
				format = GL_RGB;
			}
			else if constexpr (element_length == 4) {
				format = GL_RGBA;
			}

			// Copy data into texture
			writeTextureBaseLevel(tex, format, mData);
			tex->genMipmaps();
		}

		void fromTexture(Texture* tex) {
			assert(tex->textureType() == TextureType::CUBE_MAP);

			init(tex->width(), tex->height(), true);
			transition(StorageMode::WRITE);

			uint32_t width_ = width();
			uint32_t height_ = height();
			size_t i_read = 0;

			constexpr size_t element_length = getElementLength<OutputType>();
			std::vector<float> mData;
			readTextureBaseLevel(tex, (uint32_t)element_length, &mData);

			for (size_t i_write = 0, i_read = 0; i_write < mGrid.size(); ++i_write, i_read += element_length) {
				funcSampleCopyInto<OutputType>(&mGrid[i_write], &mData[i_read]);
			}

			transition(StorageMode::READ);
		}

		Texture* toTextureUnmanaged(const GLenum format,
			ContentFactory<Texture>* factory) {
			assert(mStorageMode == StorageMode::READ);
			checkValidFormat<OutputType>(format);
			Texture* tex = factory->makeTexture2DUnmanaged(width(), height(), format);
			GL_ASSERT;
			writeToTexture(tex);
			return tex;
		}

		Node toTexture(Texture** out,
			const GLenum format,
			ContentFactory<Texture>* factory) {
			assert(mStorageMode == StorageMode::READ);
			checkValidFormat<OutputType>(format);
			Texture* tex;
			Node texNode = factory->makeTexture2D(&tex, width(), height(), format);
			GL_ASSERT;
			writeToTexture(tex);
			if (out)
				*out = tex;
			return texNode;
		}

		void transform(const std::function<OutputType(const OutputType&)>& f) {
			for (auto& sample : mGrid) {
				sample = f(sample);
			}
		}

		template <typename ReturnType>
		friend void loadRectSurfaceStorageFromPNGinternal(const std::string& filename, RectSurfaceGridStorage<ReturnType>* out);
		template <typename ReturnType>
		friend void saveRectSurfaceStorageToPNGinternal(const std::string& filename, const RectSurfaceGridStorage<ReturnType>& f);
	};

	template <bool success>
	struct InitStorageGate;

	template <>
	struct InitStorageGate<false> {
		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, bool bAllocate) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}

		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, bool bAllocate) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}

		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ, bool bAllocate) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}
	};

	template <>
	struct InitStorageGate<true> {
		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, bool bAllocate) {
			storage->init(sizeX, bAllocate);
		}

		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, bool bAllocate) {
			storage->init(sizeX, sizeY, bAllocate);
		}

		template <typename StorageType>
		inline static void init(StorageType* storage, const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ, bool bAllocate) {
			storage->init(sizeX, sizeY, sizeZ, bAllocate);
		}
	};

	template <bool success>
	struct CreateGridGate;

	template <>
	struct CreateGridGate<false> {
		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}

		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX, VectorType* outY) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}

		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX, VectorType* outY, VectorType* outZ) {
			throw std::runtime_error("Storage does not support these dimensions!");
		}
	};

	template<>
	struct CreateGridGate<true> {
		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX) {
			storage->createGrid(outX);
		}

		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX, VectorType* outY) {
			storage->createGrid(outX, outY);
		}

		template <typename StorageType, typename VectorType>
		inline static void createGrid(const StorageType* storage, VectorType* outX, VectorType* outY, VectorType* outZ) {
			storage->createGrid(outX, outY, outZ);
		}
	};

	template <bool success>
	struct CreatePolarGridGate;

	template <>
	struct CreatePolarGridGate<false> {
		template <typename StorageType, typename VectorType>
		inline static void createPolarGrid(const StorageType* storage, VectorType* outPhi, VectorType* outTheta) {
			throw std::runtime_error("Storage does not support polar coordinates!");
		}
	};

	template <>
	struct CreatePolarGridGate<true> {
		template <typename StorageType, typename VectorType>
		inline static void createPolarGrid(const StorageType* storage, VectorType* outPhi, VectorType* outTheta) {
			storage->createPolarGrid(outPhi, outTheta);
		}
	};

	template <typename ReturnType_, 
		typename InputType_, 
		typename StorageType_,
		typename SamplerType_,
		typename InterpolatorType_>
	class SampleFunction {
	public:
		typedef InputType_ InputType;
		typedef ReturnType_ ReturnType;
		typedef StorageType_ StorageType;
		typedef SamplerType_ SamplerType;
		typedef InterpolatorType_ InterpolatorType;

		typedef SampleFunction<ReturnType,
			InputType,
			StorageType,
			SamplerType,
			InterpolatorType> FunctionType;

	private:
		StorageType mStorage;

	public:
		inline StorageType& storage() {
			return mStorage;
		}

		inline void allocate() {
			mStorage.allocate();
		}

		inline void deallocate() {
			mStorage.deallocate();
		}

		inline uint32_t width() const {
			return mStorage.width();
		}

		inline uint32_t height() const {
			return mStorage.height();
		}

		inline uint32_t depth() const {
			return mStorage.depth();
		}

		inline void init(const typename StorageType::SizeType& size, bool bAllocate = true) {
			mStorage.init(size, bAllocate);
		}

		inline void init1d(const uint32_t sizeX, bool bAllocate = true) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 1;
			InitStorageGate<same>::init(&mStorage, sizeX, bAllocate);
		}

		inline void init2d(const uint32_t sizeX, const uint32_t sizeY, bool bAllocate = true) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 2;
			InitStorageGate<same>::init(&mStorage, sizeX, sizeY, bAllocate);
		}

		inline void init3d(const uint32_t sizeX, const uint32_t sizeY, const uint32_t sizeZ, bool bAllocate = true) {
			constexpr bool same = StorageType::ALLOCATE_DIMENSION == 3;
			InitStorageGate<same>::init(&mStorage, sizeX, sizeY, sizeZ, bAllocate);
		}

		template <typename VectorType>
		inline void createGrid(VectorType* outX) const {
			constexpr bool same = StorageType::SAMPLE_POSITION_DIMENSION == 1;
			CreateGridGate<same>::createGrid(&mStorage, outX);
		}

		template <typename VectorType>
		inline void createGrid(VectorType* outX, VectorType* outY) const {
			constexpr bool same = StorageType::SAMPLE_POSITION_DIMENSION == 2;
			CreateGridGate<same>::createGrid(&mStorage, outX, outY);
		}

		template <typename VectorType>
		inline void createGrid(VectorType* outX, VectorType* outY, VectorType* outZ) const {
			constexpr bool same = StorageType::SAMPLE_POSITION_DIMENSION == 3;
			CreateGridGate<same>::createGrid(&mStorage, outX, outY, outZ);
		}

		template <typename VectorType>
		inline void createPolarGrid(VectorType* outPhi, VectorType* outTheta) const {
			constexpr bool success = StorageType::SUPPORTS_POLAR_COORDINATES;
			CreatePolarGridGate<success>::createPolarGrid(&mStorage, outPhi, outTheta);
		}

		template <typename MatrixType>
		inline void toSampleMatrix(MatrixType* out) {
			static_assert(std::is_same_v<ReturnType, typename StorageType::OutputType>, 
				"Function and function storage must accept and return the same types!");
			static_assert(std::is_same_v<InputType, typename StorageType::InputType>,
				"Function and function storage must accept and return the same types!");

			assert(mStorage.getMode() == StorageMode::READ);

			size_t size = mStorage.sampleCount();
			constexpr uint32_t element_length = (uint32_t)getElementLength<ReturnType>();
			out->resize(size, ReturnType::length());

			for (uint32_t j = 0; j < element_length; ++j) {
				for (size_t i = 0; i < size; ++i) {
					(*out)(i, j) = mStorage[i][j];
				}
			}
		}

		template <typename MatrixType>
		inline void fromSampleMatrix(const MatrixType& in) {
			static_assert(std::is_same_v<ReturnType, typename StorageType::OutputType>,
				"Function and function storage must accept and return the same types!");
			static_assert(std::is_same_v<InputType, typename StorageType::InputType>,
				"Function and function storage must accept and return the same types!");

			size_t size = mStorage.sampleCount();

			assert(in.rows() == size && in.cols() == ReturnType::length());
			assert(mStorage.getMode() == StorageMode::WRITE);

			constexpr uint32_t element_length = (uint32_t)getElementLength<ReturnType>();

			for (uint32_t j = 0; j < element_length; ++j) {
				for (size_t i = 0; i < size; ++i) {
					mStorage[i][j] = in(i, j);
				}
			}
		}

		template <typename VectorType>
		inline void fromSampleVector(const VectorType& in) {
			static_assert(std::is_same_v<ReturnType, typename StorageType::OutputType>,
				"Function and function storage must accept and return the same types!");
			static_assert(std::is_same_v<InputType, typename StorageType::InputType>,
				"Function and function storage must accept and return the same types!");

			assert(mStorage.getMode() == StorageMode::WRITE);
			
			size_t size = mStorage.sampleCount();

			assert(in.size() == size);
			assert((std::is_same_v<ReturnType, float> || std::is_same_v<ReturnType, double>));

			for (size_t i = 0; i < size; ++i) {
				mStorage[i] = static_cast<decltype(mStorage[i])>(in(i));
			}
		}

		inline void transform(const std::function<ReturnType(const ReturnType&)>& f) {
			mStorage.transform(f);
		}

		inline Texture* toTextureUnmanaged(
			const GLenum format = defaultTextureFormatForFunctionOutput<ReturnType>(),
			ContentFactory<Texture>* factory = content()->getFactory<Texture>()) {
			return mStorage.toTextureUnmanaged(format, factory);
		}

		inline Texture* toTexture(const GLenum format = defaultTextureFormatForFunctionOutput<ReturnType>(),
			ContentFactory<Texture>* factory = content()->getFactory<Texture>()) {
			return mStorage.toTexture(format, factory);
		}

		inline void fromTexture(Texture* texture) {
			mStorage.fromTexture(texture);
		}

		inline void writeToTexture(Texture* target) {
			mStorage.writeToTexture(target);
		}

		inline void transition(const StorageMode mode) {
			mStorage.transition(mode);
		}

		inline StorageMode getStorageMode() const {
			return mStorage.getMode();
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
			assert(input.getStorageMode() == StorageMode::READ);
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
			assert(input.getStorageMode() == StorageMode::READ);
			output->transition(StorageMode::WRITE);
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
	class GaussianMonteCarloKernel2D {
	private:
		std::minstd_rand generator;
		std::normal_distribution<decltype(VectorType::x)> distribution;

	public:
		constexpr static bool HAS_WEIGHTS = false;
		
		typedef decltype(VectorType::x) ScalarType;

		GaussianMonteCarloKernel2D(decltype(VectorType::x) sigma) : 
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
			KernelProc<GaussianMonteCarloKernel2D, FuncType>::apply(*this, input, output, sampleCount);
		}
	};
}