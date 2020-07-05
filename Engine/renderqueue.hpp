#pragma once

namespace Morpheus {
	template <typename T>
	class RenderQueue {
	private:
		T* mMem;
		size_t mSize;
		size_t mAlloc;

	public:
		inline T& operator[](const uint32_t i)	{ return mMem[i]; }
		inline size_t size() const				{ return mSize; }
		inline T* begin() const					{ return mMem; }
		inline T* end() const					{ return &mMem[mSize]; }
		inline void clear()						{ mSize = 0; }

		inline void reserve(const size_t n) {
			if (mMem)
				delete mMem;
			mMem = new T[n];
			mAlloc = n;
		}

		inline void resize(const size_t n) {
			if (mMem) {
				T* newPtr = new T[n];
				std::memcpy(newPtr, mMem, sizeof(T) * mSize);
				delete[] mMem;
				mMem = newPtr;
			}
			else
				reserve(n);
		}

		RenderQueue(const size_t initialSize) {
			mMem = nullptr;
			reserve(initialSize);
			mSize = 0;
		}

		RenderQueue() {
			mMem = nullptr;
			mSize = 0;
			mAlloc = 0;
		}

		~RenderQueue() {
			if (mMem)
				delete[] mMem;
		}

		inline void push(const T& t) {
			if (mSize == mAlloc)
				resize(std::max(mSize, 1u) * 2);
			mMem[mSize] = t;
			++mSize;
		}
	};
}