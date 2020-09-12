#include <engine/texture.hpp>
#include <lodepng/lodepng.h>

#include <iostream>
#include <gli/gli.hpp>
#include <glad/glad.h>

namespace Morpheus {
	ref<void> ContentFactory<Texture>::load(const std::string& source, Node& loadInto) {
		std::cout << "Loading texture " << source;
		size_t loc = source.rfind('.');
		if (loc != std::string::npos) {
			auto ext = source.substr(loc);
			if (ext == ".png") {
				std::cout << " (using lodepng)..." << std::endl;
				return loadpng(source);
			}
			else if (ext == ".ktx" || ext == ".dds" || ext == ".kmg") {
				std::cout << " (using gli)..." << std::endl;
				return loadgli(source);
			}
			else {
				std::cout << "\nFormat not recognized!" << std::endl;
				return ref<void>(nullptr);
			}
		}
		else {
			std::cout << std::endl << source << " missing file extension!" << std::endl;
			return ref<void>(nullptr);
		}
	}

	ref<Texture> ContentFactory<Texture>::loadgli(const std::string& source) {
		TextureType type;
		GLenum gltype;
		gli::texture tex = gli::load(source);
		if (tex.empty()) {
			std::cout << "Failed to load texture " << source << "!" << std::endl;
			return 0;
		}

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(tex.format(), tex.swizzles());
		GLenum Target = GL.translate(tex.target());

		GLuint TextureName = 0;
		glGenTextures(1, &TextureName);
		glBindTexture(Target, TextureName);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(tex.levels() - 1));
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

		glm::tvec3<GLsizei> const Extent(tex.extent());
		GLsizei const FaceTotal = static_cast<GLsizei>(tex.layers() * tex.faces());

		switch (tex.target()) {
		case gli::TARGET_1D:
			glTexStorage1D(
				Target, static_cast<GLint>(tex.levels()), Format.Internal,
				Extent.x);
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			glTexStorage2D(
				Target, static_cast<GLint>(tex.levels()), Format.Internal,
				Extent.x, Extent.y);
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			glTexStorage3D(
				Target, static_cast<GLint>(tex.levels()), Format.Internal,
				Extent.x, Extent.y, Extent.z);
			break;
		default:
			assert(0);
			break;
		}

		switch (tex.target()) {
		case gli::TARGET_1D:
			type = TextureType::TEXTURE_1D;
			gltype = GL_TEXTURE_1D;
			break;
		case gli::TARGET_1D_ARRAY:
			type = TextureType::TEXTURE_1D_ARRAY;
			gltype = GL_TEXTURE_1D_ARRAY;
			break;
		case gli::TARGET_2D:
			type = TextureType::TEXTURE_2D;
			gltype = GL_TEXTURE_2D;
			break;
		case gli::TARGET_CUBE:
			type = TextureType::CUBE_MAP;
			gltype = GL_TEXTURE_CUBE_MAP;
			break;
		case gli::TARGET_2D_ARRAY:
			type = TextureType::TEXTURE_2D_ARRAY;
			gltype = GL_TEXTURE_2D_ARRAY;
			break;
		case gli::TARGET_3D:
			type = TextureType::TEXTURE_3D;
			gltype = GL_TEXTURE_3D;
			break;
		case gli::TARGET_CUBE_ARRAY:
			type = TextureType::CUBE_MAP_ARRAY;
			gltype = GL_TEXTURE_CUBE_MAP_ARRAY;
			break;
		}

		for (std::size_t Layer = 0; Layer < tex.layers(); ++Layer) {
			for (std::size_t Face = 0; Face < tex.faces(); ++Face) {
				for (std::size_t Level = 0; Level < tex.levels(); ++Level)
				{
					GLsizei const LayerGL = static_cast<GLsizei>(Layer);
					glm::tvec3<GLsizei> Extent(tex.extent(Level));
					Target = gli::is_target_cube(tex.target())
						? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
						: Target;

					switch (tex.target())
					{
					case gli::TARGET_1D:
						if (gli::is_compressed(tex.format()))
							glCompressedTexSubImage1D(
								Target, static_cast<GLint>(Level), 0, Extent.x,
								Format.Internal, static_cast<GLsizei>(tex.size(Level)),
								tex.data(Layer, Face, Level));
						else
							glTexSubImage1D(
								Target, static_cast<GLint>(Level), 0, Extent.x,
								Format.External, Format.Type,
								tex.data(Layer, Face, Level));
						break;
					case gli::TARGET_1D_ARRAY:
					case gli::TARGET_2D:
					case gli::TARGET_CUBE:
						if (gli::is_compressed(tex.format()))
							glCompressedTexSubImage2D(
								Target, static_cast<GLint>(Level),
								0, 0,
								Extent.x,
								tex.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
								Format.Internal, static_cast<GLsizei>(tex.size(Level)),
								tex.data(Layer, Face, Level));
						else
							glTexSubImage2D(
								Target, static_cast<GLint>(Level),
								0, 0,
								Extent.x,
								tex.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
								Format.External, Format.Type,
								tex.data(Layer, Face, Level));
						break;
					case gli::TARGET_2D_ARRAY:
					case gli::TARGET_3D:
					case gli::TARGET_CUBE_ARRAY:
						if (gli::is_compressed(tex.format()))
							glCompressedTexSubImage3D(
								Target, static_cast<GLint>(Level),
								0, 0, 0,
								Extent.x, Extent.y,
								tex.target() == gli::TARGET_3D ? Extent.z : LayerGL,
								Format.Internal, static_cast<GLsizei>(tex.size(Level)),
								tex.data(Layer, Face, Level));
						else
							glTexSubImage3D(
								Target, static_cast<GLint>(Level),
								0, 0, 0,
								Extent.x, Extent.y,
								tex.target() == gli::TARGET_3D ? Extent.z : LayerGL,
								Format.External, Format.Type,
								tex.data(Layer, Face, Level));
						break;
					default: assert(0); break;
					}
				}
			}
		}

