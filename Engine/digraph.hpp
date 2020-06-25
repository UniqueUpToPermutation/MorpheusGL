#ifndef DI_GRAPH_H_
#define DI_GRAPH_H_

#define GRAVEYARD_FLAG -1000

#include <vector>
#include <unordered_map>
#include <string>

#include "mapcpy.hpp"

namespace Morpheus {

	class Digraph;
	class DigraphVertexIteratorF;
	class DigraphVertexIteratorB;
	class DigraphEdgeIteratorF;
	class DigraphEdgeIteratorB;
	class DigraphEdgeIteratorAll;
	class DigraphVertexIteratorAll;
	class IDigraphData;
	template <typename T>
	class DigraphEdgeData;
	template <typename T>
	class DigraphVertexData;
	template <typename T>
	struct DigraphDataView;
	template <typename T>
	struct DigraphVertexLookupView;
	template <typename T>
	struct DigraphEdgeLookupView;

	struct DigraphVertexRaw {
		int inEdge;
		int outEdge;
	};

	struct DigraphEdgeRaw {
		int head;
		int tail;
		int next;
		int prev;
		int dualNext;
		int dualPrev;
	};

	class DigraphIterator {
	protected:
		Digraph* ptr;
		int current;

	public:
		inline explicit DigraphIterator(Digraph* ptr, int current) :
			ptr(ptr), current(current) { }

		inline bool valid() { return current != -1; }
		inline int id() const { return current; }
	};

	class DigraphVertex {
	private:
		Digraph* ptr;
		int id_;

	public:
		inline explicit DigraphVertex() : ptr(nullptr), id_(-1) { }
		inline explicit DigraphVertex(Digraph* ptr, int id) :
			ptr(ptr), id_(id) { }

		inline DigraphEdgeIteratorF getOutgoingEdges();
		inline DigraphEdgeIteratorB getIngoingEdges();
		inline DigraphVertexIteratorF getOutgoingNeighbors();
		inline DigraphVertexIteratorB getIngoingNeighbors();
		inline int id() const { return id_; }
		inline uint32_t outDegree();
		inline uint32_t inDegree();

		template <typename T>
		inline T& data(const std::string& s);
	};

	class DigraphEdge {
	private:
		Digraph* ptr;
		int id_;

	public:
		inline explicit DigraphEdge() : ptr(nullptr), id_(-1) { }
		inline explicit DigraphEdge(Digraph* ptr, int id) :
			ptr(ptr), id_(id) { }

		inline DigraphVertex head();
		inline DigraphVertex tail();
		inline int id() const;

		template <typename T>
		inline T& data(const std::string& s);
	};

	class IDigraphLookup;
	template <typename T>
	class DigraphLookup;

	/// <summary>
	/// A directed graph class which features resizing memory and recycling of destroyed edges and vertices.
	/// </summary>
	class Digraph {
	private:
		DigraphVertexRaw* vertices_;
		DigraphEdgeRaw* edges_;
		uint32_t vertexCount_;
		uint32_t edgeCount_;
		uint32_t vertexReserve_;
		uint32_t edgeReserve_;
		uint32_t vertexActiveBlock_;
		uint32_t edgeActiveBlock_;
		int firstUnusedEdge;
		int firstUnusedVertex;
		float rescaleFactor;
		std::unordered_map<std::string, IDigraphData*> vertexDatas;
		std::unordered_map<std::string, IDigraphData*> edgeDatas;
		std::unordered_map<std::string, IDigraphLookup*> vertexLookups;
		std::unordered_map<std::string, IDigraphLookup*> edgeLookups;

		void resizeVertices(uint32_t newSize);
		void resizeEdges(uint32_t newSize);

		void applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize);
		void applyEdgeMap(const int map[], const uint32_t mapLen, const uint32_t newSize);

	public:
		Digraph();
		~Digraph();
		Digraph(uint32_t reserveVertices, uint32_t reserveEdges);

