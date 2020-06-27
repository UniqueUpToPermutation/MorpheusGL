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

		inline bool valid() { return mCurrent != -1; }
		inline int id() const { return mCurrent; }
	};

	class DigraphVertex {
	private:
		Digraph* mPtr;
		int mId;

	public:
		inline explicit DigraphVertex() : mPtr(nullptr), mId(-1) { }
		inline explicit DigraphVertex(Digraph* ptr, int id) :
			mPtr(ptr), mId(id) { }

		inline DigraphEdgeIteratorF getOutgoingEdges();
		inline DigraphEdgeIteratorB getIngoingEdges();
		inline DigraphVertexIteratorF getOutgoingNeighbors();
		inline DigraphVertexIteratorB getIngoingNeighbors();
		inline int id() const { return mId; }
		inline uint32_t outDegree();
		inline uint32_t inDegree();

		template <typename T>
		inline T& data(const std::string& s);
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

		DigraphVertex createVertex();
		DigraphEdge createEdge(int tail, int head);
		inline DigraphEdge createEdge(DigraphVertex& tail, DigraphVertex& head) { return createEdge(tail.id(), head.id()); }
		void deleteVertex(DigraphVertex& v);
		void deleteVertex(int v);
		void deleteEdge(DigraphEdge& e);
		void compress(bool bTight = false);

		uint32_t edgeCount() const { return mEdgeCount; }
		uint32_t vertexCount() const { return mVertexCount; }

		inline uint32_t vertexReserve() const { return mVertexReserve; }
		inline uint32_t edgeReserve() const { return mEdgeReserve; }
		inline uint32_t vertexActiveBlock() const { return mVertexActiveBlock; }
		inline uint32_t edgeActiveBlock() const { return mEdgeActiveBlock; }

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

		explicit DigraphData(Digraph* parent, const std::string& name) :
			mParent(parent), mName(name) {

			switch (type()) {
			case DigraphDataType::EDGE:
				mDataSize = parent->edgeReserve();
				break;
			case DigraphDataType::VERTEX:
				mDataSize = parent->vertexReserve();
				break;
			default:
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
		Digraph* mParent;
		DigraphLookupType mType;
		std::unordered_map<T, int> mTtoId;

	protected:
		void applyMap(const int map[], const uint32_t mapSize) override {
			for (auto& item : mTtoId)
				item.second = map[item.second];
		}

	public:
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

		inline explicit DigraphLookup(Digraph* parent, const DigraphLookupType type) :
			mParent(parent), mType(type) { }

		friend class Digraph;
		friend struct DigraphVertexLookupView<T>;
		friend struct DigraphEdgeLookupView<T>;
	};

	template <typename T>
	struct DigraphVertexLookupView {
	private:
		DigraphLookup<T>* mLookup;

	public:
		inline DigraphVertexLookupView(DigraphLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphVertexLookupView() : mLookup(nullptr) { }

		inline DigraphVertex operator[](const T& t) {
			return mLookup->mParent->getVertex(mLookup->mTtoId[t]);
		}
		inline void set(const DigraphVertex& v, const T& t) {
			mLookup->mTtoId[t] = v.id();
		}
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
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
	struct DigraphEdgeLookupView {
	private:
		DigraphLookup<T>* mLookup;

	public:
		inline DigraphEdgeLookupView(DigraphLookup<T>* lookup) : mLookup(lookup) { }
		inline DigraphEdgeLookupView() : mLookup(nullptr) { }

		inline DigraphEdge operator[](const T& t) {
			mLookup->mParent->getEdge(mLookup->mTtoId[t]);
		}
		inline void set(const DigraphEdge& e, const T& t) {
			mLookup->mTtoId[t] = e.id();
		}
		inline void clear(const T& t) {
			mLookup->mTtoId.erase(t);
		}
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
		DigraphData<T>* mPtr;

	public:
		explicit inline DigraphDataView(DigraphData<T>* ptr) : mPtr(ptr) { }
		explicit inline DigraphDataView() : mPtr(nullptr) { }

		inline DigraphDataType type() const { return mPtr->type(); }
		inline T& operator[](const int id) { return mPtr->mData[id]; }
		inline T& operator[](const DigraphVertex& v) { return mPtr->mData[v.id()]; }
		inline T& operator[](const DigraphEdge& e) { return mPtr->mData[e.id()]; }
		inline std::string& name() { return mPtr->name(); }

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

	inline DigraphVertex DigraphEdge::head() {
		return DigraphVertex(mPtr, mPtr->mEdges[mId].mHead);
	}
	inline DigraphVertex DigraphEdge::tail() {
		return DigraphVertex(mPtr, mPtr->mEdges[mId].mTail);
	}
	inline int DigraphEdge::id() const {
		return mId;
	}
	inline DigraphEdgeIteratorF DigraphVertex::getOutgoingEdges() {
		return DigraphEdgeIteratorF(mPtr, mPtr->mVertices[mId].mOutEdge);
	}
	inline DigraphEdgeIteratorB DigraphVertex::getIngoingEdges() {
		return DigraphEdgeIteratorB(mPtr, mPtr->mVertices[mId].mInEdge);
	}
	inline DigraphVertexIteratorB DigraphVertex::getIngoingNeighbors() {
		return DigraphVertexIteratorB(mPtr, mPtr->mVertices[mId].mInEdge);
	}
	inline DigraphVertexIteratorF DigraphVertex::getOutgoingNeighbors() {
		return DigraphVertexIteratorF(mPtr, mPtr->mVertices[mId].mOutEdge);
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
		mEdgeDatas[name] = ptr;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	DigraphDataView<T> Digraph::createVertexData(const std::string& name) {
		auto ptr = new DigraphVertexData<T>(this, name);
		mVertexDatas[name] = ptr;
		return DigraphDataView<T>(ptr);
	}

	template <typename T>
	void Digraph::destroyData(DigraphDataView<T>& view) {
		switch (view.type()) {
		case DigraphDataType::VERTEX:
			mVertexDatas.erase(view->name());
			break;
		case DigraphDataType::EDGE:
			mEdgeDatas.erase(view->name());
			break;
		}
		delete view.mPtr;
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
		mVertexLookups[name] = lookup;
		return DigraphVertexLookupView<T>(lookup);
	}

	template <typename T>
	DigraphEdgeLookupView<T> Digraph::createEdgeLookup(const std::string& name) {
		auto lookup = new DigraphLookup<T>(this, DigraphLookupType::EDGE);
		mEdgeLookups[name] = lookup;
		return DigraphEdgeLookupView<T>(lookup);
	}
}	

#endif