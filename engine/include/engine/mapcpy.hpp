#ifndef MAP_CPY_H_
#define MAP_CPY_H_

#include <memory>
#include <cstdint>
#include <cstring>

template <typename T, typename indexT>
void mapcpy(T* dest, const T* src, indexT map[], uint32_t mapSize) {
	for (uint32_t i = 0; i < mapSize; ++i) 
		if (map[i] >= 0)
			std::memcpy(&dest[map[i]], &src[i], sizeof(T));
}

#endif