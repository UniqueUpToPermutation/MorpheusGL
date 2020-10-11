#include <engine/spritebatch.hpp>
#include <engine/texture.hpp>

#include <GLFW/glfw3.h>

namespace Morpheus {
	template <typename positionType>
	inline void copyPosition(glm::vec3* dest, const positionType& src);

	template <>
	inline void copyPosition<glm::vec2>(glm::vec3* dest, const glm::vec2& src) {
		dest->x = src.x;
		dest->y = src.y;
		dest->z = 0.0;
	}

	template <>
	inline void copyPosition<glm::vec3>(glm::vec3* dest, const glm::vec3& src) {
		*dest = src;
	}

	SpriteBatch::SpriteBatch(uint batchSize) : INodeOwner(NodeType::SPRITE_BATCH), 
		mNearestSampler(nullptr), mLinearSampler(nullptr), 
		mDefaultShader(nullptr), mShaderToUse(nullptr),
		mLastImmediateTexture(nullptr), mImmediateCacheCount(batchSize),
		mCurrentCacheIndex(0), mLastFlushIndex(0) {
		glGenVertexArrays(1, &mVertexArray);
		glGenBuffers(1, &mBatchBuffer);

		glBindBuffer(GL_ARRAY_BUFFER, mBatchBuffer);
		getNewBuffer(mImmediateCacheCount);

		// Create the correct vertex format
		constexpr uint sz = SPRITE_INSTANCE_BUFFER_SIZE;
		glBindVertexArray(mVertexArray);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, false, sz, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, false, sz, (const void*)(4 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, false, sz, (const void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, true, sz, (const void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 1, GL_INT, false, sz, (const void*)(12 * sizeof(float) + 4 * sizeof(GLubyte)));
	}

	SpriteBatch::~SpriteBatch() {
		glDeleteBuffers(1, &mBatchBuffer);
		glDeleteVertexArrays(1, &mVertexArray);
	}

	void SpriteBatch::begin(SpriteBatchMode mode, BlendMode blendMode, SpriteBatchSampler samplerMode, 
		const glm::mat4& transform, Shader* shaderOverride, ShaderUniform<Sampler> shaderTextureUniform, 
		ShaderUniform<glm::mat4> shaderTransformUniform) {
		setBlendMode(blendMode);

		glDisable(GL_DEPTH_TEST);

		mMode = mode;

		if (!mDefaultShader)
			throw std::runtime_error("SpriteBatch has not been initialized!");

		// Resize the memory buffer to the proper size
		if (mImmediateBuffer.size() != mImmediateCacheCount * SPRITE_INSTANCE_BUFFER_SIZE) {
			mImmediateBuffer.resize(mImmediateCacheCount * SPRITE_INSTANCE_BUFFER_SIZE);
		}

		mDeferredSprites.clear();

		if (shaderOverride) {
			mShaderToUse = shaderOverride;
			mShaderTexToUse = shaderTextureUniform;
			mShaderTransformToUse = shaderTransformUniform;
		} else {
			mShaderToUse = mDefaultShader;
			mShaderTexToUse = mDefaultShaderTex;
			mShaderTransformToUse = mDefaultShaderTransform;
		}

		if (samplerMode == SpriteBatchSampler::NEAREST) {
			mSamplerToUse = mNearestSampler;
		} else {
			mSamplerToUse = mLinearSampler;
		}

		int width, height;
		glfwGetFramebufferSize(window(), &width, &height);

		glm::mat4 projTransform = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
		glm::mat4 fullTransform = projTransform * transform;

		// Bind shader, vertex array, and point geometry buffer
		mShaderToUse->bind();
		mShaderTransformToUse.set(fullTransform);
		mSamplerToUse->bind(mShaderTexToUse.unit());
		glBindBuffer(GL_ARRAY_BUFFER, mBatchBuffer);
		glBindVertexArray(mVertexArray);
	}

	void writeToData(uint8_t* data, const SpriteInstance& inst) {
		float* f_arr = reinterpret_cast<float*>(data);
		
		// Position
		int i = 0;
		f_arr[i++] = inst.mPosition.x;
		f_arr[i++] = inst.mPosition.y;
		f_arr[i++] = inst.mPosition.z;
		
		// Rotation
		f_arr[i++] = inst.mRotation;

		// Origin
		f_arr[i++] = inst.mOrigin.x;
		f_arr[i++] = inst.mOrigin.y;

		// Extents
		f_arr[i++] = inst.mExtents.x;
		f_arr[i++] = inst.mExtents.y;

		// uv_lower
		f_arr[i++] = inst.mUVLower.x;
		f_arr[i++] = inst.mUVLower.y;

		// uv_upper 
		f_arr[i++] = inst.mUVUpper.x;
		f_arr[i++] = inst.mUVUpper.y;

		// Color
		GLubyte* c_arr = reinterpret_cast<GLubyte*>(&f_arr[i]);
		c_arr[0] = inst.mColor.r;
		c_arr[1] = inst.mColor.g;
		c_arr[2] = inst.mColor.b;
		c_arr[3] = inst.mColor.a;

		// tex-id
		GLint* i_arr = reinterpret_cast<GLint*>(&c_arr[4]);
		i_arr[0] = inst.mTexId;
	}
	
	void SpriteBatch::flush() {
		// Draw all sprites since last flush
		uint count = (mCurrentCacheIndex - mLastFlushIndex);
		uint flush_offset = mLastFlushIndex * SPRITE_INSTANCE_BUFFER_SIZE;

		if (count > 0) {
			// Copy over data and submit draw call
			mLastImmediateTexture->bind(mShaderTexToUse.unit());
			
			GL_ASSERT;
			glBufferSubData(GL_ARRAY_BUFFER, flush_offset, 
				count * SPRITE_INSTANCE_BUFFER_SIZE, &mImmediateBuffer[flush_offset]);
			GL_ASSERT;
			glDrawArrays(GL_POINTS, mLastFlushIndex, count);
			GL_ASSERT;
			mLastFlushIndex = mCurrentCacheIndex;
		}
	}

	void SpriteBatch::getNewBuffer(uint spriteCount) {
		// Orphan old buffer data and create new buffer data
		glBufferData(GL_ARRAY_BUFFER, spriteCount * SPRITE_INSTANCE_BUFFER_SIZE, nullptr, GL_STREAM_DRAW);
		GL_ASSERT;
		mCurrentCacheIndex = 0;
		mLastFlushIndex = 0;
	}

	void SpriteBatch::submitInstance(const SpriteInstance& inst) {
		// Buffer is full?
		if (mCurrentCacheIndex == mImmediateCacheCount) {
			flush();
			getNewBuffer(mImmediateCacheCount);
		}

		// Texture has changed?
		if (inst.mTexture != mLastImmediateTexture) {
			flush();
			mLastImmediateTexture = inst.mTexture;
		}

		writeToData(&mImmediateBuffer[mCurrentCacheIndex * SPRITE_INSTANCE_BUFFER_SIZE], inst);
		++mCurrentCacheIndex;
	}

	template <typename positionType> 
	inline void SpriteBatch::drawInternal(Texture* texture, const positionType& position, const glm::vec2& size, 
		const UVRect& rect, float rotation, const glm::vec2& origin, const glm::vec4& color) {
		
		SpriteInstance inst;

		float width = texture->width();
		float height = texture->height();

		inst.mTexture = texture;
		copyPosition<positionType>(&inst.mPosition, position);
		inst.mRotation = rotation;
		inst.mExtents = size;
		inst.mOrigin = origin;
		inst.mTexId = 0;
		inst.mUVLower.x = rect.mPosition.x / width;
		inst.mUVLower.y = rect.mPosition.y / height;
		inst.mUVUpper.x = inst.mUVLower.x + rect.mSize.x / width;
		inst.mUVUpper.y = inst.mUVLower.y + rect.mSize.y / height;
		inst.mColor.r = static_cast<uint8_t>(color.r * 255);
		inst.mColor.g = static_cast<uint8_t>(color.g * 255);
		inst.mColor.b = static_cast<uint8_t>(color.b * 255);
		inst.mColor.a = static_cast<uint8_t>(color.a * 255);

		if (mMode != SpriteBatchMode::IMMEDIATE) 
			mDeferredSprites.emplace_back(inst);
		else
			submitInstance(inst);
	}

	void SpriteBatch::draw(Texture* texture, const glm::vec3& position, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			UVRect{glm::ivec2(0, 0), glm::ivec2(texture->width(), texture->height())},
			0.0, glm::vec2(0.0, 0.0), color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec3& position, 
		const UVRect& rect, 
		const glm::vec4& color ) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			rect, 0.0, glm::vec2(0.0, 0.0), color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec3& position, 
		float rotation, const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			UVRect{glm::ivec2(0, 0), glm::ivec2(texture->width(), texture->height())},
			rotation, origin, color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec3& position, 
		const UVRect& rect, float rotation, const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			rect, rotation, origin, color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec3& position, 
		const glm::vec2& size, const UVRect& rect, float rotation, 
		const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			size, rect, rotation, origin, color);
	}

