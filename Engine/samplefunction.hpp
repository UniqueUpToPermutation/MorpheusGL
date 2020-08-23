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
			static_assert(std::is_same_v<InputType, glm::vec2> ||
				std::is_same_v<InputType, glm::fvec2> ||
				std::is_same_v<InputType, glm::dvec2>);

			auto top = (1 - weights.x) * sampleValues[TOP_LEFT] + weights.x * sampleValues[TOP_RIGHT];
			auto bottom = (1 - weights.x) * sampleValues[BOTTOM_LEFT] + weights.x * sampleValues[BOTTOM_RIGHT];
			*out = top * (1 - weights.y) + bottom * weights.y;
		}
	}; 

	class RectSurfaceBilinearSampler {
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

	template <typename ReturnType,
		typename InputType,
		typename StorageType,
		typename SamplerType,
		typename InterpolatorType = typename SamplerType::DefaultInterpolator>
		class SampleFunction;

	template <typename ReturnType>
	using Function2D = SampleFunction<ReturnType, glm::vec2,
		RectSurfaceGridStorage<ReturnType, glm::vec2>,
		RectSurfaceBilinearSampler>;

	template <typename OutputType, typename InputType>
	class RectSurfaceGridStorage {
	public:
		typedef glm::uvec2 SizeType;
		typedef glm::ivec2 IndexType;

	private:
		std::vector<OutputType> mGrid;
		InputType mScale;
		InputType mScaleInv;
		InputType mClampBoundsUpper;
		InputType mClampBoundsLower;
		SizeType mGridSize;
		WrapType mWrapType;

	public:

		WrapType& wrap() { return mWrapType; }

		void init(const SizeType& gridSize) {
			mGridSize = gridSize;
			mScale.x = static_cast<decltype(mScale.x)>(1.0 / mGridSize.x);
			mScale.y = static_cast<decltype(mScale.y)>(1.0 / mGridSize.y);
			mScaleInv.x = static_cast<decltype(mScaleInv.x)>(mGridSize.x);
			mScaleInv.y = static_cast<decltype(mScaleInv.y)>(mGridSize.y);
			mClampBoundsLower = mScale / static_cast<decltype(mScale.x)>(2.0);
			mClampBoundsUpper = InputType(1.0f, 1.0f) - mScale / static_cast <decltype(mScale.x)>(2.0);
			mGrid.resize(gridSize.x * gridSize.y);
		}

		void init(const uint32_t sizeX, const uint32_t sizeY) {
			init(SizeType(sizeX, sizeY));
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

		OutputType& operator[](const uint32_t i) {
			return mGrid[i];
		}

		const OutputType& operator[](const uint32_t i) const {
			return mGrid[i];
		}

		InputType getSampleLocation(const IndexType& pos) const {
			InputType result;
			result.x = static_cast<decltype(result.x)>((static_cast<decltype(mScale.x)>(pos.x) + 0.5) * mScale.x);
			result.y = static_cast<decltype(result.y)>((static_cast<decltype(mScale.y)>(pos.y) + 0.5) * mScale.y);
			return result;
		}

		IndexType getIndexCoords(const uint32_t index) const {
			IndexType result;
			result.x = index % mGridSize.x;
			result.y = index / mGridSize.x;
			return result;
		}

		InputType getSampleLocation(const uint32_t index) const {
			InputType result;
			IndexType coords = getIndexCoords(index);
			result.x = static_cast<decltype(result.x)>((static_cast<decltype(mScale.x)>(coords.x) + 0.5) * mScale.x);
			result.y = static_cast<decltype(result.y)>((static_cast<decltype(mScale.y)>(coords.y) + 0.5) * mScale.y);
			return result;
		}

		OutputType& sampleValue(const IndexType& pos) {
			return mGrid[pos.x + mGridSize.x * pos.y];
		}

		OutputType& sampleValue(const uint32_t index) {
			return mGrid[index];
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

		template <typename ReturnType>
		friend void loadFunction2DfromPNGinternal(const std::string& filename, Function2D<ReturnType>* out);
		template <typename ReturnType>
		friend void saveFunction2DtoPNGinternal(const std::string& filename, const Function2D<ReturnType>& f);
	};

	void loadFunction2DfromPNG(const std::string& filename, Function2D<float>* out);
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec2>* out);
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec3>* out);
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec4>* out);

	void saveFunction2DtoPNG(const std::string& filename, const Function2D<float>& f);
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec2>& f);
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec3>& f);
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec4>& f);

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

		inline void initStorage(const typename StorageType::SizeType& size) {
			mStorage.init(size);
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
			saveFunction2DtoPNG(filename, *this);
		}

		inline void loadpng(const std::string& filename) {
			loadFunction2DfromPNG(filename, this);
		}

		template <typename ReturnType>
		friend void loadFunction2DfromPNGinternal(const std::string& filename, Function2D<ReturnType>* out);
		template <typename ReturnType>
		friend void saveFunction2DtoPNGinternal(const std::string& filename, const Function2D<ReturnType>& f);
	};

	template <typename KernelType, typename FuncType, bool weighted = KernelType::HAS_WEIGHTS>
	struct KernelProc;

	template <typename KernelType, typename FuncType>
	struct KernelProc<KernelType, FuncType, false> {
		static void apply(KernelType& kernel, const FuncType& input,
			FuncType* output, uint32_t sampleCount) {
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
		}
	};

	template <typename KernelType, typename FuncType>
	struct KernelProc<KernelType, FuncType, true> {
		static void apply(KernelType& kernel, const FuncType& input,
			FuncType* output, uint32_t sampleCount) {
			auto& storage = output->storage();
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