		Morpheus::Texture* tex_ptr = new Morpheus::Texture();
		tex_ptr->mId = TextureName;
		tex_ptr->mType = type;
		tex_ptr->mGLTarget = gltype;
		tex_ptr->mWidth = Extent.x;
		tex_ptr->mHeight = Extent.y;
		tex_ptr->mDepth = Extent.z;
		tex_ptr->mFormat = Format.Internal;
		return ref<Texture>(tex_ptr);
	}

	ref<Texture> ContentFactory<Texture>::loadpng(const std::string& source) {
		std::vector<uint8_t> image;
		uint32_t width, height;
		uint32_t error = lodepng::decode(image, width, height, source);

		//if there's an error, display it
		if (error) std::cout << "Decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

		//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
		//State state contains extra information about the PNG such as text chunks, ...
		if (!error) {
			GLuint TextureName = 0;
			glGenTextures(1, &TextureName);
			glBindTexture(GL_TEXTURE_2D, TextureName);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
			glGenerateTextureMipmap(TextureName);

			Texture* tex = new Texture();
			tex->mId = TextureName;
			tex->mType = TextureType::TEXTURE_2D;
			tex->mGLTarget = GL_TEXTURE_2D;
			tex->mWidth = width;
			tex->mHeight = height;
			tex->mDepth = 1;
			tex->mFormat = GL_RGBA8;

			return ref<Texture>(tex);
		}
		else {
			return ref<Texture>(nullptr);
		}
	}

	void Texture::savepngcubemap(const std::string& path) const {
		size_t pos = path.rfind('.');
		std::string base_path;
		std::map<GLint, std::string> face_paths;
		std::map<GLint, uint32_t> offsets;

		if (pos == std::string::npos) {
			base_path = path;
		}
		else {
			base_path = path.substr(0, pos);
		}

		face_paths[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = base_path + "_pos_x.png";
		offsets[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = 0;
		face_paths[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = base_path + "_neg_x.png";
		offsets[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = 1;
		face_paths[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = base_path + "_pos_y.png";
		offsets[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = 2;
		face_paths[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = base_path + "_neg_y.png";
		offsets[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = 3;
		face_paths[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = base_path + "_pos_z.png";
		offsets[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = 4;
		face_paths[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = base_path + "_neg_z.png";
		offsets[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = 5;

		GLsizei memSize = 4 * mWidth * mHeight * 6;
		std::vector<uint8_t> data(memSize);
		glGetTextureImage(mId, 0, GL_RGBA, GL_UNSIGNED_BYTE, memSize, &data[0]);

		auto do_save = [this, &face_paths, &offsets, &data](GLint side) {
			auto& path = face_paths[side];
			auto offset = offsets[side];
			std::cout << "Saving cubemap face " << path << "..." << std::endl;
			GLsizei memOffset = 4 * mWidth * mHeight * offset;
			auto error = lodepng::encode(path, &data[memOffset], mWidth, mHeight);

			if (error) {
				std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
				throw std::runtime_error(lodepng_error_text(error));
			}
		};

		do_save(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		do_save(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		do_save(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		do_save(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		do_save(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		do_save(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	}

	void Texture::savepngtex2d(const std::string& path) const {
		GLsizei memSize = 4 * mWidth * mHeight;
		std::vector<uint8_t> data(memSize);
		std::vector<uint8_t> png;
		glGetTextureImage(mId, 0, GL_RGBA, GL_UNSIGNED_BYTE, memSize, &data[0]);

		std::cout << "Saving texture2D " << path << "..." << std::endl;
		auto error = lodepng::encode(path, &data[0], mWidth, mHeight);

		if (error) {
			std::cout << "Encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			throw std::runtime_error(lodepng_error_text(error));
		}
	}

	void Texture::savepng(const std::string& path) const {
		switch (mType) {
		case TextureType::TEXTURE_2D:
			savepngtex2d(path);
			break;
		case TextureType::CUBE_MAP:
			savepngcubemap(path);
			break;
		default:
			std::cout << "Texture type must be TEXTURE_2D or CUBE_MAP! Cannot save to png!" << std::endl;
			throw std::runtime_error("Texture type must be TEXTURE_2D or CUBE_MAP! Cannot save to png!");
			break;
		}	
	}

	void Texture::genMipmaps() {
		glGenerateTextureMipmap(mId);
		GL_ASSERT;
	}

	void ContentFactory<Texture>::unload(ref<void>& ref) {
		auto tex = ref.reinterpretGet<Texture>();
		glDeleteTextures(1, &tex->mId);
		delete tex;
	}

	void ContentFactory<Texture>::dispose() {
		delete this;
	}

	ref<Texture> ContentFactory<Texture>::makeTexture2DUnmanaged(const uint32_t width, const uint32_t height, 
		const GLenum format, const int miplevels) {
		GLuint TextureName = 0;
		glGenTextures(1, &TextureName);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		GL_ASSERT;

		GLsizei actual_mip_levels = 0;
		if (miplevels >= 0) {
			actual_mip_levels = miplevels;
		}
		else {
			uint32_t pow = 0;
			uint32_t size = std::min(width, height);
			while (size) { size /= 2; ++pow; }
			actual_mip_levels = pow;
		}

		glTexStorage2D(GL_TEXTURE_2D, actual_mip_levels, format, width, height);
		GL_ASSERT;

		Texture* tex = new Texture();
		tex->mWidth = width;
		tex->mHeight = height;
		tex->mType = TextureType::TEXTURE_2D;
		tex->mGLTarget = GL_TEXTURE_2D;
		tex->mId = TextureName;
		tex->mDepth = 1;
		tex->mFormat = format;
		return ref<Texture>(tex);
	}

	Node ContentFactory<Texture>::makeTexture2DUnparented(ref<Texture>* out, const uint32_t width, const uint32_t height,
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeTexture2DUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef);
	}

	Node ContentFactory<Texture>::makeTexture2D(ref<Texture>* out, Node parent, 
		const uint32_t width, const uint32_t height, 
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeTexture2DUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef, parent);
	}

	Node ContentFactory<Texture>::makeTexture2D(ref<Texture>* out, NodeHandle parent, 
		const uint32_t width, const uint32_t height, 
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeTexture2DUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef, parent);
	}

	ref<Texture> ContentFactory<Texture>::makeCubemapUnmanaged(const uint32_t width, const uint32_t height,
		const GLenum format, const int miplevels) {
		GLuint TextureName = 0;
		glGenTextures(1, &TextureName);
		glBindTexture(GL_TEXTURE_CUBE_MAP, TextureName);
		GL_ASSERT;

		GLsizei actual_mip_levels = 0;
		if (miplevels >= 0) {
			actual_mip_levels = miplevels;
		}
		else {
			uint32_t pow = 0;
			uint32_t size = std::min(width, height);
			while (size) { size /= 2; ++pow; }
			actual_mip_levels = pow;
		}

		glTexStorage2D(GL_TEXTURE_CUBE_MAP, actual_mip_levels, format, width, height);
		GL_ASSERT;

		Texture* tex = new Texture();
		tex->mWidth = width;
		tex->mHeight = height;
		tex->mType = TextureType::CUBE_MAP;
		tex->mGLTarget = GL_TEXTURE_CUBE_MAP;
		tex->mId = TextureName;
		tex->mDepth = 1;
		tex->mFormat = format;
		return ref<Texture>(tex);
	}

	Node ContentFactory<Texture>::makeCubemapUnparented(ref<Texture>* out, const uint32_t width, const uint32_t height,
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeCubemapUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef);
	}

	Node ContentFactory<Texture>::makeCubemap(ref<Texture>* out, Node parent,
		const uint32_t width, const uint32_t height, 
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeCubemapUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef, parent);
	}

	Node ContentFactory<Texture>::makeCubemap(ref<Texture>* out, NodeHandle parent, 
		const uint32_t width, const uint32_t height, 
		const GLenum format, const int miplevels) {
		ref<Texture> texRef = makeCubemapUnmanaged(width, height, format, miplevels);
		if (out)
			*out = texRef;
		return content()->createContentNode(texRef, parent);
	}

	std::string ContentFactory<Texture>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(Texture);
	}
}