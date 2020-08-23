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
	void saveCubemapStorageToPNGinternal(const std::string& filename, const CubemapStorage<ReturnType>& f) {
		std::string filename_base;
		size_t pindx = filename.rfind('.');
		if (pindx == std::string::npos)
			filename_base = filename;
		else
			filename_base = filename.substr(0, pindx);

		std::map<uint32_t, std::string> append_str;
		append_str[f.FACE_POSITIVE_X] = "_pos_x";
		append_str[f.FACE_NEGATIVE_X] = "_neg_x";
		append_str[f.FACE_POSITIVE_Y] = "_pos_y";
		append_str[f.FACE_NEGATIVE_Y] = "_neg_y";
		append_str[f.FACE_POSITIVE_Z] = "_pos_z";
		append_str[f.FACE_NEGATIVE_Z] = "_neg_z";

		std::vector<uint8_t> image;
		uint32_t width = f.width();
		uint32_t height = f.height();
		image.resize((size_t)width * (size_t)height * 4);
		size_t pixelCount = (size_t)width * (size_t)height;

		for (uint32_t face = 0; face < 6; ++face) {
			for (size_t i = 0; i < pixelCount; ++i) {
				uint8_t* ptr = &image[i * 4];
				savePixel(ptr, &f.atIndex(i, face));
			}

			std::string save_path = filename_base + append_str[face] + ".png";

			uint32_t error = lodepng::encode(save_path, &image[0], width, height);

			if (error) {
				std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
				throw std::exception(lodepng_error_text(error));
			}
		}
	}

	template <typename ReturnType>
	void loadCubemapStorageFromPNGinternal(const std::string& filename, CubemapStorage<ReturnType>* out) {

		std::string filename_base;
		size_t pindx = filename.rfind('.');
		if (pindx == std::string::npos)
			filename_base = filename;
		else
			filename_base = filename.substr(0, pindx);

		std::map<uint32_t, std::string> append_str;
		append_str[out->FACE_POSITIVE_X] = "_pos_x";
		append_str[out->FACE_NEGATIVE_X] = "_neg_x";
		append_str[out->FACE_POSITIVE_Y] = "_pos_y";
		append_str[out->FACE_NEGATIVE_Y] = "_neg_y";
		append_str[out->FACE_POSITIVE_Z] = "_pos_z";
		append_str[out->FACE_NEGATIVE_Z] = "_neg_z";

	
		for (uint32_t face = 0; face < 6; ++face) {
			std::string save_path = filename_base + append_str[face] + ".png";

			std::vector<uint8_t> image;

			uint32_t width;
			uint32_t height;
			uint32_t error = lodepng::decode(image, width, height, save_path);

			if (error) {
				std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
				throw std::exception(lodepng_error_text(error));
			}

			if (width != height) {
				throw std::exception("Cubemap height must be equal to width!");
			}

			if (face == 0) {
				out->allocate(width, height);
			}
			else {
				if (width != out->width()) {
					throw std::exception("Cubemap face widths are inconsistent!");
				}
			}

			size_t pixelCount = (size_t)out->width() * (size_t)out->height();

			for (size_t i = 0; i < pixelCount; ++i) {
				uint8_t* ptr = &image[i * 4];
				loadPixel(&out->atIndex(i, face), ptr);
			}
		}

		out->transition(StorageMode::READ);
	}

	template <typename ReturnType>
	void saveRectSurfaceStorageToPNGinternal(const std::string& filename, const RectSurfaceGridStorage<ReturnType>& f) {
		std::vector<uint8_t> image;
		uint32_t width = f.width();
		uint32_t height = f.height();
		auto& gridData = f.mGrid;

		image.resize((size_t)width * (size_t)height * 4);
		size_t pixelCount = (size_t)width * (size_t)height;
		for (size_t i = 0; i < pixelCount; ++i) {
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
	void loadRectSurfaceStorageFromPNGinternal(const std::string& filename, RectSurfaceGridStorage<ReturnType>* out) {
		std::vector<uint8_t> image;
		uint32_t width, height;
		uint32_t error = lodepng::decode(image, width, height, filename);

		if (error) {
			std::cout << "Decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			throw std::exception(lodepng_error_text(error));
		}

		out->wrap() = WrapType::REPEAT;
		out->allocate(width, height);
		auto& gridData = out->mGrid;

		size_t pixelCount = (size_t)width * (size_t)height;
		for (size_t i = 0; i < pixelCount; ++i) {
			uint8_t* ptr = &image[i * 4];
			loadPixel(&gridData[i], ptr);
		}

		out->transition(StorageMode::READ);
	}

	void testSaveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f) {
		std::string filename_base;
		size_t pindx = filename.rfind('.');
		if (pindx == std::string::npos)
			filename_base = filename;
		else
			filename_base = filename.substr(0, pindx);

		std::map<uint32_t, std::string> append_str;
		append_str[f.FACE_POSITIVE_X] = "_pos_x";
		append_str[f.FACE_NEGATIVE_X] = "_neg_x";
		append_str[f.FACE_POSITIVE_Y] = "_pos_y";
		append_str[f.FACE_NEGATIVE_Y] = "_neg_y";
		append_str[f.FACE_POSITIVE_Z] = "_pos_z";
		append_str[f.FACE_NEGATIVE_Z] = "_neg_z";

		std::vector<uint8_t> image;
		uint32_t width = f.mGridSize.x;
		uint32_t height = f.mGridSize.y;
		image.resize((size_t)width * (size_t)height * 4);
		size_t pixelCount = (size_t)width * (size_t)height;

		for (uint32_t face = 0; face < 6; ++face) {
			for (size_t i = 0; i < pixelCount; ++i) {
				uint8_t* ptr = &image[i * 4];
				savePixel(ptr, &f.mCubemapData[face * (size_t)width * (size_t)height + i]);
			}

			std::string save_path = filename_base + append_str[face] + ".png";

			uint32_t error = lodepng::encode(save_path, &image[0], width, height);

			if (error) {
				std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
				throw std::exception(lodepng_error_text(error));
			}
		}
	}

	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<float>* out) {
		loadRectSurfaceStorageFromPNGinternal(filename, out);
	}
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec2>* out) {
		loadRectSurfaceStorageFromPNGinternal(filename, out);
	}
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec3>* out) {
		loadRectSurfaceStorageFromPNGinternal(filename, out);
	}
	void loadRectSurfaceStorageFromPNG(const std::string& filename, RectSurfaceGridStorage<glm::vec4>* out) {
		loadRectSurfaceStorageFromPNGinternal(filename, out);
	}

	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<float>& f) {
		saveRectSurfaceStorageToPNGinternal(filename, f);
	}
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec2>& f) {
		saveRectSurfaceStorageToPNGinternal(filename, f);
	}
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec3>& f) {
		saveRectSurfaceStorageToPNGinternal(filename, f);
	}
	void saveRectSurfaceStorageToPNG(const std::string& filename, const RectSurfaceGridStorage<glm::vec4>& f) {
		saveRectSurfaceStorageToPNGinternal(filename, f);
	}

	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<float>* out) {
		loadCubemapStorageFromPNGinternal(filename, out);
	}
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec2>* out) {
		loadCubemapStorageFromPNGinternal(filename, out);
	}
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec3>* out) {
		loadCubemapStorageFromPNGinternal(filename, out);
	}
	void loadCubemapStorageFromPNG(const std::string& filename, CubemapStorage<glm::vec4>* out) {
		loadCubemapStorageFromPNGinternal(filename, out);
	}

	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<float>& f) {
		saveCubemapStorageToPNGinternal(filename, f);
	}
	void saveCubemapeStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec2>& f) {
		saveCubemapStorageToPNGinternal(filename, f);
	}
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec3>& f) {
		saveCubemapStorageToPNGinternal(filename, f);
	}
	void saveCubemapStorageToPNG(const std::string& filename, const CubemapStorage<glm::vec4>& f) {
		saveCubemapStorageToPNGinternal(filename, f);
	}
}