	void SpriteBatch::draw(Texture* texture, const glm::vec2& position, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			UVRect{glm::ivec2(0, 0), glm::ivec2(texture->width(), texture->height())},
			0.0, glm::vec2(0.0, 0.0), color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec2& position, 
		const UVRect& rect, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			rect, 0.0, glm::vec2(0.0, 0.0), color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec2& position, 
		float rotation, const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			UVRect{glm::ivec2(0, 0), glm::ivec2(texture->width(), texture->height())},
			rotation, origin, color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec2& position, 
		const UVRect& rect, float rotation, const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			glm::vec2((float)texture->width(), (float)texture->height()),
			rect, rotation, origin, color);
	}
	void SpriteBatch::draw(Texture* texture, const glm::vec2& position, 
		const glm::vec2& size, const UVRect& rect, float rotation, 
		const glm::vec2& origin, 
		const glm::vec4& color) {
		drawInternal(texture, position, 
			size, rect, rotation, origin, color);
	}

	bool textureSort(const SpriteInstance& a, const SpriteInstance& b) {
		return a.mTexture < b.mTexture;
	}

	bool sortFrontToBack(const SpriteInstance& a, const SpriteInstance& b) {
		return a.mPosition.z < b.mPosition.z;
	}

	bool sortBackToFront(const SpriteInstance& a, const SpriteInstance& b) {
		return a.mPosition.z > b.mPosition.z;
	}

	void SpriteBatch::end() {
		switch (mMode) {
			case SpriteBatchMode::DEFERRED_SORT_TEXTURE:
				std::sort(mDeferredSprites.begin(), mDeferredSprites.end(), &textureSort);
				break;
			case SpriteBatchMode::DEFERRED_SORT_FRONT_TO_BACK:
				std::sort(mDeferredSprites.begin(), mDeferredSprites.end(), &sortFrontToBack);
				break;
			case SpriteBatchMode::DEFERRED_SORT_BACK_TO_FRONT:
				std::sort(mDeferredSprites.begin(), mDeferredSprites.end(), &sortBackToFront);
				break;
			case SpriteBatchMode::DEFERRED_IN_ORDER:
				break;
		}

		for (auto& inst : mDeferredSprites) {
			submitInstance(inst);
		}

		mDeferredSprites.clear();

		flush();
	}

	void SpriteBatch::init() {
		// Load samplers
		if (!mLinearSampler)
			mLinearSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, this);
		if (!mNearestSampler)
			mNearestSampler = load<Sampler>(POINT_CLAMP_SAMPLER_SRC, this);

		if (!mDefaultShader) {
			mDefaultShader = load<Shader>("internal/spritebatch.json", this);
			mDefaultShaderTex.find(mDefaultShader, "spriteTexture");
			mDefaultShaderTransform.find(mDefaultShader, "transformMatrix");
		}
	}
}