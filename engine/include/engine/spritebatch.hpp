#pragma once

#include <engine/engine.hpp>
#include <engine/blend.hpp>
#include <engine/shader.hpp>
#include <engine/texture.hpp>
#include <engine/sampler.hpp>

#include <vector>

#include <glad/glad.h>

#define SPRITE_INSTANCE_BUFFER_SIZE 16 * sizeof(float)
#define DEFAULT_BATCH_SIZE 2000

namespace Morpheus {

	struct UVRect {
		glm::ivec2 mPosition;
		glm::ivec2 mSize;
	};

	enum class SpriteBatchMode {
		IMMEDIATE,
		DEFERRED_IN_ORDER,
		DEFERRED_SORT_FRONT_TO_BACK,
		DEFERRED_SORT_BACK_TO_FRONT,
		DEFERRED_SORT_TEXTURE
	};

	enum class SpriteBatchSampler {
		NEAREST,
		LINEAR
	};

	struct SpriteInstance {
		Texture* mTexture;
		glm::vec3 mPosition;
		float mRotation;
		glm::vec2 mOrigin;
		glm::vec2 mExtents;
		glm::vec2 mUVUpper;
		glm::vec2 mUVLower;
		ByteColor mColor;
		int mTexId; 
	};

	class SpriteBatch : public INodeOwner {
	private:
		GLuint mBatchBuffer;
		GLuint mVertexArray;

		Sampler* mLinearSampler;
		Sampler* mNearestSampler;
		Sampler* mSamplerToUse;

		Shader* mDefaultShader;
		ShaderUniform<Sampler> mDefaultShaderTex;
		ShaderUniform<glm::mat4> mDefaultShaderTransform;

		Shader* mShaderToUse;
		ShaderUniform<Sampler> mShaderTexToUse;
		ShaderUniform<glm::mat4> mShaderTransformToUse;

		Texture* mLastImmediateTexture;

		uint mImmediateCacheCount;
		uint mCurrentCacheIndex;
		uint mLastFlushIndex;

		std::vector<SpriteInstance> mDeferredSprites;
		std::vector<uint8_t> mImmediateBuffer;

		SpriteBatchMode mMode;

		template <typename positionType> 
		void drawInternal(Texture* texture, const positionType& position, const glm::vec2& size, 
			const UVRect& rect, float rotation, const glm::vec2& origin, const glm::vec4& color);

		void getNewBuffer(uint spriteCount);
		void submitInstance(const SpriteInstance& inst);

	public:
		SpriteBatch(uint batchSize = DEFAULT_BATCH_SIZE);
		~SpriteBatch();

		void init() override;

		void begin(SpriteBatchMode mode = SpriteBatchMode::IMMEDIATE,
			BlendMode blendMode = BlendMode::ALPHA,
			SpriteBatchSampler samplerMode = SpriteBatchSampler::LINEAR,
			const glm::mat4& transform = glm::identity<glm::mat4>(),
			Shader* shaderOverride = nullptr,
			ShaderUniform<Sampler> shaderTextureUniform = ShaderUniform<Sampler>(),
			ShaderUniform<glm::mat4> shaderTransformUniform = ShaderUniform<glm::mat4>());

		void draw(Texture* texture, const glm::vec3& position, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec3& position, 
			const UVRect& rect, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec3& position, 
			float rotation, const glm::vec2& origin, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec3& position, 
			const UVRect& rect, float rotation, const glm::vec2& origin, 
			const glm::vec4& color);
		void draw(Texture* texture, const glm::vec3& position, 
			const glm::vec2& size, const UVRect& rect, float rotation, 
			const glm::vec2& origin, 
			const glm::vec4& color = glm::one<glm::vec4>());

		void draw(Texture* texture, const glm::vec2& position, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec2& position, 
			const UVRect& rect, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec2& position, 
			float rotation, const glm::vec2& origin, 
			const glm::vec4& color = glm::one<glm::vec4>());
		void draw(Texture* texture, const glm::vec2& position, 
			const UVRect& rect, float rotation, const glm::vec2& origin, 
			const glm::vec4& color);
		void draw(Texture* texture, const glm::vec2& position, 
			const glm::vec2& size, const UVRect& rect, float rotation, 
			const glm::vec2& origin, 
			const glm::vec4& color = glm::one<glm::vec4>());

		void flush();
		void end();
	};
	SET_NODE_ENUM(SpriteBatch, SPRITE_BATCH);
}