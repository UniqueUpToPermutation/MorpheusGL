#pragma once

#include <engine/core.hpp>
#include <engine/texture.hpp>

#include <glad/glad.h>

namespace Morpheus {

	template <>
	struct ContentExtParams<Framebuffer> {
		bool bSeparateDepthStencil;
		uint mWidth;
		uint mHeight;
		uint mSamples;
		std::vector<GLenum> mColorComponentFormats;
		GLenum mDepthFormat;
		GLenum mStencilFormat;
		GLenum mDepthStencilFormat;
	};

	class Framebuffer : public INodeOwner {
	private:
		GLuint mId;
		std::vector<Texture*> mColorAttachments;
		Texture* mDepthAttachment;
		Texture* mStencilAttachment;
		Texture* mDepthStencilAttachment;
		uint mSamples;
		uint mWidth;
		uint mHeight;

	public:
		inline Framebuffer() : INodeOwner(NodeType::FRAMEBUFFER) {
		}

		inline GLuint id() const {
			return mId;
		}

		inline Texture* getDepth() {
			return mDepthAttachment;
		}

		inline Texture* getStencil() {
			return mStencilAttachment;
		}

		inline Texture* getDepthStencil() {
			return mDepthStencilAttachment;
		}

		inline Texture* getColor(uint index = 0) {
			return mColorAttachments[index];
		}

		inline uint colorAttachmentCount() const {
			return mColorAttachments.size();
		}

		inline uint samples() const {
			return mSamples;
		}

		inline uint width() const {
			return mWidth;
		}

		inline uint height() const {
			return mHeight;
		}

		void resize(uint width, uint height, int samples = -1);

		void bind();
		void blit(Framebuffer* target, GLbitfield copyComponents);
		void blitToBackBuffer(GLbitfield copyComponents);

		Framebuffer* toFramebuffer() override;

		friend class ContentFactory<Framebuffer>;
	};
	SET_NODE_ENUM(Framebuffer, FRAMEBUFFER);

	template <>
	class ContentFactory<Framebuffer> : public IContentFactory {
	public:
		INodeOwner* load(const std::string& source, Node loadInto) override;
		INodeOwner* loadEx(const std::string& source, Node loadInto, const void* extParams) override;

		Framebuffer* makeFramebufferUnmanaged(uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthStencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebufferUnmanaged(uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount,
			GLenum depthStencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebufferUnmanaged(uint width, uint height, 
			GLenum colorComponentFormat,
			GLenum depthStencilFormat=0, 
			uint samples=1);

		Framebuffer* makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthFormat=0,
			GLenum stencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount,
			GLenum depthFormat=0,
			GLenum stencilFormat=0,
			uint samples=1);
		Framebuffer* makeFramebufferSeparateDSUnmanaged(uint width, uint height, 
			GLenum colorComponentFormat,
			GLenum depthFormat=0,
			GLenum stencilFormat=0,
			uint samples=1);

		Framebuffer* makeFramebuffer(INodeOwner* parent, uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthStencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebuffer(INodeOwner* parent, uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount=0,
			GLenum depthStencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebuffer(INodeOwner* parent, uint width, uint height, 
			GLenum colorComponentFormat=0,
			GLenum depthStencilFormat=0, 
			uint samples=1);

		Framebuffer* makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
			const std::vector<GLenum>& colorComponentFormats,
			GLenum depthFormat=0,
			GLenum stencilFormat=0, 
			uint samples=1);
		Framebuffer* makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
			GLenum colorComponentFormats[],
			uint colorComponentCount,
			GLenum depthFormat=0,
			GLenum stencilFormat=0,
			uint samples=1);
		Framebuffer* makeFramebufferSeparateDS(INodeOwner* parent, uint width, uint height, 
			GLenum colorComponentFormat,
			GLenum depthFormat=0,
			GLenum stencilFormat=0,
			uint samples=1);

		void unload(INodeOwner* ref) override;
		
		~ContentFactory();

		std::string getContentTypeString() const override;
	};
}