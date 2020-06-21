#ifndef POOL_H_
#define POOL_H_

#include "mapcpy.hpp"

#include <stack>

namespace Morpheus {
	
	template <typename T, typename offsetType>
	class Pool;

	/// <summary>
	/// A handle to an object in a pool.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="offsetType"></typeparam>
	template <typename T, typename offsetType=uint32_t>
	class PoolHandle {
	private:
		Pool<T, offsetType>* poolPtr;
		offsetType offset;

	public:
		PoolHandle(T* ptr, offsetType offset);

		inline T* operator->();
		inline T* get();

		friend class Pool<T, offsetType>;
	};

	/// <summary>
	/// A resizing pool for managing memory and mantaining memory coherence.
	/// </summary>
	/// <typeparam name="T">The object type.</typeparam>
	/// <typeparam name="offsetType">The offset type in the pool (i.e. uint32)</typeparam>
	template <typename T, typename offsetType=uint32_t>
	class Pool {
	private:
		T* data;
		std::stack<offsetType> freeIndices;
		offsetType freeBlockStart;
		offsetType dataSize;
		double rescaleFactor;

	public:
		Pool(const offsetType initialSize, const double rescaleFactor = 2.0d) :
			freeBlockStart(0), rescaleFactor(rescaleFactor) 
		{
			resize(initialSize);
		}

		~Pool() {
			delete[] data;
		}

		void resize(const offsetType newSize) {
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
			offsetType* map = new offsetType[freeBlockStart];
			char* cull = new char[freeBlockStart];
			memset(cull, 0, sizeof(char) * freeBlockStart);

			while (!freeIndices.empty())
				cull[freeIndices.pop()] = true;
			
			for (offsetType i = 0, new_i = 0; i < freeBlockStart; ++i)
				if (!cull[i])
					map[i] = new_i++;
				else
					map[i] = -1;

			offsetType newSize = (offsetType)(new_i * rescaleFactor);
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
		PoolHandle<T, offsetType> alloc() {
			size_t cnt = freeIndices.size();
			if (cnt > 0) {
				offsetType of = freeIndices.pop();
				return PoolHandle<T, offsetType>(&data[of], of);
			}
			else if (freeBlockStart == dataSize) {
				resize((offsetType)(dataSize * rescaleFactor));
				offsetType of = freeBlockStart++;
				return PoolHandle<T, offsetType>(&data[of], of);
			}
			else {
				offsetType of = freeBlockStart++;
				return PoolHandle<T, offsetType>(&data[of], of);
			}
		}

		/// <summary>
		/// Frees an object in the pool.
		/// </summary>
		/// <param name="h">Handle to the object to free</param>
		void dealloc(PoolHandle<T, offsetType>& h) {
			freeIndices.push(h.offset);
		}

		friend class PoolHandle<T, offsetType>;
	};

	template<typename T, typename offsetType>
	inline PoolHandle<T, offsetType>::PoolHandle(T* ptr, offsetType offset) 
		: poolPtr(ptr), offset(offset)
	{
	}

	template<typename T, typename offsetType>
	inline T* PoolHandle<T, offsetType>::operator->()
	{
		return &poolPtr->data[offset];
	}
	template<typename T, typename offsetType>
	inline T* PoolHandle<T, offsetType>::get()
	{
		return &poolPtr->data[offset];
	}
}

#endif