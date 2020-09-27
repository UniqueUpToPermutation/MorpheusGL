#pragma once

#define GRAVEYARD_FLAG -1000

#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <stack>
#include <assert.h>
#include <sstream>

#include <engine/mapcpy.hpp>

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
	template <typename T>
	struct DigraphSparseDataView;
	template <typename T>
	struct DigraphTwoWayVertexLookupView;
	template <typename T>
	struct DigraphTwoWayEdgeLookupView;

	struct DigraphVertexRaw {
		int mInEdge;
		int mOutEdge;
	};

	struct DigraphEdgeRaw {
		int mHead;
		int mTail;
		int mNext;
		int mPrev;
		int mDualNext;
		int mDualPrev;
	};

	class DigraphIterator {
	protected:
		Digraph* mPtr;
		int mCurrent;

	public:
		inline explicit DigraphIterator(Digraph* ptr, int current) :
			mPtr(ptr), mCurrent(current) { }

		// Returns whether or not this iterator is still valid.
		inline bool valid() const { return mCurrent != -1; }
		// Returns the id of the current object.
		inline int id() const { return mCurrent; }
	};

	
	// A vertex in a directed graph.
	class DigraphVertex {
	private:
		Digraph* mPtr;
		int mId;

	public:
		inline explicit DigraphVertex() : mPtr(nullptr), mId(-1) { }
		inline explicit DigraphVertex(Digraph* ptr, int id) :
			mPtr(ptr), mId(id) { }

		// Get an iterator over all outgoing edges of this vertex.
		// returns: An iterator over all outgoing edges. 
		inline DigraphEdgeIteratorF outgoing();
		
		// Get an iterator over all ingoing edges of this vertex.
		// returns: An iterator over all ingoing edges. 
		inline DigraphEdgeIteratorB incomming();
		
		// Get an iterator over all children of this vertex.
		// returns: An iterator over all children. 
		inline DigraphVertexIteratorF children();
		
		// Get an iterator over all parents of this vertex.
		// returns: An iterator over all parents. 
		inline DigraphVertexIteratorB parents();
		
		// Gets the number of children that this vertex has.
		// returns: Number of children. 
		inline uint32_t childCount();
		
		// Gets the number of parents that this vertex has.
		// returns: Number of parents. 
		inline uint32_t parentCount();
		
		// Get the id of this vertex.
		// returns:The vertex id. 
		inline int id() const { return mId; }
		
		// Get the out degree of this vertex.
		// returns:The out degree of this vertex. 
		inline uint32_t outDegree();
		
		// Get the in degree of this vertex.
		// returns:The in degree of this vertex. 
		inline uint32_t inDegree();
		
		// Whether or not this is a valid vertex.
		inline bool valid() const { return mId != -1; }
		
		// Returns the graph this vertex belongs to.
		inline Digraph* graph() { return mPtr; }
		
		// Adds an edge going from this vertex to v.
		// v: The head of the new edge.
		inline void addChild(const DigraphVertex& v);
		
		// Adds an edge going from v to this vertex.
		// v: The tail of the new edge.
		inline void addParent(const DigraphVertex& v);

		// Gets the i-th child of a node. Note that children are stored
		// in a linked list format, so this is not a good way to retrieve
		// children if the number of children is large.
		// i: The index of the child.
		// returns: The i-th child. 
		inline DigraphVertex getChild(const uint32_t i);

		
		// Gets the i-th parent of a node. Note that the parents are stored
		// in a linked list format, so this is not a good way to retrieve
		// parents if the number of parents is large.
		// i: The index of the parent.
		// returns: The i-th parent. 
		inline DigraphVertex getParent(const uint32_t i);

		// Query data associated with this vertex by name.
		// T: The data type to return.
		// s: The name of the data.
		// returns: Data associated with this vertex. 
		template <typename T>
		inline T& data(const std::string& s);
		
		// Create an invalid vertex.
		// returns: An invalid vertex. 
		static inline DigraphVertex invalid() {
			return DigraphVertex(nullptr, -1);
		}
	};

	class DigraphEdge {
	private:
		Digraph* mPtr;
		int mId;

	public:
		inline explicit DigraphEdge() : mPtr(nullptr), mId(-1) { }
		inline explicit DigraphEdge(Digraph* ptr, int id) :
			mPtr(ptr), mId(id) { }

		inline DigraphVertex head();
		inline DigraphVertex tail();
		inline int id() const;

		template <typename T>
		inline T& data(const std::string& s);
	};

	class IDigraphLookup;
	template <typename T>
	class DigraphLookup;

	// A directed graph class which features resizing memory and recycling of destroyed edges and vertices.
	class Digraph {
	private:
		DigraphVertexRaw* mVertices;
		DigraphEdgeRaw* mEdges;
		uint32_t mVertexCount;
		uint32_t mEdgeCount;
		uint32_t mVertexReserve;
		uint32_t mEdgeReserve;
		uint32_t mVertexActiveBlock;
		uint32_t mEdgeActiveBlock;
		uint32_t mDatasCreated;
		uint32_t mLookupsCreated;
		int mFirstUnusedEdge;
		int mFirstUnusedVertex;
		float mRescaleFactor;
		std::unordered_map<std::string, IDigraphData*> mVertexDatas;
		std::unordered_map<std::string, IDigraphData*> mEdgeDatas;
		std::unordered_map<std::string, IDigraphLookup*> mVertexLookups;
		std::unordered_map<std::string, IDigraphLookup*> mEdgeLookups;

		void resizeVertices(uint32_t newSize);
		void resizeEdges(uint32_t newSize);

	protected:

		virtual void applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize);
		virtual void applyEdgeMap(const int map[], const uint32_t mapLen, const uint32_t newSize);

	public:
		Digraph();
		~Digraph();
		Digraph(uint32_t reserveVertices, uint32_t reserveEdges);

		// Create a vertex in the graph.
		// returns: The vertex created. 
		DigraphVertex createVertex();
		
		// Create an edge in the graph.
		// tail: The numerical id of the tail vertex of the edge to create.
		// head :The numerical id of the head vertex of the edge to create.
		// returns: The edge created. 
		DigraphEdge createEdge(int tail, int head);
		
		// Create an edge in the graph.
		// tail: The tail vertex of the edge to create.
		// head: The head vertex of the edge to create.
		// returns: The create edge. 
		inline DigraphEdge createEdge(DigraphVertex& tail, DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		
		// Create an edge in the graph.
		// tail: The tail vertex of the edge to create.
		// head: The head vertex of the edge to create.
		// returns: The create edge. 
		inline DigraphEdge createEdge(const DigraphVertex& tail, const DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		
		// Deletes a vertex in the graph. Removes all ingoing and outgoing edges.
		// v: The vertex to delete.
		void deleteVertex(DigraphVertex v);
		
		// Deletes a vertex in the graph. Removes all ingoing and outgoing edges.
		// v: The numerical id of the vertex to delete.
		void deleteVertex(int v);
		
		// Deletes an edge in the graph.
		// e: The edge to delete.
		void deleteEdge(DigraphEdge e);
		
		// Attempts to reduce the amount of memory needed to store the graph.
		// bTight: Whether or not the amount of memory to use should be exactly the amount needed.
		void compress(bool bTight = false);

		// Returns the number of edges in the graph.
		uint32_t edgeCount() const { return mEdgeCount; }
		
		// Returns the number of vertices in the graph.
		uint32_t vertexCount() const { return mVertexCount; }

		// Returns the size of reserved memory for vertices.
		inline uint32_t vertexReserve() const { return mVertexReserve; }
		
		// Returns the size of reserved memory for edges.
		inline uint32_t edgeReserve() const { return mEdgeReserve; }
		
		// Returns the size of the memory block containing active vertices.
		inline uint32_t vertexActiveBlock() const { return mVertexActiveBlock; }
		
		// Returns the size of the memory block containing active edges.
		inline uint32_t edgeActiveBlock() const { return mEdgeActiveBlock; }

		// Get a vertex by id.
		// id: The id of the vertex to get.
		// returns: The vertex. 
		inline DigraphVertex getVertex(int id) { return DigraphVertex(this, id); }
		
		// Get an edge by id.
		// id: The id of the edge to get.
		// returns: The edge. 
		inline DigraphEdge getEdge(int id) { return DigraphEdge(this, id); }

		// Gets an iterator over all edges of the graph.
		inline DigraphEdgeIteratorAll edges();
		
		// Gets an iterator over all vertices of the graph.
		inline DigraphVertexIteratorAll vertices();

		// Create a new sparse data attachment of type T over the vertices.
		// T: The type of the data attachment.
		// returns: A view to the data attachment. 
		template <typename T>
		DigraphSparseDataView<T> createSparseVertexData(const T& default_);
		
		// Create a new sparse data attachment of type T over the edges.
		// T: The type of the data attachment.
		// returns: A view to the data attachment. 
		template <typename T>
		DigraphSparseDataView<T> createSparseEdgeData(const T& default_);
		
		// Create a new sparse data attachment of type T over the vertices.
		// name: The name of the data attachment.
		// T: The type of the data attachment.
		// returns: A view to the data attachment. 
		template <typename T>
		DigraphSparseDataView<T> createSparseVertexData(const T& default_, const std::string& name);
		
		// Create a new sparse data attachment of type T over the edges.
		// name: The name of the data attachment.
		// T: The type of the data attachment.
		// returns: A view to the data attachment. 
		template <typename T>
		DigraphSparseDataView<T> createSparseEdgeData(const T& default_, const std::string& name);

		// Create a new data attachment of type T over the vertices.
		// T: The type of the data attachment.
		// returns: A view to the created data. 
		template <typename T>
		DigraphDataView<T> createVertexData();

		// Create a new data attachment of type T over the edges.
		// T: The type of the data attachment.
		// returns: A view to the created data. 
		template <typename T>
		DigraphDataView<T> createEdgeData();

		// Create a new data attachment of type T over the edges.
		// T: The type of the data attachment.
		// name: The name of the data attachment.
		// returns: A view to the created data. 
		template <typename T>
		DigraphDataView<T> createEdgeData(const std::string& name);
		
		// Create a new data attachment of type T over the vertices.
		// T: The type of the data attachment.
		// name: The name of the data attachment.
		// returns: A view to the created data. 
		template <typename T>
		DigraphDataView<T> createVertexData(const std::string& name);

		// Creates a lookup attachment of type T over the vertices.
		// T: The type of the lookup attachment.
		// returns: A view to the created lookup. 
		template <typename T>
		DigraphVertexLookupView<T> createVertexLookup();

		// Creates a lookup attachment of type T over the edges.
		// T: The type of the lookup attachment.
		// returns: A view to the created lookup. 
		template <typename T>
		DigraphEdgeLookupView<T> createEdgeLookup();

		// Creates a lookup attachment of type T over the vertices.
		// T: The type of the lookup attachment.
		// name: The name of the lookup attachment.
		// returns: A view to the created lookup. 
		template <typename T>
		DigraphVertexLookupView<T> createVertexLookup(const std::string& name);

		// Creates a lookup attachment of type T over the edges.
		// T: The type of the lookup attachment.
		// name: The name of the lookup attachment.
		// returns: A view to the created lookup. 
		template <typename T>
		DigraphEdgeLookupView<T> createEdgeLookup(const std::string& name);

		template <typename T>
		DigraphTwoWayEdgeLookupView<T> createTwoWayEdgeLookup(const std::string& name);
		template <typename T>
		DigraphTwoWayVertexLookupView<T> createTwoWayVertexLookup(const std::string& name);
		template <typename T>
		DigraphTwoWayEdgeLookupView<T> createTwoWayEdgeLookup();
		template <typename T>
		DigraphTwoWayVertexLookupView<T> createTwoWayVertexLookup();

		// Destroys a data attachment from its view.
		// T: The type of the data attachment
		// view: The view to the data attachment to destroy
		template <typename T>
		void destroyData(DigraphDataView<T> view);

		// Destroys a data attachment from its view.
		// T: The type of the data attachment
		// view: The view to the data attachment to destroy
		template <typename T>
		void destroyData(DigraphSparseDataView<T> view);

		// Destroys the lookup associated with this view
		// T: The type of the lookup attachment.
		// view: The view to the lookup attachment to destroy.
		template <typename T>
		void destroyLookup(DigraphVertexLookupView<T> view);

		// Destroys the lookup associated with this view
		// T: The type of the lookup attachment.
		// view: The view to the lookup attachment to destroy.
		template <typename T>
		void destroyLookup(DigraphEdgeLookupView<T> view);

		template <typename T>
		void destroyLookup(DigraphTwoWayEdgeLookupView<T> view);

		template <typename T>
		void destroyLookup(DigraphTwoWayVertexLookupView<T> view);

		// Get an edge data attachment by name.
		// T: The type of the attachment.
		// name: The name of the attachment.
		// returns: A view to the data attachment. 
		template <typename T>
		inline DigraphDataView<T> getEdgeData(const std::string& name);

		// Get a vertex data attachment by name.
		// T: The type of the attachment.
		// name: The name of the attachment.
		// returns: A view to the data attachment. 
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
	template <typename T>
	class DigraphSparseData;

	// An interface to a graph data attachment.
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

	// A digraph data attachment by type.
	// T: The type of the attachment.
	template <typename T>
	class DigraphData : public IDigraphData {
	protected:
		Digraph* mParent;
		T* mData;
		uint32_t mDataSize;
		std::string mName;

		virtual DigraphDataType type() const {
			return DigraphDataType::UNKNOWN;
		}

		void resize(const uint32_t newSize) override {
			auto new_data = new T[newSize];
			memcpy(new_data, mData, sizeof(T) * mDataSize);
			delete[] mData;
			mData = new_data;
			mDataSize = newSize;
		}
		void compress(const int map[], const uint32_t mapSize, const uint32_t newSize) override {
			auto new_data = new T[newSize];
			mapcpy(new_data, mData, map, mapSize);
			delete[] mData;
			mData = new_data;
			mDataSize = newSize;
		}

	public:

		std::string name() const override {
			return mName;
		}

		explicit DigraphData(Digraph* parent, const std::string& name, const DigraphDataType type_) :
			mParent(parent), mName(name) {

			switch (type_) {
			case DigraphDataType::EDGE:
				mDataSize = parent->edgeReserve();
				break;
			case DigraphDataType::VERTEX:
				mDataSize = parent->vertexReserve();
				break;
			default:
				assert(false);
				mDataSize = parent->vertexReserve();
				break;
			}

			mData = new T[parent->vertexReserve()];
		}

		~DigraphData() override
		{
			delete[] mData;
		}

		friend struct DigraphDataView<T>;
	};

	// A digraph data attachment by type.
	// T: The type of the attachment.
	template <typename T>
	class DigraphSparseData : public IDigraphData {
	protected:
		Digraph* mParent;
		std::unordered_map<int, T> mMap;
		std::string mName;
		T mDefault;

		virtual DigraphDataType type() const {
			return DigraphDataType::UNKNOWN;
		}

		void resize(const uint32_t newSize) override {
		}
		void compress(const int map[], const uint32_t mapSize, const uint32_t newSize) override {

			std::unordered_map<int, T> newMap;

			for (auto& it : mMap)
				newMap[map[it.first]] = it.second;

			std::swap(mMap, newMap);
		}

	public:

		std::string name() const override {
			return mName;
		}

		explicit DigraphSparseData(Digraph* parent, const std::string& name, const T& default_) :
			mParent(parent), mName(name), mDefault(default_) {
		}

		friend struct DigraphSparseDataView<T>;
	};


	enum class DigraphLookupType {
		VERTEX,
		EDGE
	};

	// An interface to a digraph lookup attachment.
	class IDigraphLookup {
	public:
		virtual void applyMap(const int map[], const uint32_t mapSize) = 0;
	};

	// A digraph lookup attachment by type.
	// T: The type of the lookup.
	template <typename T>
	class DigraphLookup : public IDigraphLookup {
	private:
		Digraph* mParent;
		DigraphLookupType mType;
		std::unordered_map<T, int> mTtoId;
		std::string mName;

	protected:
		void applyMap(const int map[], const uint32_t mapSize) override {
			for (auto& item : mTtoId)
				item.second = map[item.second];
		}

	public:
		inline std::string name() const {
			return mName;
		}
		inline void setv(const DigraphVertex& v, const T& t) {
			mTtoId[t] = v.id();
		}
		inline void sete(const DigraphEdge& e, const T& t) {
			mTtoId[t] = e.id();
		}
		inline DigraphVertex getv(const T& t) {
			return mParent->getVertex(mTtoId[t]);
		}
		inline DigraphEdge gete(const T& t) {
			return mParent->getEdge(mTtoId[t]);
		}

		inline explicit DigraphLookup(Digraph* parent, 
			const DigraphLookupType type, const std::string& name) :
			mParent(parent), mType(type), mName(name) { }

		friend class Digraph;
		friend struct DigraphVertexLookupView<T>;
		friend struct DigraphEdgeLookupView<T>;
	};

	template <typename T>
	class DigraphTwoWayLookup : public IDigraphLookup {
	private:
		Digraph* mParent;
		DigraphLookupType mType;
		std::unordered_map<T, int> mTtoId;
		std::unordered_map<int, T> mIdToT;
		std::string mName;

	protected:
		void applyMap(const int map[], const uint32_t mapSize) override {
			mIdToT.clear();
			mIdToT.reserve(mTtoId.size());
			for (auto& item : mTtoId) {
				item.second = map[item.second];
				mIdToT[item.second] = item.first;
			}
		}

	public:
		inline std::string name() const {
			return mName;
		}
		inline void setv(const DigraphVertex& v, const T& t) {
			mTtoId[t] = v.id();
			mIdToT[v.id()] = t;
		}
		inline void sete(const DigraphEdge& e, const T& t) {
			mTtoId[t] = e.id();
			mIdToT[e.id()] = t;
		}
		inline DigraphVertex getv(const T& t) {
			return mParent->getVertex(mTtoId[t]);
		}
		inline DigraphEdge gete(const T& t) {
			return mParent->getEdge(mTtoId[t]);
		}
		inline T get(const DigraphVertex& v) {
			return mIdToT[v.id()];
		}
		inline T get(const DigraphEdge& e) {
			return mIdToT[e.id()];
		}
		inline void clear() {
			mTtoId.clear();
			mIdToT.clear();
		}
		inline void remove(const int i) {
			auto it = mIdToT.find(i);
			if (it != mIdToT.end()) {
				auto first = it->first;
				auto second = it->second;
				mIdToT.erase(it);

				auto otherIt = mTtoId.find(second);
				if (otherIt != mTtoId.end()) {
					if (otherIt->second == i)
						mTtoId.erase(otherIt);
				}
			}
		}
		inline void remove(const DigraphVertex& v) {
			remove(v.id());
		}
		inline void remove(const DigraphEdge& e) {
			remove(e.id());
		}
		inline void remove(const T& t) {
			auto it = mTtoId.find(t);
			if (it != mTtoId.end()) {
				auto first = it->first;
				auto second = it->second;
				mTtoId.erase(it);

				auto otherIt = mIdToT.find(second);
				if (otherIt != mIdToT.end()) {
					if (otherIt->second == t)
						mIdToT.erase(otherIt);
				}
			}
		}
		inline bool tryFind(const DigraphVertex& v, T* out) {
			auto it = mIdToT.find(v.id());
			if (it == mIdToT.end())
				return false;
			else {
				*out = it->second;
				return true;
			}
		}
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = mTtoId.find(t);
			if (it == mTtoId.end())
				return false;
			else {
				*out = DigraphVertex(mParent, it->second);
				return true;
			}
		}
		inline bool tryFind(const T& t, DigraphEdge* out) {
			auto it = mTtoId.find(t);
			if (it == mTtoId.end())
				return false;
			else {
				*out = DigraphEdge(mParent, it->second);
				return true;
			}
		}

		inline explicit DigraphTwoWayLookup(Digraph* parent,
			const DigraphLookupType type, const std::string& name) :
			mParent(parent), mType(type), mName(name) { }

		friend class Digraph;
		friend struct DigraphTwoWayVertexLookupView<T>;
		friend struct DigraphTwoWayEdgeLookupView<T>;
	};

	// A view to a lookup attachment on vertices of a graph.
	// T: The type of the lookup
	template <typename T>
	struct DigraphVertexLookupView {
	private:
		DigraphLookup<T>* mLookup;

	public:
		inline DigraphVertexLookupView(DigraphLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphVertexLookupView() : mLookup(nullptr) { }

		inline std::string name() {
			return mLookup->mName;
		}

		inline DigraphVertex operator[](const T& t) {
			return mLookup->mParent->getVertex(mLookup->mTtoId[t]);
		}
		
		// Assigns a vertex to a value in this lookup.
		// v: The vertex to assign to this value.
		// t: The value.
		inline void set(const DigraphVertex& v, const T& t) {
			mLookup->mTtoId[t] = v.id();
		}
		
		// Clear this value in the lookup.
		// t: The value to clear.
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
		
		// Try to find this value in the lookup.
		// t: The value to find.
		// out: The vertex found.
		// returns: Whether or not a vertex was found. 
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = mLookup->mTtoId.find(t);
			if (it == mLookup->mTtoId.end())
				return false;
			else {
				*out = mLookup->mParent->getVertex(it->second);
				return true;
			}
		}

		friend class Digraph;
	};

	// A view to a lookup attachment on edges of a graph.
	template <typename T>
	struct DigraphEdgeLookupView {
	private:
		DigraphLookup<T>* mLookup;

	public:
		inline DigraphEdgeLookupView(DigraphLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphEdgeLookupView() : mLookup(nullptr) { }

		inline std::string name() {
			return mLookup->mName;
		}

		inline DigraphEdge operator[](const T& t) {
			return mLookup->mParent->getEdge(mLookup->mTtoId[t]);
		}
		
		// Assigns a vertex to a value in this lookup.
		// v: The vertex to assign to this value.
		// t: The value.
		inline void set(const DigraphEdge& e, const T& t) {
			mLookup->mTtoId[t] = e.id();
		}
		
		// Clear this value in the lookup.
		// t: The value to clear.
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
		
		// Try to find this value in the lookup.
		// t: The value to find.
		// out: The vertex found.
		// returns: Whether or not a vertex was found. 
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = mLookup->mTtoId.find(t);
			if (it == mLookup->mTtoId.end())
				return false;
			else {
				*out = mLookup->mParent->getVertex(it->second);
				return true;
			}
		}

		friend class Digraph;
	};

	template <typename T>
	struct DigraphTwoWayVertexLookupView {
	private:
		DigraphTwoWayLookup<T>* mLookup;

	public:
		inline DigraphTwoWayVertexLookupView(DigraphTwoWayLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphTwoWayVertexLookupView() : mLookup(nullptr) { }

		inline std::string name() {
			return mLookup->mName;
		}

		inline DigraphVertex operator[](const T& t) {
			return mLookup->mParent->getVertex(mLookup->mTtoId[t]);
		}

		inline T operator[](const DigraphVertex& v) {
			return mLookup->get(v);
		}

		inline void set(const DigraphVertex& v, const T& t) {
			mLookup->setv(v, t);
		}

		inline void remove(const T& t) {
			mLookup->remove(t);
		}

		inline void remove(const DigraphVertex& v) {
			mLookup->remove(v);
		}

		inline void clear() {
			mLookup->clear();
		}

		inline bool tryFind(const T& t, DigraphVertex* out) {
			return mLookup->tryFind(t, out);
		}

		inline bool tryFind(const DigraphVertex& v, T* out) {
			return mLookup->tryFind(v, out);
		}

		friend class Digraph;
	};

	template <typename T>
	struct DigraphTwoWayEdgeLookupView {
	private:
		DigraphTwoWayLookup<T>* mLookup;

	public:
		inline DigraphTwoWayEdgeLookupView(DigraphTwoWayLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphTwoWayEdgeLookupView() : mLookup(nullptr) { }

		inline std::string name() {
			return mLookup->mName;
		}

		inline DigraphEdge operator[](const T& t) {
			return mLookup->mParent->getVertex(mLookup->mTtoId[t]);
		}

		inline T operator[](const DigraphEdge& v) {
			return mLookup->get(v);
		}

		inline void set(const DigraphEdge& v, const T& t) {
			mLookup->setv(v, t);
		}

		inline void remove(const T& t) {
			mLookup->remove(t);
		}

		inline void remove(const DigraphEdge& v) {
			mLookup->remove(v);
		}

		inline void clear() {
			mLookup->clear();
		}

		inline bool tryFind(const T& t, DigraphEdge* out) {
			return mLookup->tryFind(t, out);
		}

		inline bool tryFind(const DigraphEdge& v, T* out) {
			return mLookup->tryFind(v, out);
		}

		friend class Digraph;
	};

	template <typename T>
	class DigraphEdgeData : public DigraphData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::EDGE;
		}

	public:
		explicit inline DigraphEdgeData(Digraph* parent, const std::string& name) :
			DigraphData<T>(parent, name, DigraphDataType::EDGE) { }
	};

	template <typename T>
	class DigraphVertexData : public DigraphData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::VERTEX;
		}

	public:
		explicit inline DigraphVertexData(Digraph* parent, const std::string& name) :
			DigraphData<T>(parent, name, DigraphDataType::VERTEX) { }
	};

	template <typename T>
	class DigraphSparseVertexData : public DigraphSparseData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::VERTEX;
		}

	public:
		explicit inline DigraphSparseVertexData(Digraph* parent, const std::string& name, const T& default_) :
			DigraphSparseData<T>(parent, name, default_) { }
	};

	template <typename T>
	class DigraphSparseEdgeData : public DigraphSparseData<T> {
	protected:
		DigraphDataType type() const override {
			return DigraphDataType::EDGE;
		}

	public:
		explicit inline DigraphSparseEdgeData(Digraph* parent, const std::string& name, const T& default_) :
			DigraphSparseData<T>(parent, name, default_) { }
	};

	
	// A view to a data attachment of a directed graph.
	// T: The type of the data.
	template <typename T>
	struct DigraphDataView {
	private:
		DigraphData<T>* mPtr;

	public:
		explicit inline DigraphDataView(DigraphData<T>* ptr) : mPtr(ptr) { }
		explicit inline DigraphDataView() : mPtr(nullptr) { }

		inline void memset(const T& val) {
			for (uint32_t i = 0; i < mPtr->mDataSize; ++i)
				mPtr->mData[i] = val;
		}
		inline DigraphDataType type() const { return mPtr->type(); }
		inline T& operator[](const int id) { return mPtr->mData[id]; }
		inline T& operator[](const DigraphVertex& v) { return mPtr->mData[v.id()]; }
		inline T& operator[](const DigraphEdge& e) { return mPtr->mData[e.id()]; }
		inline std::string name() { return mPtr->name(); }

		friend class Digraph;
	};

	// A view to a sparse data attachment of a directed graph.
	// T: The type of the data.
	template <typename T>
	struct DigraphSparseDataView {
	private:
		DigraphSparseData<T>* mPtr;

	public:
		explicit inline DigraphSparseDataView(DigraphSparseData<T>* ptr) : mPtr(ptr) { }
		explicit inline DigraphSparseDataView() : mPtr(nullptr) { }

		inline void memset(const T& val) {
			mPtr->mDefault = val;
			mPtr->mMap.clear();
		}
		inline DigraphDataType type() const { return mPtr->type(); }
		inline void set(const DigraphVertex& v, const T& val) {
			mPtr->mMap[v.id()] = val;
		}
		inline T get(const DigraphVertex& v) {
			auto it = mPtr->mMap.find(v.id());
			if (it == mPtr->mMap.end())
				return mPtr->mDefault;
			else
				return it->second;
		}
		inline std::string name() const { return mPtr->name(); }

		friend class Digraph;
	};

	class DigraphVertexIteratorF : public DigraphIterator {
	public:
		inline explicit DigraphVertexIteratorF(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphVertex operator()() {
			return DigraphVertex(mPtr, mPtr->mEdges[mCurrent].mHead);
		}
		inline void next() {
			mCurrent = mPtr->mEdges[mCurrent].mNext;
		}
	};

	class DigraphVertexIteratorB : public DigraphIterator {
	public:
		inline explicit DigraphVertexIteratorB(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphVertex operator()() {
			return DigraphVertex(mPtr, mPtr->mEdges[mCurrent].mTail);
		}
		inline void next() {
			mCurrent = mPtr->mEdges[mCurrent].mDualNext;
		}
	};

	class DigraphEdgeIterator : public DigraphIterator {
	public:
		inline explicit DigraphEdgeIterator(Digraph* ptr, int id) :
			DigraphIterator(ptr, id) { }

		inline DigraphEdge operator()() {
			return DigraphEdge(mPtr, mCurrent);
		}
	};

	class DigraphEdgeIteratorF : public DigraphEdgeIterator {
	public:
		inline explicit DigraphEdgeIteratorF(Digraph* ptr, int id) :
			DigraphEdgeIterator(ptr, id) { }

		inline void next() {
			mCurrent = mPtr->mEdges[mCurrent].mNext;
		}
	};

	class DigraphEdgeIteratorB : public DigraphEdgeIterator {
	public:
		inline explicit DigraphEdgeIteratorB(Digraph* ptr, int id) :
			DigraphEdgeIterator(ptr, id) { }

		inline void next() {
			mCurrent = mPtr->mEdges[mCurrent].mDualNext;
		}
	};

	class DigraphVertexIteratorAll : public DigraphIterator {
	protected:
		inline void findNextValid() {
			while (true) {
				if (mCurrent >= (int)mPtr->mVertexActiveBlock)
					mCurrent = -1;
				else if (mPtr->mVertices[mCurrent].mInEdge == GRAVEYARD_FLAG) { // Skip everything in the graveyard
					mCurrent++;
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
			return DigraphVertex(mPtr, mCurrent);
		}

		inline void next() {
			mCurrent++;
			findNextValid();
		}
	};

	class DigraphEdgeIteratorAll : public DigraphEdgeIterator {
	protected:
		inline void findNextValid() {
			while (true) {
				if (mCurrent >= (int)mPtr->mEdgeActiveBlock)
					mCurrent = -1;
				else if (mPtr->mEdges[mCurrent].mPrev == GRAVEYARD_FLAG) { // Skip everything in the graveyard
					mCurrent++;
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
			mCurrent++;
			findNextValid();
		}
	};

	// An iterator for a depth first search of a digraph starting at a given node.
	class DigraphDepthFirstSearch {
	private:
		std::stack<int> mVertexStack;
		DigraphDataView<bool> mVisited;
		Digraph* digraph;

	public:
		DigraphDepthFirstSearch(DigraphVertex& start);
		DigraphDepthFirstSearch();
		~DigraphDepthFirstSearch();

		void restart(DigraphVertex& start);

		inline bool valid() {
			return !mVertexStack.empty();
		}

		inline DigraphVertex next() {
			DigraphVertex v(digraph, mVertexStack.top());
			mVertexStack.pop();
			for (auto it = v.children(); it.valid(); it.next()) {
				auto neighbor = it();
				if (!mVisited[neighbor])
					mVertexStack.push(neighbor.id());
			}
			return v;
		}
	};

	// An iterator for a breadth first search of a digraph starting at a given node.
	class DigraphBreadthFirstSearch {
	private:
		std::queue<int> mVertexQueue;
		DigraphDataView<bool> mVisited;
		Digraph* digraph;

	public:
		DigraphBreadthFirstSearch(DigraphVertex& start);
		DigraphBreadthFirstSearch();
		~DigraphBreadthFirstSearch();
		void restart(DigraphVertex& start);

		inline bool valid() {
			return !mVertexQueue.empty();
		}

		inline DigraphVertex next() {
			DigraphVertex v(digraph, mVertexQueue.front());
			mVertexQueue.pop();
			for (auto it = v.children(); it.valid(); it.next()) {
				auto neighbor = it();
				if (!mVisited[neighbor])
					mVertexQueue.push(neighbor.id());
			}
			return v;
		}
	};

	inline DigraphVertex DigraphEdge::head() {
		return DigraphVertex(mPtr, mPtr->mEdges[mId].mHead);
	}
	inline DigraphVertex DigraphEdge::tail() {
		return DigraphVertex(mPtr, mPtr->mEdges[mId].mTail);
	}
	inline int DigraphEdge::id() const {
		return mId;
	}
	inline DigraphEdgeIteratorF DigraphVertex::outgoing() {
		return DigraphEdgeIteratorF(mPtr, mPtr->mVertices[mId].mOutEdge);
	}
	inline DigraphEdgeIteratorB DigraphVertex::incomming() {
		return DigraphEdgeIteratorB(mPtr, mPtr->mVertices[mId].mInEdge);
	}
	inline DigraphVertexIteratorB DigraphVertex::parents() {
		return DigraphVertexIteratorB(mPtr, mPtr->mVertices[mId].mInEdge);
	}
	inline DigraphVertexIteratorF DigraphVertex::children() {
		return DigraphVertexIteratorF(mPtr, mPtr->mVertices[mId].mOutEdge);
	}
	inline uint32_t DigraphVertex::childCount() {
		uint32_t count = 0;
		for (auto it = children(); it.valid(); it.next())
			++count;
		return count;
	}
	inline uint32_t DigraphVertex::parentCount() {
		uint32_t count = 0;
		for (auto it = parents(); it.valid(); it.next())
			++count;
		return count;
	}
	inline DigraphEdgeIteratorAll Digraph::edges() {
		return DigraphEdgeIteratorAll(this, 0);
	}
	inline DigraphVertexIteratorAll Digraph::vertices() {
		return DigraphVertexIteratorAll(this, 0);
	}

	template <typename T>
	DigraphDataView<T> Digraph::createVertexData() {
		std::stringstream s;
		s << "__unamed__";
		s << mDatasCreated;
		s << "__";
		return createVertexData<T>(s.str());
	}

	template <typename T>
	DigraphDataView<T> Digraph::createEdgeData() {
		std::stringstream s;
		s << "__unamed__";
		s << mDatasCreated;
		s << "__";
		return createEdgeData<T>(s.str());
	}


	template <typename T>
	DigraphDataView<T> Digraph::createEdgeData(const std::string& name) {
		auto ptr = new DigraphEdgeData<T>(this, name);
		mEdgeDatas[name] = ptr;
		++mDatasCreated;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	DigraphDataView<T> Digraph::createVertexData(const std::string& name) {
		auto ptr = new DigraphVertexData<T>(this, name);
		mVertexDatas[name] = ptr;
		++mDatasCreated;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	void Digraph::destroyData(DigraphDataView<T> view) {
		switch (view.type()) {
		case DigraphDataType::VERTEX:
			mVertexDatas.erase(view.name());
			break;
		case DigraphDataType::EDGE:
			mEdgeDatas.erase(view.name());
			break;
		default:
			break;
		}
		delete view.mPtr;
	}

	template <typename T>
	void Digraph::destroyData(DigraphSparseDataView<T> view) {
		switch (view.type()) {
		case DigraphDataType::VERTEX:
			mVertexDatas.erase(view.name());
			break;
		case DigraphDataType::EDGE:
			mEdgeDatas.erase(view.name());
			break;
		}
		delete view.mPtr;
	}

	template <typename T>
	void Digraph::destroyLookup(DigraphVertexLookupView<T> view) {
		mVertexLookups.erase(view.name());
		delete view.mLookup;
	}

	template <typename T>
	void Digraph::destroyLookup(DigraphEdgeLookupView<T> view) {
		mEdgeLookups.erase(view.name());
		delete view.mLookup;
	}

	template <typename T>
	void Digraph::destroyLookup(DigraphTwoWayEdgeLookupView<T> view) {
		mEdgeLookups.erase(view.name());
		delete view.mLookup;
	}

	template <typename T>
	void Digraph::destroyLookup(DigraphTwoWayVertexLookupView<T> view) {
		mVertexLookups.erase(view.name());
		delete view.mLookup;
	}


	template <typename T>
	inline T& DigraphVertex::data(const std::string& s) {
		return mPtr->getVertexData<T>(s)[*this];
	}

	template <typename T>
	inline T& DigraphEdge::data(const std::string& s) {
		return mPtr->getEdgeData<T>(s)[*this];
	}

	template <typename T>
	inline DigraphDataView<T> Digraph::getEdgeData(const std::string& name) {
		return DigraphDataView<T>(mEdgeDatas[name]->as<T>());
	}

	template <typename T>
	inline DigraphDataView<T> Digraph::getVertexData(const std::string& name) {
		return DigraphDataView<T>(mVertexDatas[name]->as<T>());
	}

	inline uint32_t DigraphVertex::outDegree() {
		uint32_t result = 0u;
		for (auto it = outgoing(); it.valid(); it.next())
			++result;
		return result;
	}
	inline uint32_t DigraphVertex::inDegree() {
		uint32_t result = 0u;
		for (auto it = incomming(); it.valid(); it.next())
			++result;
		return result;
	}

	inline void DigraphVertex::addChild(const DigraphVertex& v) {
		mPtr->createEdge(*this, v);
	}

	inline void DigraphVertex::addParent(const DigraphVertex& v) {
		mPtr->createEdge(v, *this);
	}

	inline DigraphVertex DigraphVertex::getChild(const uint32_t i) {
		auto it = children();
		for (uint32_t current = 0; it.valid() && current < i; it.next(), ++current);
		return it();
	}

	inline DigraphVertex DigraphVertex::getParent(const uint32_t i) {
		auto it = parents();
		for (uint32_t current = 0; it.valid() && current < i; it.next(), ++current);
		return it();
	}

	template <typename T>
	DigraphVertexLookupView<T> Digraph::createVertexLookup(const std::string& name) {
		auto lookup = new DigraphLookup<T>(this, DigraphLookupType::VERTEX, name);
		mVertexLookups[name] = lookup;
		++mLookupsCreated;
		return DigraphVertexLookupView<T>(lookup);
	}

	template <typename T>
	DigraphEdgeLookupView<T> Digraph::createEdgeLookup(const std::string& name) {
		auto lookup = new DigraphLookup<T>(this, DigraphLookupType::EDGE, name);
		mEdgeLookups[name] = lookup;
		++mLookupsCreated;
		return DigraphEdgeLookupView<T>(lookup);
	}

	template <typename T>
	DigraphTwoWayEdgeLookupView<T> Digraph::createTwoWayEdgeLookup(const std::string& name) {
		auto lookup = new DigraphTwoWayLookup<T>(this, DigraphLookupType::EDGE, name);
		mEdgeLookups[name] = lookup;
		++mLookupsCreated;
		return DigraphTwoWayEdgeLookupView<T>(lookup);
	}
	template <typename T>
	DigraphTwoWayVertexLookupView<T> Digraph::createTwoWayVertexLookup(const std::string& name) {
		auto lookup = new DigraphTwoWayLookup<T>(this, DigraphLookupType::VERTEX, name);
		mVertexLookups[name] = lookup;
		++mLookupsCreated;
		return DigraphTwoWayVertexLookupView<T>(lookup);
	}
	template <typename T>
	DigraphTwoWayEdgeLookupView<T> Digraph::createTwoWayEdgeLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createTwoWayEdgeLookup<T>(s.str());
	}
	template <typename T>
	DigraphTwoWayVertexLookupView<T> Digraph::createTwoWayVertexLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createTwoWayEdgeLookup<T>(s.str());
	}


	template <typename T>
	DigraphVertexLookupView<T> Digraph::createVertexLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createVertexLookup<T>(s.str());
	}

	template <typename T>
	DigraphEdgeLookupView<T> Digraph::createEdgeLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createEdgeLookup<T>(s.str());
	}

	template <typename T>
	DigraphSparseDataView<T> Digraph::createSparseVertexData(const T& default_) {
		std::stringstream s;
		s << "__unamed__";
		s << mDatasCreated;
		s << "__";
		return createSparseVertexData<T>(default_, s.str());
	}

	template <typename T>
	DigraphSparseDataView<T> Digraph::createSparseEdgeData(const T& default_) {
		std::stringstream s;
		s << "__unamed__";
		s << mDatasCreated;
		s << "__";
		return createSparseEdgeData<T>(default_, s.str());
	}
	
	template <typename T>
	DigraphSparseDataView<T> Digraph::createSparseVertexData(const T& default_, const std::string& name) {
		DigraphSparseVertexData<T>* dat = new DigraphSparseVertexData<T>(this, name, default_);
		mVertexDatas[name] = dat;
		++mDatasCreated;
		return DigraphSparseDataView<T>(dat);
	}

	template <typename T>
	DigraphSparseDataView<T> Digraph::createSparseEdgeData(const T& default_, const std::string& name) {
		DigraphSparseEdgeData<T>* dat = new DigraphSparseEdgeData<T>(this, name, default_);
		mEdgeDatas[name] = dat;
		++mDatasCreated;
		return DigraphSparseDataView<T>(dat);
	}
}	