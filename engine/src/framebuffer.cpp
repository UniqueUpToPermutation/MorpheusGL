#include <engine/framebuffer.hpp>

#include <GLFW/glfw3.h>

namespace Morpheus {

	Framebuffer* Framebuffer::toFramebuffer() {
		return this;
	}

	void Framebuffer::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mId);
	}

	void Framebuffer::blit(Framebuffer* target, GLbitfield copyComponents) {
		uint copy_width = std::min(target->width(), width());
		uint copy_height = std::min(target->height(), height());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, mId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->mId);
		glBlitFramebuffer(0, 0, copy_width, copy_height,
			0, 0, copy_width, copy_height, copyComponents, GL_NEAREST);
	}

	void Framebuffer::blitToBackBuffer(GLbitfield copyComponents) {

		int bbWidth, bbHeight;
		getFramebufferSize(&bbWidth, &bbHeight);

		uint copy_width = std::min<uint>(bbWidth, width());
		uint copy_height = std::min<uint>(bbHeight, height());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, mId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, copy_width, copy_height,
			0, 0, copy_width, copy_height, copyComponents, GL_NEAREST);
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferUnmanaged(uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthStencilFormat, 
			uint samples) {
		return makeFramebufferUnmanaged(width, height, 
			(GLenum*)&colorComponentFormats[0], 
			colorComponentFormats.size(),
			depthStencilFormat, samples);
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferUnmanaged(uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount,
			GLenum depthStencilFormat, 
			uint samples) {

		auto textureFactory = getFactory<Texture>();

		std::vector<Texture*> colorTextures;
		Texture* depthStencilTexture = nullptr;

		for (auto it = colorComponentFormats, end = &colorComponentFormats[colorComponentCount];
			it < end; ++it) {
			colorTextures.emplace_back(textureFactory->makeTexture2DUnmanaged(width, height, *it, 1));
		}

		if (depthStencilFormat != 0) {
			depthStencilTexture = textureFactory->makeTexture2DUnmanaged(width, height, depthStencilFormat, 1);
		}

		GLuint framebufferId;
		glGenFramebuffers(1, &framebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
		uint i = 0;
		for (auto tex : colorTextures) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->id(), 0);
			++i;
		}

		if (depthStencilTexture) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilTexture->id(), 0);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferUnmanaged(uint width, uint height, 
		GLenum colorComponentFormat,
		GLenum depthStencilFormat, 
		uint samples) {
		return makeFramebufferUnmanaged(width, height, &colorComponentFormat, 1, depthStencilFormat, samples);
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthFormat,
			GLenum stencilFormat, 
			uint samples) {
		return makeFramebufferSeparateDSUnmanaged(width, height, (GLenum*)&colorComponentFormats[0],
			colorComponentFormats.size(), depthFormat, stencilFormat, samples);
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount,
			GLenum depthFormat,
			GLenum stencilFormat,
			uint samples) {
		auto textureFactory = getFactory<Texture>();

		std::vector<Texture*> colorTextures;
		Texture* depthTexture = nullptr;
		Texture* stencilTexture = nullptr;

		for (auto it = colorComponentFormats, end = &colorComponentFormats[colorComponentCount];
			it < end; ++it) {
			colorTextures.emplace_back(textureFactory->makeTexture2DUnmanaged(width, height, *it, 1));
		}

		if (depthFormat != 0) {
			depthTexture = textureFactory->makeTexture2DUnmanaged(width, height, depthFormat, 1);
		}

		if (stencilFormat != 0) {
			stencilTexture = textureFactory->makeTexture2DUnmanaged(width, height, stencilFormat, 1);
		}

		GLuint framebufferId;
		glGenFramebuffers(1, &framebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
		uint i = 0;
		for (auto tex : colorTextures) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->id(), 0);
			++i;
		}

		if (depthTexture) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->id(), 0);
		}

		if (stencilTexture) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, stencilTexture->id(), 0);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			GLenum colorComponentFormat,
			GLenum depthFormat,
			GLenum stencilFormat,
			uint samples) {
		return makeFramebufferSeparateDSUnmanaged(width, height, &colorComponentFormat, 1, 
			depthFormat, stencilFormat, samples);
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebuffer(INodeOwner* parent, uint width, uint height, 
		const std::vector<GLenum>& colorComponentFormats,
		GLenum depthStencilFormat, 
		uint samples) {
		auto framebuffer = makeFramebufferUnmanaged(width, height, colorComponentFormats, 
			depthStencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebuffer(INodeOwner* parent, uint width, uint height, 
		GLenum colorComponentFormats[],
		uint colorComponentCount,
		GLenum depthStencilFormat, 
		uint samples) {
		auto framebuffer = makeFramebufferUnmanaged(width, height, colorComponentFormats, 
			colorComponentCount, depthStencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebuffer(INodeOwner* parent, uint width, uint height, 
		GLenum colorComponentFormat,
		GLenum depthStencilFormat, 
		uint samples) {
		auto framebuffer = makeFramebufferUnmanaged(width, height, colorComponentFormat,
			depthStencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
		const std::vector<GLenum>& colorComponentFormats,
		GLenum depthFormat,
		GLenum stencilFormat, 
		uint samples) {
		auto framebuffer = makeFramebufferSeparateDSUnmanaged(width, height, colorComponentFormats, 
			depthFormat, stencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
		GLenum colorComponentFormats[],
		uint colorComponentCount,
		GLenum depthFormat,
		GLenum stencilFormat,
		uint samples) {
		auto framebuffer = makeFramebufferSeparateDSUnmanaged(width, height, colorComponentFormats, colorComponentCount,
			depthFormat, stencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	Framebuffer* ContentFactory<Framebuffer>::makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
		GLenum colorComponentFormat,
		GLenum depthFormat,
		GLenum stencilFormat,
		uint samples) {
		auto framebuffer = makeFramebufferSeparateDSUnmanaged(width, height, colorComponentFormat, 
			depthFormat, stencilFormat, samples);
		createContentNode(framebuffer, parent);
		return framebuffer;
	}

	INodeOwner* ContentFactory<Framebuffer>::load(const std::string& source, Node loadInto) {
		throw std::runtime_error("Load not supported for framebuffer! Use loadEx");
	}

	INodeOwner* ContentFactory<Framebuffer>::loadEx(const std::string& source, Node loadInto, const void* extParams) {
		const ContentExtParams<Framebuffer>* ext = reinterpret_cast<const ContentExtParams<Framebuffer>*>(extParams);
		if (ext->bSeparateDepthStencil) {
			return makeFramebufferSeparateDSUnmanaged(ext->mWidth, ext->mHeight, 
				ext->mColorComponentFormats, ext->mDepthFormat, ext->mStencilFormat,
				ext->mSamples);
		} else {
			return makeFramebufferSeparateDSUnmanaged(ext->mWidth, ext->mHeight,
				ext->mColorComponentFormats, ext->mDepthStencilFormat, ext->mSamples);
		}
	}
}