		DigraphVertex createVertex();
		DigraphEdge createEdge(int tail, int head);
		inline DigraphEdge createEdge(DigraphVertex& tail, DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		void deleteVertex(DigraphVertex& v);
		void deleteVertex(int v);
		void deleteEdge(DigraphEdge& e);
		void compress(bool bTight = false);

		uint32_t edgeCount() const { return edgeCount_; }
		uint32_t vertexCount() const { return vertexCount_; }

		inline uint32_t vertexReserve() const { return vertexReserve_; }
		inline uint32_t edgeReserve() const { return edgeReserve_; }
		inline uint32_t vertexActiveBlock() const { return vertexActiveBlock_; }
		inline uint32_t edgeActiveBlock() const { return edgeActiveBlock_; }

		inline DigraphVertex getVertex(int id) { return DigraphVertex(this, id); }
		inline DigraphEdge getEdge(int id) { return DigraphEdge(this, id); }

		inline DigraphEdgeIteratorAll edges();
		inline DigraphVertexIteratorAll vertices();

		template <typename T>
		DigraphDataView<T> createEdgeData(const std::string& name);

		template <typename T>
		DigraphDataView<T> createVertexData(const std::string& name);

		template <typename T>
		DigraphVertexLookupView<T> createVertexLookup(const std::string& name);

		template <typename T>
		DigraphEdgeLookupView<T> createEdgeLookup(const std::string& name);

		template <typename T>
		void destroyData(DigraphDataView<T>& view);

		template <typename T>
		inline DigraphDataView<T> getEdgeData(const std::string& name);

		template <typename T>
		inline DigraphDataView<T> getVertexData(const std::string& name);

		friend class DigraphVertex;
		friend class DigraphEdge;
		friend class DigraphIterator;
		friend class DigraphVertexIteratorF;
		friend class DigraphEdgeIteratorF;
		friend class DigraphVertexIteratorB;
		friend class DigraphEdgeIteratorB;
		friend class DigraphEdgeIterator;
		friend class DigraphVertexIteratorAll;
		friend class DigraphEdgeIteratorAll;
	};

	enum class DigraphDataType {
		UNKNOWN,
		EDGE,
		VERTEX
	};

	template <typename T>
	class DigraphData;

	class IDigraphData {
	protected:
		virtual void resize(uint32_t newSize) = 0;
		virtual void compress(const int map[], uint32_t mapSize, uint32_t newSize) = 0;

	public:

		template <typename T>
		inline DigraphData<T>* as() { return (DigraphData<T>*)this; }

		virtual std::string name() const = 0;

		virtual ~IDigraphData() { }

		friend class Digraph;
	};

	template <typename T>
	class DigraphData : public IDigraphData {
	protected:
		Digraph* parent;
		T* data;
		uint32_t dataSize;
		std::string name_;

		virtual DigraphDataType type() const {
			return DigraphDataType::UNKNOWN;
		}

		void resize(const uint32_t newSize) override {
			auto new_data = new T[newSize];
			memcpy(new_data, data, sizeof(T) * dataSize);
			delete[] data;
			data = new_data;
			dataSize = newSize;
		}
		void compress(const int map[], const uint32_t mapSize, const uint32_t newSize) override {
			auto new_data = new T[newSize];
			mapcpy(new_data, data, map, mapSize);
			delete[] data;
			data = new_data;
			dataSize = newSize;
		}

	public:

		std::string name() const override {
			return name_;
		}

		explicit DigraphData(Digraph* parent, const std::string& name) :
			parent(parent), name_(name) {

			switch (type()) {
			case DigraphDataType::EDGE:
				dataSize = parent->edgeReserve();
				break;
			case DigraphDataType::VERTEX:
				dataSize = parent->vertexReserve();
				break;
			default:
				dataSize = parent->vertexReserve();
				break;
			}

			data = new T[parent->vertexReserve()];
		}

		~DigraphData() override
		{
			delete[] data;
		}

		friend struct DigraphDataView<T>;
	};


	enum class DigraphLookupType {
		VERTEX,
		EDGE
	};

	class IDigraphLookup {
	public:
		virtual void applyMap(const int map[], const uint32_t mapSize) = 0;
	};

	template <typename T>
	class DigraphLookup : public IDigraphLookup {
	private:
		Digraph* parent;
		DigraphLookupType type_;
		std::unordered_map<T, int> TtoId;

	protected:
		void applyMap(const int map[], const uint32_t mapSize) override {
			for (auto& item : TtoId)
				item.second = map[item.second];
		}

	public:
		inline void setv(const DigraphVertex& v, const T& t) {
			TtoId[t] = v.id();
		}
		inline void sete(const DigraphEdge& e, const T& t) {
			TtoId[t] = e.id();
		}
		inline DigraphVertex getv(const T& t) {
			return parent->getVertex(TtoId[t]);
		}
		inline DigraphEdge gete(const T& t) {
			return parent->getEdge(TtoId[t]);
		}

		inline explicit DigraphLookup(Digraph* parent, const DigraphLookupType type) :
			parent(parent), type_(type) { }

		friend class Digraph;
		friend struct DigraphVertexLookupView<T>;
		friend struct DigraphEdgeLookupView<T>;
	};

