#pragma once

#include "core.hpp"
#include "texture.hpp"

#include <type_traits>

namespace Morpheus {
	enum class RectLinearSample {
		TOP_LEFT = 0,
		TOP_RIGHT = 1,
		BOTTOM_LEFT = 2,
		BOTTOM_RIGHT = 3
	};

	enum class RectLinearSamplePosition {
		TOP_LEFT = 0,
		SCALE_INV = 1
	};

	class BilinearInterpolator {
	public:
		constexpr static uint32_t RequestedSampleValues = 4;
		typedef glm::vec2 WeightType;

		template <typename ReturnType, typename InputType>
		static void interp(const ReturnType sampleValues[], const uint32_t sampleCount, 
			const WeightType& weights, ReturnType* out) {
			assert(sampleCount == RequestedSampleValues);
			static_assert(std::is_same_v<InputType, glm::vec2> ||
				std::is_same_v<InputType, glm::fvec2> ||
				std::is_same_v<InputType, glm::dvec2>);

			auto top = (1 - weights.x) * sampleValues[(int)RectLinearSample::TOP_LEFT] +
				weights.x * sampleValues[(int)RectLinearSample::TOP_RIGHT];
			auto bottom = (1 - weights.x) * sampleValues[(int)RectLinearSample::BOTTOM_LEFT] +
				weights.x * sampleValues[(int)RectLinearSample::BOTTOM_RIGHT];
			*out = top * (1 - weights.y) + bottom * weights.y;
		}
	}; 

	class RectSurfaceBilinearSampler {
	public:
		typedef BilinearInterpolator DefaultInterpolator;
		typedef glm::vec2 WeightType;

		constexpr static uint32_t RequestedSamplePositions = 2;

		template <typename InputType>
		static void computeWeights(const InputType queryPosition,
			const InputType samplePositions[],
			const uint32_t samplePositionCount,
			WeightType* weightsOut) {
			assert(samplePositionCount == RequestedSamplePositions);

			auto scale_inv = samplePositions[(int)RectLinearSamplePosition::SCALE_INV];
			auto u = (queryPosition.x - samplePositions[(int)RectLinearSamplePosition::TOP_LEFT].x) * scale_inv.x;
			auto v = (queryPosition.y - samplePositions[(int)RectLinearSamplePosition::TOP_LEFT].y) * scale_inv.y;
			weightsOut->x = u;
			weightsOut->y = v;
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
			uint32_t* sampleValueCountOut) {
			IndexType upperLeftGridSample;
			IndexType lowerRightGridSample;

			if (mWrapType == WrapType::CLAMP) {
				auto clamp_query = glm::min(glm::max(query, glm::zero<InputType>()), glm::one<InputType>());
				auto scaled_query = clamp_query * mScaleInv;
				scaled_query.x -= 0.5;
				scaled_query.y -= 0.5;
				upperLeftGridSample.x = static_cast<decltype(upperLeftGridSample.x)>(scaled_query.x);
				upperLeftGridSample.y = static_cast<decltype(upperLeftGridSample.y)>(scaled_query.y);
				upperLeftGridSample = glm::min(upperLeftGridSample, IndexType(mGridSize.x - 1, mGridSize.y - 1));
				lowerRightGridSample.x = upperLeftGridSample.x + 1;
				lowerRightGridSample.y = upperLeftGridSample.y + 1;
			}
			else if (mWrapType == WrapType::REPEAT) {
				auto wrap_query = query;
				wrap_query.x = wrap_query.x - std::floor(wrap_query.x);
				wrap_query.y = wrap_query.y - std::floor(wrap_query.y);
				auto scaled_query = wrap_query * mScaleInv;
				scaled_query.x -= 0.5;
				scaled_query.y -= 0.5;
				upperLeftGridSample.x = static_cast<decltype(upperLeftGridSample.x)>(scaled_query.x);
				upperLeftGridSample.y = static_cast<decltype(upperLeftGridSample.y)>(scaled_query.y);
				lowerRightGridSample.x = (upperLeftGridSample.x + 1) % mGridSize.x;
				lowerRightGridSample.y = (upperLeftGridSample.y + 1) % mGridSize.y;
			}

			samplePositionsOut[(int)RectLinearSamplePosition::TOP_LEFT].x = upperLeftGridSample.x * mScale.x;
			samplePositionsOut[(int)RectLinearSamplePosition::TOP_LEFT].y = upperLeftGridSample.y * mScale.y;
			samplePositionsOut[(int)RectLinearSamplePosition::SCALE_INV] = mScaleInv;

			sampleValuesOut[(int)RectLinearSample::TOP_LEFT] = (*this)(upperLeftGridSample.x, upperLeftGridSample.y);
			sampleValuesOut[(int)RectLinearSample::TOP_RIGHT] = (*this)(lowerRightGridSample.x, upperLeftGridSample.y);
			sampleValuesOut[(int)RectLinearSample::BOTTOM_LEFT] = (*this)(upperLeftGridSample.x, lowerRightGridSample.y);
			sampleValuesOut[(int)RectLinearSample::BOTTOM_RIGHT] = (*this)(lowerRightGridSample.x, lowerRightGridSample.y);

			*samplePositionCountOut = 2;
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

		inline StorageType& storage() {
			return mStorage;
		}

		inline void initStorage(const typename StorageType::SizeType& size) {
			mStorage.init(size);
		}

		ReturnType operator()(const InputType& input) {
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
}