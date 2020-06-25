#include "digraph.hpp"
#include "mapcpy.hpp"

#define DEFAULT_GRAPH_VERTEX_COUNT 128
#define DEFAULT_GRAPH_EDGE_COUNT 128
#define DEFAULT_RESCALE_FACTOR 2.0 

namespace Morpheus {
	Digraph::Digraph() :
		Digraph(DEFAULT_GRAPH_VERTEX_COUNT, 
			DEFAULT_GRAPH_EDGE_COUNT) { }

	Digraph::~Digraph()
	{
		delete[] vertices_;
		delete[] edges_;

		for (auto& data : vertexDatas)
			delete data.second;
		for (auto& data : edgeDatas)
			delete data.second;
	}

	Digraph::Digraph(uint32_t reserveVertices, uint32_t reserveEdges) :
		firstUnusedEdge(-1), 
		firstUnusedVertex(-1), 
		rescaleFactor(DEFAULT_RESCALE_FACTOR), 
		vertexActiveBlock_(0),
		edgeActiveBlock_(0),
		edgeCount_(0),
		vertexCount_(0) 
	{
		vertices_ = new DigraphVertexRaw[reserveVertices];
		vertexReserve_ = reserveVertices;
		edges_ = new DigraphEdgeRaw[reserveEdges];
		edgeReserve_ = reserveEdges;
	}

	void Digraph::resizeVertices(uint32_t newSize) {
		auto newMem = new DigraphVertexRaw[newSize];
		std::memcpy(newMem, vertices_, sizeof(DigraphVertexRaw) * vertexReserve_);
		delete[] vertices_;
		vertices_ = newMem;
		vertexReserve_ = newSize;

		for (auto& data : vertexDatas)
			data.second->resize(newSize);
	}

	void Digraph::resizeEdges(uint32_t newSize) {
		auto newMem = new DigraphEdgeRaw[newSize];
		std::memcpy(newMem, edges_, sizeof(DigraphEdgeRaw) * edgeReserve_);
		delete[] edges_;
		edges_ = newMem;
		edgeReserve_ = newSize;

		for (auto& data : edgeDatas)
			data.second->resize(newSize);
	}

