#include "samplefunction.hpp"

#include "lodepng/lodepng.h"

#include <iostream>

namespace Morpheus {

	inline void loadPixel(float* dest, const uint8_t* src) {
		*dest = ((float)*src) / 255.0f;
	}

	inline void loadPixel(glm::vec2* dest, const uint8_t* src) {
		dest->x = ((float)src[0]) / 255.0f;
		dest->y = ((float)src[1]) / 255.0f;
	}

	inline void loadPixel(glm::vec3* dest, const uint8_t* src) {
		dest->x = ((float)src[0]) / 255.0f;
		dest->y = ((float)src[1]) / 255.0f;
		dest->z = ((float)src[2]) / 255.0f;
	}

	inline void loadPixel(glm::vec4* dest, const uint8_t* src) {
		dest->x = ((float)src[0]) / 255.0f;
		dest->y = ((float)src[1]) / 255.0f;
		dest->z = ((float)src[2]) / 255.0f;
		dest->w = ((float)src[3]) / 255.0f;
	}

	inline void savePixel(uint8_t* dest, const float* src) {
		dest[0] = (uint8_t)((*src) * 255.0f);
		dest[1] = 0;
		dest[2] = 0;
		dest[3] = 255u;
	}

	inline void savePixel(uint8_t* dest, const glm::vec2* src) {
		dest[0] = (uint8_t)(src->x * 255.0f);
		dest[1] = (uint8_t)(src->y * 255.0f);
		dest[2] = 0;
		dest[3] = 255u;
	}

	inline void savePixel(uint8_t* dest, const glm::vec3* src) {
		dest[0] = (uint8_t)(src->x * 255.0f);
		dest[1] = (uint8_t)(src->y * 255.0f);
		dest[2] = (uint8_t)(src->z * 255.0f);
		dest[3] = 255u;
	}

	inline void savePixel(uint8_t* dest, const glm::vec4* src) {
		dest[0] = (uint8_t)(src->x * 255.0f);
		dest[1] = (uint8_t)(src->y * 255.0f);
		dest[2] = (uint8_t)(src->z * 255.0f);
		dest[3] = (uint8_t)(src->w * 255.0f);
	}

	template <typename ReturnType>
	void saveFunction2DtoPNGinternal(const std::string& filename, const Function2D<ReturnType>& f) {
		std::vector<uint8_t> image;
		uint32_t width = f.mStorage.mGridSize.x, height = f.mStorage.mGridSize.y;
		auto& gridData = f.mStorage.mGrid;

		image.resize(width * height * 4);
		uint32_t pixelCount = width * height;
		for (uint32_t i = 0; i < pixelCount; ++i) {
			uint8_t* ptr = &image[i * 4];
			savePixel(ptr, &gridData[i]);
		}

		uint32_t error = lodepng::encode(filename, &image[0], width, height);

		if (error) {
			std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			throw std::exception(lodepng_error_text(error));
		}
	}

	template <typename ReturnType>
	void loadFunction2DfromPNGinternal(const std::string& filename, Function2D<ReturnType>* out) {
		std::vector<uint8_t> image;
		uint32_t width, height;
		uint32_t error = lodepng::decode(image, width, height, filename);

		if (error) {
			std::cout << "Decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			throw std::exception(lodepng_error_text(error));
		}

		out->mStorage.wrap() = WrapType::REPEAT;
		out->mStorage.init(width, height);
		auto& gridData = out->mStorage.mGrid;

		uint32_t pixelCount = width * height;
		for (uint32_t i = 0; i < pixelCount; ++i) {
			uint8_t* ptr = &image[i * 4];
			loadPixel(&gridData[i], ptr);
		}
	}

	void loadFunction2DfromPNG(const std::string& filename, Function2D<float>* out) {
		loadFunction2DfromPNGinternal(filename, out);
	}
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec2>* out) {
		loadFunction2DfromPNGinternal(filename, out);
	}
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec3>* out) {
		loadFunction2DfromPNGinternal(filename, out);
	}
	void loadFunction2DfromPNG(const std::string& filename, Function2D<glm::vec4>* out) {
		loadFunction2DfromPNGinternal(filename, out);
	}

	void saveFunction2DtoPNG(const std::string& filename, const Function2D<float>& f) {
		saveFunction2DtoPNGinternal(filename, f);
	}
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec2>& f) {
		saveFunction2DtoPNGinternal(filename, f);
	}
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec3>& f) {
		saveFunction2DtoPNGinternal(filename, f);
	}
	void saveFunction2DtoPNG(const std::string& filename, const Function2D<glm::vec4>& f) {
		saveFunction2DtoPNGinternal(filename, f);
	}
}