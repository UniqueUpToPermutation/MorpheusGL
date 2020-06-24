#ifndef POOL_H_
#define POOL_H_

#include "mapcpy.hpp"

#include <stack>

namespace Morpheus {
	
	template <typename T>
	class Pool;
	template <typename T>
	class PoolHandle;

	class VoidPoolHandle {
	public:
		void* poolPtr;
		uint32_t offset;

		inline VoidPoolHandle(void* poolPtr, uint32_t offset) : poolPtr(poolPtr), offset(offset) { }
		template <typename T>
		inline VoidPoolHandle(PoolHandle<T>& handle);
		inline VoidPoolHandle() { }
	};

	/// <summary>
	/// A handle to an object in a pool.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="uint32_t"></typeparam>
	template <typename T>
	class PoolHandle {
	private:
		Pool<T>* poolPtr;
		uint32_t offset;

	public:
		inline PoolHandle(T* ptr, uint32_t offset);
		inline PoolHandle(VoidPoolHandle& handle);
		inline PoolHandle() { }

		inline T* operator->();
		inline T* get();

		friend class Pool<T>;
		friend class VoidPoolHandle;
	};

	/// <summary>
	/// A resizing pool for managing memory and mantaining memory coherence.
	/// </summary>
	/// <typeparam name="T">The object type.</typeparam>
	/// <typeparam name="uint32_t">The offset type in the pool (i.e. uint32)</typeparam>
	template <typename T>
	class Pool {
	private:
		T* data;
		std::stack<uint32_t> freeIndices;
		uint32_t freeBlockStart;
		uint32_t dataSize;
		double rescaleFactor;

	public:
		Pool(const uint32_t initialSize, const double rescaleFactor = 2.0d) :
			freeBlockStart(0), rescaleFactor(rescaleFactor) 
		{
			resize(initialSize);
		}

		~Pool() {
			delete[] data;
		}

		void resize(const uint32_t newSize) {
			// Resize pool
			auto old = data;
			data = new T[newSize];
			if (old) {
				std::memcpy(data, old, dataSize * sizeof(T));
				dataSize = newSize;
				delete[] old;
			}
		}

		/// <summary>
		/// Compresses the pool to save memory space.
		/// </summary>
		/// <param name="bTight">Whether or not to make the compression tight, if set to true, any additional alloc will require a resize.</param>
		void compress(const bool bTight = false) {
			uint32_t* map = new uint32_t[freeBlockStart];
			char* cull = new char[freeBlockStart];
			memset(cull, 0, sizeof(char) * freeBlockStart);

			while (!freeIndices.empty())
				cull[freeIndices.pop()] = true;
			
			for (uint32_t i = 0, new_i = 0; i < freeBlockStart; ++i)
				if (!cull[i])
					map[i] = new_i++;
				else
					map[i] = -1;

			uint32_t newSize = (uint32_t)(new_i * rescaleFactor);
			auto old = data;
			data = new T[newSize];
			mapcpy(data, old, map, freeBlockStart);
			dataSize = newSize;
			delete[] old;
			delete[] cull;
			delete[] map;
		}

		/// <summary>
		/// Allocate an object from the pool. Note the constructor of the object will not be called.
		/// </summary>
		/// <returns>A new object.</returns>
		PoolHandle<T> alloc() {
			size_t cnt = freeIndices.size();
			if (cnt > 0) {
				uint32_t of = freeIndices.pop();
				return PoolHandle<T, uint32_t>(&data[of], of);
			}
			else if (freeBlockStart == dataSize) {
				resize((uint32_t)(dataSize * rescaleFactor));
				uint32_t of = freeBlockStart++;
				return PoolHandle<T, uint32_t>(&data[of], of);
			}
			else {
				uint32_t of = freeBlockStart++;
				return PoolHandle<T, uint32_t>(&data[of], of);
			}
		}

		/// <summary>
		/// Frees an object in the pool.
		/// </summary>
		/// <param name="h">Handle to the object to free</param>
		void dealloc(PoolHandle<T, uint32_t>& h) {
			freeIndices.push(h.offset);
		}

		friend class PoolHandle<T, uint32_t>;
	};

	template<typename T>
	inline PoolHandle<T>::PoolHandle(T* ptr, uint32_t offset) 
		: poolPtr(ptr), offset(offset)
	{
	}

	template<typename T>
	inline T* PoolHandle<T>::operator->()
	{
		return &poolPtr->data[offset];
	}
	template<typename T>
	inline T* PoolHandle<T>::get()
	{
		return &poolPtr->data[offset];
	}

	template <typename T>
	PoolHandle<T>::PoolHandle(VoidPoolHandle& handle) : poolPtr((Pool<T>)*handle.poolPtr), offset(handle.offset) {
	}

	template <typename T>
	VoidPoolHandle::VoidPoolHandle(PoolHandle<T>& handle) : poolPtr(handle.poolPtr), offset(handle.offset) {
	}
}

#endif