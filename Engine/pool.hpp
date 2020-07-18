#ifndef POOL_H_
#define POOL_H_

#include "mapcpy.hpp"

#include <stack>
#include <algorithm>

namespace Morpheus {
	
	template <typename T>
	class Pool;
	template <typename T>
	class PoolHandle;

	/// <summary>
	/// A handle to an object in a pool.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="uint32_t"></typeparam>
	template <typename T>
	class PoolHandle {
	private:
		Pool<T>* mPoolPtr;
		uint32_t mOffset;

	public:
		inline PoolHandle(Pool<T>* ptr, uint32_t offset);
		inline PoolHandle() { }

		inline T* operator->();
		inline T* get() const;
		inline Pool<T>* getPool();
		inline bool operator==(const PoolHandle& other);

		friend class Pool<T>;
		friend class PoolHandle<void>;
	};

	template <>
	class PoolHandle<void> {
	public:
		void* mPoolPtr;
		uint32_t mOffset;

		inline PoolHandle(void* poolPtr, uint32_t offset) : mPoolPtr(poolPtr), mOffset(offset) { }
		template <typename T>
		inline PoolHandle(const PoolHandle<T>& handle);
		inline PoolHandle() { }
		template <typename T>
		inline PoolHandle<T> reinterpret() const {
			return PoolHandle<T>((Pool<T>*)mPoolPtr, mOffset);
		}
	};

	/// <summary>
	/// A resizing pool for managing memory and mantaining memory coherence.
	/// </summary>
	/// <typeparam name="T">The object type.</typeparam>
	template <typename T>
	class Pool {
	private:
		T* mData;
		std::stack<uint32_t> mFreeIndices;
		uint32_t mFreeBlockStart;
		uint32_t mDataSize;
		double mRescaleFactor;

	public:
		Pool() : mFreeBlockStart(0), mRescaleFactor(2.0) {
			resize(0);
		}

		Pool(const uint32_t initialSize, const double rescaleFactor = 2.0d) :
			mFreeBlockStart(0), mRescaleFactor(rescaleFactor) 
		{
			resize(initialSize);
		}

		~Pool() {
			delete[] mData;
		}

		inline T* at(const uint32_t offset) {
			return &mData[offset];
		}

		void resize(const uint32_t newSize) {
			// Resize pool
			auto old = mData;
			if (newSize > 0)
				mData = new T[newSize];
			else
				mData = nullptr;
			
			if (old) {
				std::memcpy(mData, old, mDataSize * sizeof(T));
				delete[] old;
			}

			mDataSize = newSize;
			mFreeBlockStart = std::min(mDataSize, mFreeBlockStart);
		}

		/// <summary>
		/// Compresses the pool to save memory space.
		/// </summary>
		/// <param name="bTight">Whether or not to make the compression tight, if set to true, any additional alloc will require a resize.</param>
		void compress(const bool bTight = false) {
			uint32_t* map = new uint32_t[mFreeBlockStart];
			char* cull = new char[mFreeBlockStart];
			memset(cull, 0, sizeof(char) * mFreeBlockStart);

			while (!mFreeIndices.empty())
				cull[mFreeIndices.pop()] = true;
			
			uint32_t new_i = 0;
			for (uint32_t i = 0; i < mFreeBlockStart; ++i)
				if (!cull[i])
					map[i] = new_i++;
				else
					map[i] = -1;

			uint32_t newSize = (uint32_t)(new_i * mRescaleFactor);
			auto old = mData;
			mData = new T[newSize];
			mapcpy(mData, old, map, mFreeBlockStart);
			mDataSize = newSize;
			delete[] old;
			delete[] cull;
			delete[] map;
		}

		/// <summary>
		/// Allocate an object from the pool. Note the constructor of the object will not be called.
		/// </summary>
		/// <returns>A new object.</returns>
		PoolHandle<T> alloc() {
			size_t cnt = mFreeIndices.size();
			if (cnt > 0) {
				uint32_t of = mFreeIndices.top();
				mFreeIndices.pop();
				return PoolHandle<T>(this, of);
			}
			else if (mFreeBlockStart == mDataSize) {
				if (mDataSize == 0)
					mDataSize = 50;
				resize((uint32_t)(mDataSize * mRescaleFactor));
				uint32_t of = mFreeBlockStart++;
				return PoolHandle<T>(this, of);
			}
			else {
				uint32_t of = mFreeBlockStart++;
				return PoolHandle<T>(this, of);
			}
		}

		/// <summary>
		/// Frees an object in the pool.
		/// </summary>
		/// <param name="h">Handle to the object to free</param>
		void dealloc(PoolHandle<T>& h) {
			mFreeIndices.push(h.mOffset);
		}

		friend class PoolHandle<T>;
	};

	template<typename T>
	inline PoolHandle<T>::PoolHandle(Pool<T>* ptr, uint32_t offset) 
		: mPoolPtr(ptr), mOffset(offset)
	{
	}

	template<typename T>
	inline T* PoolHandle<T>::operator->()
	{
		return &mPoolPtr->mData[mOffset];
	}
	template<typename T>
	inline T* PoolHandle<T>::get() const
	{
		return &mPoolPtr->mData[mOffset];
	}
	template<typename T>
	inline Pool<T>* PoolHandle<T>::getPool() {
		return mPoolPtr;
	}
	template<typename T>
	inline bool PoolHandle<T>::operator==(const PoolHandle& other) {
		return mPoolPtr == other.mPoolPtr && mOffset == other.mOffset;
	}

	template <typename T>
	PoolHandle<void>::PoolHandle(const PoolHandle<T>& handle) : mPoolPtr(handle.mPoolPtr), mOffset(handle.mOffset) {
	}
}

#endif