	template <typename T>
	struct DigraphVertexLookupView {
	private:
		DigraphLookup<T>* lookup;

	public:
		inline DigraphVertexLookupView(DigraphLookup<T>* lookup) : lookup(lookup) { }
		inline DigraphVertexLookupView() : lookup(nullptr) { }

		inline DigraphVertex operator[](const T& t) {
			return lookup->parent->getVertex(lookup->TtoId[t]);
		}
		inline void set(const DigraphVertex& v, const T& t) {
			lookup->TtoId[t] = v.id();
		}
		inline void clear(const T& t) {
			lookup->TtoId.erase(t);
		}
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = lookup->TtoId.find(t);
			if (it == lookup->TtoId.end())
				return false;
			else {
				*out = lookup->parent->getVertex(it->second);
				return true;
			}
		}
	};

	template <typename T>
	struct DigraphEdgeLookupView {
	private:
		DigraphLookup<T>* lookup;

	public:
		inline DigraphEdgeLookupView(DigraphLookup<T>* lookup) : lookup(lookup) { }
		inline DigraphEdgeLookupView() : lookup(nullptr) { }

		inline DigraphEdge operator[](const T& t) {
			lookup->parent->getEdge(lookup->TtoId[t]);
		}
		inline void set(const DigraphEdge& e, const T& t) {
			lookup->TtoId[t] = e.id();
		}
		inline void clear(const T& t) {
			lookup->TtoId.erase(t);
		}
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = lookup->TtoId.find(t);
			if (it == lookup->TtoId.end())
				return false;
			else {
				*out = lookup->parent->getVertex(it->second);
				return true;
			}
		}
	};

	template <typename T>
	class DigraphEdgeData : public DigraphData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::EDGE;
		}

	public:
		explicit inline DigraphEdgeData(Digraph* parent, const std::string& name) :
			DigraphData<T>(parent, name) { }
	};

	template <typename T>
	class DigraphVertexData : public DigraphData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::VERTEX;
		}

	public:
		explicit inline DigraphVertexData(Digraph* parent, const std::string& name) :
			DigraphData<T>(parent, name) { }
	};

	template <typename T>
	struct DigraphDataView {
	private:
		DigraphData<T>* ptr;

	public:
		explicit inline DigraphDataView(DigraphData<T>* ptr) : ptr(ptr) { }
		explicit inline DigraphDataView() : ptr(nullptr) { }

		inline DigraphDataType type() const { return ptr->type(); }
		inline T& operator[](const int id) { return ptr->data[id]; }
		inline T& operator[](const DigraphVertex& v) { return ptr->data[v.id()]; }
		inline T& operator[](const DigraphEdge& e) { return ptr->data[e.id()]; }
		inline std::string& name() { return ptr->name(); }

		friend class Digraph;
	};

	class DigraphVertexIteratorF : public DigraphIterator {
	public:
		inline explicit DigraphVertexIteratorF(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphVertex operator()() {
			return DigraphVertex(ptr, ptr->edges_[current].head);
		}
		inline void next() {
			current = ptr->edges_[current].next;
		}
	};

	class DigraphVertexIteratorB : public DigraphIterator {
	public:
		inline explicit DigraphVertexIteratorB(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphVertex operator()() {
			return DigraphVertex(ptr, ptr->edges_[current].tail);
		}
		inline void next() {
			current = ptr->edges_[current].dualNext;
		}
	};

	class DigraphEdgeIterator : public DigraphIterator {
	public:
		inline explicit DigraphEdgeIterator(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphEdge operator()() {
			return DigraphEdge(ptr, current);
		}
	};

	class DigraphEdgeIteratorF : public DigraphEdgeIterator {
	public:
		inline explicit DigraphEdgeIteratorF(Digraph* ptr, int id) :
			DigraphEdgeIterator(ptr, id) { }

		inline void next() {
			current = ptr->edges_[current].next;
		}
	};

	class DigraphEdgeIteratorB : public DigraphEdgeIterator {
	public:
		inline explicit DigraphEdgeIteratorB(Digraph* ptr, int id) :
			DigraphEdgeIterator(ptr, id) { }

		inline void next() {
			current = ptr->edges_[current].dualNext;
		}
	};

	class DigraphVertexIteratorAll : public DigraphIterator {
	protected:
		inline void findNextValid() {
			while (true) {
				if (current >= (int)ptr->vertexActiveBlock_)
					current = -1;
				else if (ptr->vertices_[current].inEdge == GRAVEYARD_FLAG) { // Skip everything in the graveyard
					current++;
					continue;
				}
				break;
			}
		}

	public:
		inline explicit DigraphVertexIteratorAll(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { 
			findNextValid();
		}

		inline DigraphVertex operator()() {
			return DigraphVertex(ptr, current);
		}

		inline void next() {
			current++;
			findNextValid();
		}
	};

	class DigraphEdgeIteratorAll : public DigraphEdgeIterator {
	protected:
		inline void findNextValid() {
			while (true) {
				if (current >= (int)ptr->edgeActiveBlock_)
					current = -1;
				else if (ptr->edges_[current].prev == GRAVEYARD_FLAG) { // Skip everything in the graveyard
					current++;
					continue;
				}
				break;
			}
		}

	public:
		inline explicit DigraphEdgeIteratorAll(Digraph* ptr, int id) :
			DigraphEdgeIterator(ptr, id) { 

			findNextValid();
		}

		inline void next() {
			current++;
			findNextValid();
		}
	};

	inline DigraphVertex DigraphEdge::head() {
		return DigraphVertex(ptr, ptr->edges_[id_].head);
	}
	inline DigraphVertex DigraphEdge::tail() {
		return DigraphVertex(ptr, ptr->edges_[id_].tail);
	}
	inline int DigraphEdge::id() const {
		return id_;
	}
	inline DigraphEdgeIteratorF DigraphVertex::getOutgoingEdges() {
		return DigraphEdgeIteratorF(ptr, ptr->vertices_[id_].outEdge);
	}
	inline DigraphEdgeIteratorB DigraphVertex::getIngoingEdges() {
		return DigraphEdgeIteratorB(ptr, ptr->vertices_[id_].inEdge);
	}
	inline DigraphVertexIteratorB DigraphVertex::getIngoingNeighbors() {
		return DigraphVertexIteratorB(ptr, ptr->vertices_[id_].inEdge);
	}
	inline DigraphVertexIteratorF DigraphVertex::getOutgoingNeighbors() {
		return DigraphVertexIteratorF(ptr, ptr->vertices_[id_].outEdge);
	}
	inline DigraphEdgeIteratorAll Digraph::edges() {
		return DigraphEdgeIteratorAll(this, 0);
	}
	inline DigraphVertexIteratorAll Digraph::vertices() {
		return DigraphVertexIteratorAll(this, 0);
	}

	template <typename T>
	DigraphDataView<T> Digraph::createEdgeData(const std::string& name) {
		auto ptr = new DigraphEdgeData<T>(this, name);
		edgeDatas[name] = ptr;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	DigraphDataView<T> Digraph::createVertexData(const std::string& name) {
		auto ptr = new DigraphVertexData<T>(this, name);
		vertexDatas[name] = ptr;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	void Digraph::destroyData(DigraphDataView<T>& view) {
		switch (view.type()) {
		case DigraphDataType::VERTEX:
			vertexDatas.erase(view->name());
			break;
		case DigraphDataType::EDGE:
			edgeDatas.erase(view->name());
			break;
		}
		delete view.ptr;
	}

	template <typename T>
	inline T& DigraphVertex::data(const std::string& s) {
		return ptr->getVertexData<T>(s)[*this];
	}

	template <typename T>
	inline T& DigraphEdge::data(const std::string& s) {
		return ptr->getEdgeData<T>(s)[*this];
	}

	template <typename T>
	inline DigraphDataView<T> Digraph::getEdgeData(const std::string& name) {
		return DigraphDataView<T>(edgeDatas[name]->as<T>());
	}

	template <typename T>
	inline DigraphDataView<T> Digraph::getVertexData(const std::string& name) {
		return DigraphDataView<T>(vertexDatas[name]->as<T>());
	}

	inline uint32_t DigraphVertex::outDegree() {
		uint32_t result = 0u;
		for (auto it = getOutgoingEdges(); it.valid(); it.next())
			++result;
		return result;
	}
	inline uint32_t DigraphVertex::inDegree() {
		uint32_t result = 0u;
		for (auto it = getIngoingEdges(); it.valid(); it.next())
			++result;
		return result;
	}

	template <typename T>
	DigraphVertexLookupView<T> Digraph::createVertexLookup(const std::string& name) {
		auto lookup = new DigraphLookup<T>(this, DigraphLookupType::VERTEX);
		vertexLookups[name] = lookup;
		return DigraphVertexLookupView<T>(lookup);
	}

	template <typename T>
	DigraphEdgeLookupView<T> Digraph::createEdgeLookup(const std::string& name) {
		auto lookup = new DigraphLookup<T>(this, DigraphLookupType::EDGE);
		edgeLookups[name] = lookup;
		return DigraphEdgeLookupView<T>(lookup);
	}
}	

#endif