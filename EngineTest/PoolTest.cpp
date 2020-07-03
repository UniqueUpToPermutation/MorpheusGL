#include "pch.h"
#include "CppUnitTest.h"
#include "../Engine/pool.hpp"
#include "../Engine/pool.hpp"

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Morpheus;

namespace EngineTest
{
	TEST_CLASS(PoolTest)
	{
	public:
		TEST_METHOD(PoolUsageTest)
		{
			Pool<int> a;
			auto handle = a.alloc();
			auto handle2 = a.alloc();
			*handle.get() = 3;
			*handle2.get() = 4;
			
			std::vector<PoolHandle<int>> handles;
			for (auto i = 0; i < 100; ++i) {
				handles.push_back(a.alloc());
			}

			Assert::AreEqual(*handle.get(), 3);

			a.dealloc(handle);

			Assert::AreEqual(*handle2.get(), 4);

			a.dealloc(handle2);

			handle = a.alloc();
			handle2 = a.alloc();

			*handle.get() = 2;

			a.dealloc(handle2);

			Assert::AreEqual(*handle.get(), 2);
		}
	};
}