	void Digraph::applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize) {
		for (auto& data : vertexDatas)
			data.second->compress(map, mapLen, newSize);

		for (auto& lookup : vertexLookups)
			lookup.second->applyMap(map, mapLen);
	}

	void Digraph::applyEdgeMap(const int map[], const uint32_t mapLen, const uint32_t newSize) {
		for (auto& data : edgeDatas)
			data.second->compress(map, mapLen, newSize);

		for (auto& lookup : edgeLookups)
			lookup.second->applyMap(map, mapLen);
	}

	DigraphVertex Digraph::createVertex() {
		if (firstUnusedVertex != -1) {
			auto next = vertices_[firstUnusedVertex].outEdge;
			auto id = firstUnusedVertex;

			vertices_[id].inEdge = -1;
			vertices_[id].outEdge = -1;

			firstUnusedVertex = next;
			vertexCount_++;
			return DigraphVertex(this, id);
		}
		else {
			// Resize buffer if necessary
			if (vertexCount_ == vertexReserve_)
				resizeVertices((uint32_t)(vertexReserve_ * rescaleFactor));

			vertices_[vertexCount_].inEdge = -1;
			vertices_[vertexCount_].outEdge = -1;
			vertexActiveBlock_++;
			return DigraphVertex(this, vertexCount_++);
		}
	}

	DigraphEdge Digraph::createEdge(int tail, int head) {

		int id;
		if (firstUnusedEdge != -1) {
			auto next = edges_[firstUnusedEdge].next;
			id = firstUnusedEdge;
			firstUnusedEdge = next;
		}
		else {
			// Resize buffer if necessary
			if (edgeCount_ == edgeReserve_)
				resizeEdges((uint32_t)(edgeReserve_ * rescaleFactor));

			id = edgeCount_;
			edgeActiveBlock_++;
		}

		edges_[id].head = head;
		edges_[id].tail = tail;
		edges_[id].prev = -1;
		edges_[id].dualPrev = -1;
		edges_[id].next = vertices_[tail].outEdge;
		edges_[id].dualNext = vertices_[head].inEdge;
		auto oldEdge = vertices_[tail].outEdge;
		if (oldEdge != -1)
			edges_[oldEdge].prev = id;
		vertices_[tail].outEdge = id;
		oldEdge = vertices_[head].inEdge;
		if (oldEdge != -1)
			edges_[oldEdge].dualPrev = id;
		vertices_[head].inEdge = id;

		edgeCount_++;

		return DigraphEdge(this, id);
	}

	void Digraph::deleteVertex(DigraphVertex& v) {
		for (auto inIt = v.getIngoingEdges(); inIt.valid(); ) {
			auto e = inIt();
			inIt.next();
			deleteEdge(e);
		}

		for (auto outIt = v.getOutgoingEdges(); outIt.valid(); ) {
			auto e = outIt();
			outIt.next();
			deleteEdge(e);
		}

		// Send vertex to graveyard
		auto id = v.id();
		vertices_[id].outEdge = firstUnusedVertex;
		vertices_[id].inEdge = GRAVEYARD_FLAG;
		firstUnusedVertex = id;

		--vertexCount_;
	}

	void Digraph::deleteVertex(int v) {
		auto vert = getVertex(v);
		deleteVertex(vert);
	}

	void Digraph::deleteEdge(DigraphEdge& e) {
		auto id = e.id();
		auto head = edges_[id].head;
		auto tail = edges_[id].tail;

		// Doubly linked list implementation
		if (vertices_[tail].outEdge == id)
			vertices_[tail].outEdge = edges_[id].next;
		if (vertices_[head].inEdge == id)
			vertices_[head].inEdge = edges_[id].dualNext;

		if (edges_[id].prev != -1)
			edges_[edges_[id].prev].next = edges_[id].next;
		if (edges_[id].next != -1)
			edges_[edges_[id].next].prev = edges_[id].prev;
		if (edges_[id].dualPrev != -1)
			edges_[edges_[id].dualPrev].dualNext = edges_[id].dualNext;
		if (edges_[id].dualNext != -1)
			edges_[edges_[id].dualNext].dualPrev = edges_[id].dualPrev;

		// Send edge to graveyard
		edges_[id].next = firstUnusedEdge;
		edges_[id].prev = GRAVEYARD_FLAG;
		firstUnusedEdge = id;

		--edgeCount_;
	}

	void Digraph::compress(bool bTight)
	{
		int* vmap = new int[vertexActiveBlock_];
		int* emap = new int[edgeActiveBlock_];

		uint32_t new_v = 0;
		for (uint32_t v = 0; v < vertexActiveBlock_; ++v) {
			if (vertices_[v].inEdge != GRAVEYARD_FLAG)
				vmap[v] = new_v++;
			else
				vmap[v] = -1;
		}

		uint32_t new_e = 0;
		for (uint32_t e = 0; e < edgeActiveBlock_; ++e) {
			if (edges_[e].prev != GRAVEYARD_FLAG)
				emap[e] = new_e++;
			else
				emap[e] = -1;
		}

		uint32_t newEdgeReserve = bTight ? new_e : (uint32_t)(new_e * rescaleFactor);
		uint32_t newVertexReserve = bTight ? new_v : (uint32_t)(new_v * rescaleFactor); 

		auto new_vdata = new DigraphVertexRaw[newVertexReserve];
		auto new_edata = new DigraphEdgeRaw[newEdgeReserve];

		mapcpy(new_vdata, vertices_, vmap, vertexActiveBlock_);
		mapcpy(new_edata, edges_, emap, edgeActiveBlock_);

		delete[] vertices_;
		delete[] edges_;

		vertices_ = new_vdata;
		edges_ = new_edata;

		vertexReserve_ = newVertexReserve;
		edgeReserve_ = newEdgeReserve;

		edgeActiveBlock_ = edgeCount_;
		vertexActiveBlock_ = vertexCount_;

		// Relabel vertices as necessary and update data and lookups
		applyVertexMap(vmap, vertexActiveBlock_, newVertexReserve);
		applyEdgeMap(emap, edgeActiveBlock_, newEdgeReserve);

		delete[] vmap;
		delete[] emap;
	}
}