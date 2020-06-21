#include "pch.h"
#include "CppUnitTest.h"
#include "../Engine/digraph.hpp"
#include "../Engine/digraph.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Morpheus;

namespace EngineTest
{
	TEST_CLASS(DigraphTest)
	{
	public:
		TEST_METHOD(DigraphUsageTest)
		{
			Digraph g(1, 1);
			auto d = g.createVertexData<double>("d");
			auto w = g.createEdgeData<double>("w");

			auto v1 = g.createVertex();
			d[v1] = 1.0;
			auto v2 = g.createVertex();
			auto v3 = g.createVertex();
			auto v4 = g.createVertex();

			auto e12 = g.createEdge(v1, v2);
			w[e12] = 1.0;
			auto e13 = g.createEdge(v1, v3);
			auto e23 = g.createEdge(v2, v3);
			auto e24 = g.createEdge(v2, v4);
			auto e34 = g.createEdge(v3, v4);
			w[e34] = 5.0;

			Assert::AreEqual(w[e12], 1.0);
			Assert::AreEqual(d[v1], 1.0);
			Assert::AreEqual(v1.outDegree(), 2u);
			Assert::AreEqual(v4.inDegree(), 2u);

			g.deleteVertex(v2);

			Assert::AreEqual(g.vertexCount(), 3u);
			Assert::AreEqual(g.edgeCount(), 2u);
			Assert::AreEqual(d[v1], 1.0);
			Assert::AreEqual(w[e34], 5.0);
			Assert::AreEqual(v1.outDegree(), 1u);
			Assert::AreEqual(v4.inDegree(), 1u);

			g.deleteEdge(e13);

			Assert::AreEqual(g.edgeCount(), 1u);
		}
	};
}
