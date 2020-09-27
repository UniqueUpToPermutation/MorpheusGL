#include <engine/digraph.hpp>
#include <engine/mapcpy.hpp>

#define DEFAULT_GRAPH_VERTEX_COUNT 128
#define DEFAULT_GRAPH_EDGE_COUNT 128
#define DEFAULT_RESCALE_FACTOR 2.0 

namespace Morpheus {
	Digraph::Digraph() :
		Digraph(DEFAULT_GRAPH_VERTEX_COUNT, 
			DEFAULT_GRAPH_EDGE_COUNT) { }

	Digraph::~Digraph()
	{
		delete[] mVertices;
		delete[] mEdges;

		for (auto& data : mVertexDatas)
			delete data.second;
		for (auto& data : mEdgeDatas)
			delete data.second;
		for (auto& lookup : mVertexLookups)
			delete lookup.second;
		for (auto& lookup : mEdgeLookups)
			delete lookup.second;
	}

	Digraph::Digraph(uint32_t reserveVertices, uint32_t reserveEdges) :
		mFirstUnusedEdge(-1), 
		mFirstUnusedVertex(-1), 
		mRescaleFactor(DEFAULT_RESCALE_FACTOR), 
		mVertexActiveBlock(0),
		mEdgeActiveBlock(0),
		mEdgeCount(0),
		mVertexCount(0),
		mDatasCreated(0),
		mLookupsCreated(0)
	{
		mVertices = new DigraphVertexRaw[reserveVertices];
		mVertexReserve = reserveVertices;
		mEdges = new DigraphEdgeRaw[reserveEdges];
		mEdgeReserve = reserveEdges;
	}

	void Digraph::resizeVertices(uint32_t newSize) {
		auto newMem = new DigraphVertexRaw[newSize];
		std::memcpy(newMem, mVertices, sizeof(DigraphVertexRaw) * mVertexReserve);
		delete[] mVertices;
		mVertices = newMem;
		mVertexReserve = newSize;

		for (auto& data : mVertexDatas)
			data.second->resize(newSize);
	}

	void Digraph::resizeEdges(uint32_t newSize) {
		auto newMem = new DigraphEdgeRaw[newSize];
		std::memcpy(newMem, mEdges, sizeof(DigraphEdgeRaw) * mEdgeReserve);
		delete[] mEdges;
		mEdges = newMem;
		mEdgeReserve = newSize;

		for (auto& data : mEdgeDatas)
			data.second->resize(newSize);
	}

