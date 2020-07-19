#ifndef DI_GRAPH_H_
#define DI_GRAPH_H_

#define GRAVEYARD_FLAG -1000

#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <stack>
#include <assert.h>
#include <sstream>

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
	template <typename T>
	struct DigraphSparseDataView;

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

		/// <summary>
		/// Returns whether or not this iterator is still valid.
		/// </summary>
		/// <returns></returns>
		inline bool valid() { return mCurrent != -1; }
		inline int id() const { return mCurrent; }
	};

	/// <summary>
	/// A vertex in a directed graph.
	/// </summary>
	class DigraphVertex {
	private:
		Digraph* mPtr;
		int mId;

	public:
		inline explicit DigraphVertex() : mPtr(nullptr), mId(-1) { }
		inline explicit DigraphVertex(Digraph* ptr, int id) :
			mPtr(ptr), mId(id) { }

		/// <summary>
		/// Get an iterator over all outgoing edges of this vertex.
		/// </summary>
		/// <returns>An iterator over all outgoing edges.</returns>
		inline DigraphEdgeIteratorF outgoing();
		/// <summary>
		/// Get an iterator over all ingoing edges of this vertex.
		/// </summary>
		/// <returns>An iterator over all ingoing edges.</returns>
		inline DigraphEdgeIteratorB incomming();
		/// <summary>
		/// Get an iterator over all children of this vertex.
		/// </summary>
		/// <returns>An iterator over all children.</returns>
		inline DigraphVertexIteratorF children();
		/// <summary>
		/// Get an iterator over all parents of this vertex.
		/// </summary>
		/// <returns>An iterator over all parents.</returns>
		inline DigraphVertexIteratorB parents();
		/// <summary>
		/// Gets the number of children that this vertex has.
		/// </summary>
		/// <returns>Number of children.</returns>
		inline uint32_t childCount();
		/// <summary>
		/// Gets the number of parents that this vertex has.
		/// </summary>
		/// <returns>Number of parents.</returns>
		inline uint32_t parentCount();
		/// <summary>
		/// Get the id of this vertex.
		/// </summary>
		/// <returns>The vertex id.</returns>
		inline int id() const { return mId; }
		/// <summary>
		/// Get the out degree of this vertex.
		/// </summary>
		/// <returns>The out degree of this vertex.</returns>
		inline uint32_t outDegree();
		/// <summary>
		/// Get the in degree of this vertex.
		/// </summary>
		/// <returns>The in degree of this vertex.</returns>
		inline uint32_t inDegree();
		/// <summary>
		/// Whether or not this is a valid vertex.
		/// </summary>
		/// <returns></returns>
		inline bool isValid() const { return mId != -1; }
		/// <summary>
		/// Returns the graph this vertex belongs to.
		/// </summary>
		/// <returns>The graph this vertex belongs to.</returns>
		inline Digraph* graph() { return mPtr; }
		/// <summary>
		/// Adds an edge going from this vertex to v.
		/// </summary>
		/// <param name="v">The head of the new edge.</param>
		inline void addChild(const DigraphVertex& v);
		/// <summary>
		/// Adds an edge going from v to this vertex.
		/// </summary>
		/// <param name="v">The tail of the new edge.</param>
		inline void addParent(const DigraphVertex& v);
		/// <summary>
		/// Query data associated with this vertex by name.
		/// </summary>
		/// <typeparam name="T">The data type to return.</typeparam>
		/// <param name="s">The name of the data.</param>
		/// <returns>Data associated with this vertex.</returns>
		template <typename T>
		inline T& data(const std::string& s);

		/// <summary>
		/// Create an invalid vertex.
		/// </summary>
		/// <returns>An invalid vertex.</returns>
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

	/// <summary>
	/// A directed graph class which features resizing memory and recycling of destroyed edges and vertices.
	/// </summary>
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

		void applyVertexMap(const int map[], const uint32_t mapLen, const uint32_t newSize);
		void applyEdgeMap(const int map[], const uint32_t mapLen, const uint32_t newSize);

	public:
		Digraph();
		~Digraph();
		Digraph(uint32_t reserveVertices, uint32_t reserveEdges);

		/// <summary>
		/// Create a vertex in the graph.
		/// </summary>
		/// <returns>The vertex created.</returns>
		DigraphVertex createVertex();
		/// <summary>
		/// Create an edge in the graph.
		/// </summary>
		/// <param name="tail">The numerical id of the tail vertex of the edge to create.</param>
		/// <param name="head">The numerical id of the head vertex of the edge to create.</param>
		/// <returns>The edge created.</returns>
		DigraphEdge createEdge(int tail, int head);
		/// <summary>
		/// Create an edge in the graph.
		/// </summary>
		/// <param name="tail">The tail vertex of the edge to create.</param>
		/// <param name="head">The head vertex of the edge to create.</param>
		/// <returns>The create edge.</returns>
		inline DigraphEdge createEdge(DigraphVertex& tail, DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		/// <summary>
		/// Create an edge in the graph.
		/// </summary>
		/// <param name="tail">The tail vertex of the edge to create.</param>
		/// <param name="head">The head vertex of the edge to create.</param>
		/// <returns>The create edge.</returns>
		inline DigraphEdge createEdge(const DigraphVertex& tail, const DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		/// <summary>
		/// Deletes a vertex in the graph. Removes all ingoing and outgoing edges.
		/// </summary>
		/// <param name="v">The vertex to delete.</param>
		void deleteVertex(DigraphVertex& v);
		/// <summary>
		/// Deletes a vertex in the graph. Removes all ingoing and outgoing edges.
		/// </summary>
		/// <param name="v">The numerical id of the vertex to delete.</param>
		void deleteVertex(int v);
		/// <summary>
		/// Deletes an edge in the graph.
		/// </summary>
		/// <param name="e">The edge to delete.</param>
		void deleteEdge(DigraphEdge& e);
		/// <summary>
		/// Attempts to reduce the amount of memory needed to store the graph.
		/// </summary>
		/// <param name="bTight">Whether or not the amount of memory to use should be exactly the amount needed.</param>
		void compress(bool bTight = false);

		/// <summary>
		/// Returns the number of edges in the graph.
		/// </summary>
		/// <returns>The number of edges in the graph.</returns>
		uint32_t edgeCount() const { return mEdgeCount; }
		/// <summary>
		/// Returns the number of vertices in the graph.
		/// </summary>
		/// <returns>The number of vertices in the graph.</returns>
		uint32_t vertexCount() const { return mVertexCount; }

		/// <summary>
		/// Returns the size of reserved memory for vertices.
		/// </summary>
		/// <returns>The size of the vertex reserve memory.</returns>
		inline uint32_t vertexReserve() const { return mVertexReserve; }
		/// <summary>
		/// Returns the size of reserved memory for edges.
		/// </summary>
		/// <returns>The size of edge reserve memory.</returns>
		inline uint32_t edgeReserve() const { return mEdgeReserve; }
		/// <summary>
		/// Returns the size of the memory block containing active vertices.
		/// </summary>
		/// <returns>The size of the memory block containing active vertices.</returns>
		inline uint32_t vertexActiveBlock() const { return mVertexActiveBlock; }
		/// <summary>
		/// Returns the size of the memory block containing active edges.
		/// </summary>
		/// <returns>The size of the memory block containing active edges.</returns>
		inline uint32_t edgeActiveBlock() const { return mEdgeActiveBlock; }

		/// <summary>
		/// Get a vertex by id.
		/// </summary>
		/// <param name="id">The id of the vertex to get.</param>
		/// <returns>The vertex.</returns>
		inline DigraphVertex getVertex(int id) { return DigraphVertex(this, id); }
		/// <summary>
		/// Get an edge by id.
		/// </summary>
		/// <param name="id">The id of the edge to get.</param>
		/// <returns>The edge.</returns>
		inline DigraphEdge getEdge(int id) { return DigraphEdge(this, id); }

		/// <summary>
		/// Gets an iterator over all edges of the graph.
		/// </summary>
		/// <returns>An iterator over all edges of the graph.</returns>
		inline DigraphEdgeIteratorAll edges();
		/// <summary>
		/// Gets an iterator over all vertices of the graph.
		/// </summary>
		/// <returns>An iterator over all vertices of the graph.</returns>
		inline DigraphVertexIteratorAll vertices();

		/// <summary>
		/// Create a new sparse data attachment of type T over the vertices.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the data attachment.</returns>
		template <typename T>
		DigraphSparseDataView<T> createSparseVertexData(const T& default_);
		/// <summary>
		/// Create a new sparse data attachment of type T over the edges.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the data attachment.</returns>
		template <typename T>
		DigraphSparseDataView<T> createSparseEdgeData(const T& default_);
		/// <summary>
		/// Create a new sparse data attachment of type T over the vertices.
		/// </summary>
		/// <param name="name">The name of the data attachment.</param>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the data attachment.</returns>
		template <typename T>
		DigraphSparseDataView<T> createSparseVertexData(const T& default_, const std::string& name);
		/// <summary>
		/// Create a new sparse data attachment of type T over the edges.
		/// </summary>
		/// <param name="name">The name of the data attachment.</param>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the data attachment.</returns>
		template <typename T>
		DigraphSparseDataView<T> createSparseEdgeData(const T& default_, const std::string& name);

		/// <summary>
		/// Create a new data attachment of type T over the vertices.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the created data.</returns>
		template <typename T>
		DigraphDataView<T> createVertexData();

		/// <summary>
		/// Create a new data attachment of type T over the edges.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <returns>A view to the created data.</returns>
		template <typename T>
		DigraphDataView<T> createEdgeData();

		/// <summary>
		/// Create a new data attachment of type T over the edges.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <param name="name">The name of the data attachment.</param>
		/// <returns>A view to the created data.</returns>
		template <typename T>
		DigraphDataView<T> createEdgeData(const std::string& name);
		
		/// <summary>
		/// Create a new data attachment of type T over the vertices.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment.</typeparam>
		/// <param name="name">The name of the data attachment.</param>
		/// <returns>A view to the created data.</returns>
		template <typename T>
		DigraphDataView<T> createVertexData(const std::string& name);

		/// <summary>
		/// Creates a lookup attachment of type T over the vertices.
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <returns>A view to the created lookup.</returns>
		template <typename T>
		DigraphVertexLookupView<T> createVertexLookup();

		/// <summary>
		/// Creates a lookup attachment of type T over the edges.
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <returns>A view to the created lookup.</returns>
		template <typename T>
		DigraphEdgeLookupView<T> createEdgeLookup();

		/// <summary>
		/// Creates a lookup attachment of type T over the vertices.
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <param name="name">The name of the lookup attachment.</param>
		/// <returns>A view to the created lookup.</returns>
		template <typename T>
		DigraphVertexLookupView<T> createVertexLookup(const std::string& name);

		/// <summary>
		/// Creates a lookup attachment of type T over the edges.
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <param name="name">The name of the lookup attachment.</param>
		/// <returns>A view to the created lookup.</returns>
		template <typename T>
		DigraphEdgeLookupView<T> createEdgeLookup(const std::string& name);

		/// <summary>
		/// Destroys a data attachment from its view.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment</typeparam>
		/// <param name="view">The view to the data attachment to destroy</param>
		template <typename T>
		void destroyData(DigraphDataView<T>& view);

		/// <summary>
		/// Destroys a data attachment from its view.
		/// </summary>
		/// <typeparam name="T">The type of the data attachment</typeparam>
		/// <param name="view">The view to the data attachment to destroy</param>
		template <typename T>
		void destroyData(DigraphSparseDataView<T>& view);

		/// <summary>
		/// Destroys the lookup associated with this view
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <param name="view">The view to the lookup attachment to destroy.</param>
		template <typename T>
		void destroyLookup(DigraphVertexLookupView<T>& view);

		/// <summary>
		/// Destroys the lookup associated with this view
		/// </summary>
		/// <typeparam name="T">The type of the lookup attachment.</typeparam>
		/// <param name="view">The view to the lookup attachment to destroy.</param>
		template <typename T>
		void destroyLookup(DigraphEdgeLookupView<T>& view);

		/// <summary>
		/// Get an edge data attachment by name.
		/// </summary>
		/// <typeparam name="T">The type of the attachment.</typeparam>
		/// <param name="name">The name of the attachment.</param>
		/// <returns>A view to the data attachment.</returns>
		template <typename T>
		inline DigraphDataView<T> getEdgeData(const std::string& name);

		/// <summary>
		/// Get a vertex data attachment by name.
		/// </summary>
		/// <typeparam name="T">The type of the attachment.</typeparam>
		/// <param name="name">The name of the attachment.</param>
		/// <returns>A view to the data attachment.</returns>
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

	/// <summary>
	/// An interface to a graph data attachment.
	/// </summary>
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

	/// <summary>
	/// A digraph data attachment by type.
	/// </summary>
	/// <typeparam name="T">The type of the attachment.</typeparam>
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


	/// <summary>
	/// A digraph data attachment by type.
	/// </summary>
	/// <typeparam name="T">The type of the attachment.</typeparam>
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

	/// <summary>
	/// An interface to a digraph lookup attachment.
	/// </summary>
	class IDigraphLookup {
	public:
		virtual void applyMap(const int map[], const uint32_t mapSize) = 0;
	};

	/// <summary>
	/// A digraph lookup attachment by type.
	/// </summary>
	/// <typeparam name="T">The type of the lookup.</typeparam>
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

	/// <summary>
	/// A view to a lookup attachment on vertices of a graph.
	/// </summary>
	/// <typeparam name="T">The type of the lookup</typeparam>
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
		/// <summary>
		/// Assigns a vertex to a value in this lookup.
		/// </summary>
		/// <param name="v">The vertex to assign to this value.</param>
		/// <param name="t">The value.</param>
		inline void set(const DigraphVertex& v, const T& t) {
			mLookup->mTtoId[t] = v.id();
		}
		/// <summary>
		/// Clear this value in the lookup.
		/// </summary>
		/// <param name="t">The value to clear.</param>
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
		/// <summary>
		/// Try to find this value in the lookup.
		/// </summary>
		/// <param name="t">The value to find.</param>
		/// <param name="out">The vertex found.</param>
		/// <returns>Whether or not a vertex was found.</returns>
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = mLookup->mTtoId.find(t);
			if (it == mLookup->mTtoId.end())
				return false;
			else {
				*out = mLookup->mParent->getVertex(it->second);
				return true;
			}
		}
	};

	/// <summary>
	/// A view to a lookup attachment on edges of a graph.
	/// </summary>
	/// <typeparam name="T"></typeparam>
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
			mLookup->mParent->getEdge(mLookup->mTtoId[t]);
		}
		/// <summary>
		/// Assigns a vertex to a value in this lookup.
		/// </summary>
		/// <param name="v">The vertex to assign to this value.</param>
		/// <param name="t">The value.</param>
		inline void set(const DigraphEdge& e, const T& t) {
			mLookup->mTtoId[t] = e.id();
		}
		/// <summary>
		/// Clear this value in the lookup.
		/// </summary>
		/// <param name="t">The value to clear.</param>
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
		/// <summary>
		/// Try to find this value in the lookup.
		/// </summary>
		/// <param name="t">The value to find.</param>
		/// <param name="out">The vertex found.</param>
		/// <returns>Whether or not a vertex was found.</returns>
		inline bool tryFind(const T& t, DigraphVertex* out) {
			auto it = mLookup->mTtoId.find(t);
			if (it == mLookup->mTtoId.end())
				return false;
			else {
				*out = mLookup->mParent->getVertex(it->second);
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

	/// <summary>
	/// A view to a data attachment of a directed graph.
	/// </summary>
	/// <typeparam name="T">The type of the data.</typeparam>
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

	/// <summary>
	/// A view to a sparse data attachment of a directed graph.
	/// </summary>
	/// <typeparam name="T">The type of the data.</typeparam>
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

	/// <summary>
	/// An iterator for a depth first search of a digraph starting at a given node.
	/// </summary>
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

	/// <summary>
	/// An iterator for a breadth first search of a digraph starting at a given node.
	/// </summary>
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
	void Digraph::destroyData(DigraphDataView<T>& view) {
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
	void Digraph::destroyData(DigraphSparseDataView<T>& view) {
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
	void Digraph::destroyLookup(DigraphVertexLookupView<T>& view) {
		mVertexLookups.erase(view.name());
	}

	template <typename T>
	void Digraph::destroyLookup(DigraphEdgeLookupView<T>& view) {
		mEdgeLookups.erase(view.name());
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
	DigraphVertexLookupView<T> Digraph::createVertexLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createVertexLookup(s.str());
	}

	template <typename T>
	DigraphEdgeLookupView<T> Digraph::createEdgeLookup() {
		std::stringstream s;
		s << "__unamed__";
		s << mLookupsCreated;
		s << "__";
		return createEdgeLookup(s.str());
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

#endif