	void Digraph::applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize) {
		for (auto& data : mVertexDatas)
			data.second->compress(map, mapLen, newSize);

		for (auto& lookup : mVertexLookups)
			lookup.second->applyMap(map, mapLen);
	}

	void Digraph::applyEdgeMap(const int map[], const uint32_t mapLen, const uint32_t newSize) {
		for (auto& data : mEdgeDatas)
			data.second->compress(map, mapLen, newSize);

		for (auto& lookup : mEdgeLookups)
			lookup.second->applyMap(map, mapLen);
	}

	DigraphVertex Digraph::createVertex() {
		if (mFirstUnusedVertex != -1) {
			auto next = mVertices[mFirstUnusedVertex].mOutEdge;
			auto id = mFirstUnusedVertex;

			mVertices[id].mInEdge = -1;
			mVertices[id].mOutEdge = -1;

			mFirstUnusedVertex = next;
			mVertexCount++;
			return DigraphVertex(this, id);
		}
		else {
			// Resize buffer if necessary
			if (mVertexCount == mVertexReserve)
				resizeVertices((uint32_t)(mVertexReserve * mRescaleFactor));

			mVertices[mVertexCount].mInEdge = -1;
			mVertices[mVertexCount].mOutEdge = -1;
			mVertexActiveBlock++;
			return DigraphVertex(this, mVertexCount++);
		}
	}

	DigraphEdge Digraph::createEdge(int tail, int head) {

		int id;
		if (mFirstUnusedEdge != -1) {
			auto next = mEdges[mFirstUnusedEdge].mNext;
			id = mFirstUnusedEdge;
			mFirstUnusedEdge = next;
		}
		else {
			// Resize buffer if necessary
			if (mEdgeCount == mEdgeReserve)
				resizeEdges((uint32_t)(mEdgeReserve * mRescaleFactor));

			id = mEdgeCount;
			mEdgeActiveBlock++;
		}

		mEdges[id].mHead = head;
		mEdges[id].mTail = tail;
		mEdges[id].mPrev = -1;
		mEdges[id].mDualPrev = -1;
		mEdges[id].mNext = mVertices[tail].mOutEdge;
		mEdges[id].mDualNext = mVertices[head].mInEdge;
		auto oldEdge = mVertices[tail].mOutEdge;
		if (oldEdge != -1)
			mEdges[oldEdge].mPrev = id;
		mVertices[tail].mOutEdge = id;
		oldEdge = mVertices[head].mInEdge;
		if (oldEdge != -1)
			mEdges[oldEdge].mDualPrev = id;
		mVertices[head].mInEdge = id;

		mEdgeCount++;

		return DigraphEdge(this, id);
	}

	void Digraph::deleteVertex(DigraphVertex v) {
		for (auto inIt = v.incomming(); inIt.valid(); ) {
			auto e = inIt();
			inIt.next();
			deleteEdge(e);
		}

		for (auto outIt = v.outgoing(); outIt.valid(); ) {
			auto e = outIt();
			outIt.next();
			deleteEdge(e);
		}

		// Send vertex to graveyard
		auto id = v.id();
		mVertices[id].mOutEdge = mFirstUnusedVertex;
		mVertices[id].mInEdge = GRAVEYARD_FLAG;
		mFirstUnusedVertex = id;

		--mVertexCount;
	}

	void Digraph::deleteVertex(int v) {
		auto vert = getVertex(v);
		deleteVertex(vert);
	}

	void Digraph::deleteEdge(DigraphEdge e) {
		auto id = e.id();
		auto head = mEdges[id].mHead;
		auto tail = mEdges[id].mTail;

		// Doubly linked list implementation
		if (mVertices[tail].mOutEdge == id)
			mVertices[tail].mOutEdge = mEdges[id].mNext;
		if (mVertices[head].mInEdge == id)
			mVertices[head].mInEdge = mEdges[id].mDualNext;

		if (mEdges[id].mPrev != -1)
			mEdges[mEdges[id].mPrev].mNext = mEdges[id].mNext;
		if (mEdges[id].mNext != -1)
			mEdges[mEdges[id].mNext].mPrev = mEdges[id].mPrev;
		if (mEdges[id].mDualPrev != -1)
			mEdges[mEdges[id].mDualPrev].mDualNext = mEdges[id].mDualNext;
		if (mEdges[id].mDualNext != -1)
			mEdges[mEdges[id].mDualNext].mDualPrev = mEdges[id].mDualPrev;

		// Send edge to graveyard
		mEdges[id].mNext = mFirstUnusedEdge;
		mEdges[id].mPrev = GRAVEYARD_FLAG;
		mFirstUnusedEdge = id;

		--mEdgeCount;
	}

	void Digraph::compress(bool bTight)
	{
		int* vmap = new int[mVertexActiveBlock];
		int* emap = new int[mEdgeActiveBlock];

		uint32_t new_v = 0;
		for (uint32_t v = 0; v < mVertexActiveBlock; ++v) {
			if (mVertices[v].mInEdge != GRAVEYARD_FLAG)
				vmap[v] = new_v++;
			else
				vmap[v] = -1;
		}

		uint32_t new_e = 0;
		for (uint32_t e = 0; e < mEdgeActiveBlock; ++e) {
			if (mEdges[e].mPrev != GRAVEYARD_FLAG)
				emap[e] = new_e++;
			else
				emap[e] = -1;
		}

		uint32_t newEdgeReserve = bTight ? new_e : (uint32_t)(new_e * mRescaleFactor);
		uint32_t newVertexReserve = bTight ? new_v : (uint32_t)(new_v * mRescaleFactor); 

		auto new_vdata = new DigraphVertexRaw[newVertexReserve];
		auto new_edata = new DigraphEdgeRaw[newEdgeReserve];

		mapcpy(new_vdata, mVertices, vmap, mVertexActiveBlock);
		mapcpy(new_edata, mEdges, emap, mEdgeActiveBlock);

		delete[] mVertices;
		delete[] mEdges;

		mVertices = new_vdata;
		mEdges = new_edata;

		mVertexReserve = newVertexReserve;
		mEdgeReserve = newEdgeReserve;

		mEdgeActiveBlock = mEdgeCount;
		mVertexActiveBlock = mVertexCount;

		// Relabel vertices as necessary and update data and lookups
		applyVertexMap(vmap, mVertexActiveBlock, newVertexReserve);
		applyEdgeMap(emap, mEdgeActiveBlock, newEdgeReserve);

		delete[] vmap;
		delete[] emap;
	}

	DigraphBreadthFirstSearch::DigraphBreadthFirstSearch(DigraphVertex& start) {
		digraph = start.graph();
		mVisited = digraph->createVertexData<bool>();
		mVisited.memset(false);
		mVertexQueue.push(start.id());
	}

	DigraphBreadthFirstSearch::DigraphBreadthFirstSearch() : digraph(nullptr) { }

	DigraphBreadthFirstSearch::~DigraphBreadthFirstSearch() {
		if (digraph)
			digraph->destroyData(mVisited);
	}

	void DigraphBreadthFirstSearch::restart(DigraphVertex& start) {
		if (!digraph) {
			digraph = start.graph();
			mVisited = digraph->createVertexData<bool>();
			mVisited.memset(false);
		}
		mVertexQueue = std::queue<int>();
		mVertexQueue.push(start.id());
	}

	DigraphDepthFirstSearch::DigraphDepthFirstSearch(DigraphVertex& start) {
		digraph = start.graph();
		mVisited = digraph->createVertexData<bool>();
		mVisited.memset(false);
		mVertexStack.push(start.id());
	}

	DigraphDepthFirstSearch::DigraphDepthFirstSearch() : digraph(nullptr) { }

	DigraphDepthFirstSearch::~DigraphDepthFirstSearch() {
		if (digraph)
			digraph->destroyData(mVisited);
	}

	void DigraphDepthFirstSearch::restart(DigraphVertex& start) {
		if (!digraph) {
			digraph = start.graph();
			mVisited = digraph->createVertexData<bool>();
			mVisited.memset(false);
		}
		mVertexStack = std::stack<int>();
		mVertexStack.push(start.id());
	}
}