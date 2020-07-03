#pragma once

#include "core.hpp"

namespace Morpheus {
	typedef glm::vec3 vec3type;
	typedef glm::vec2 vec2type;
	typedef float stype;

	struct RawVertex {
		int edge;

		inline RawVertex(const int edge) : edge(edge) {}
	};

	struct RawEdge {
		int face;
		int head;
		int next;
		int opposite;

		inline RawEdge(const int face, const int head, const int next, const int opposite) :
			face(face), head(head), next(next), opposite(opposite) {}
	};

	struct RawFace {
		int edge;

		inline RawFace(const int edge) : edge(edge) {}
	};

	class Vertex;
	class Edge;
	class Face;
	class EdgeIterator;
	class VertexIterator;
	class FaceIterator;
	class HalfEdgeGeometry;

	class Vertex {
	private:
		HalfEdgeGeometry* geo_;
		const HalfEdgeGeometry* cgeo_;
		int id_;

	public:
		explicit inline Vertex(HalfEdgeGeometry* geo, const HalfEdgeGeometry* cgeo, int id) : geo_(geo), cgeo_(cgeo), id_(id) {}

		inline bool isConst() const { return geo_ == nullptr; }
		inline int id() const { return id_; }

		inline vec3type* ptrPosition();
		inline vec2type* ptrUV();
		inline vec3type* ptrNormal();
		inline vec3type* ptrTangent();
		inline vec3type position() const;
		inline vec2type uv() const;
		inline vec3type normal() const;
		inline vec3type tangent() const;
		inline void setPosition(const stype x, const stype y, const stype z);
		inline void setPosition(const vec3type& p);
		inline void setUV(const stype x, const stype y);
		inline void setUV(const vec2type& uv);
		inline void setNormal(const stype x, const stype y, const stype z);
		inline void setNormal(const vec3type& normal);
		inline void setTangent(const stype x, const stype y, const stype z);
		inline void setTangent(const vec3type& tangent);
		inline RawVertex* raw();
		inline void setEdge(const Edge& e);
		inline Edge edge() const;
		inline EdgeIterator outgoing() const;
		inline EdgeIterator incoming() const;
		inline VertexIterator neighbors() const;
		inline FaceIterator faces() const;
		inline Vertex nextById() const;
		inline bool isValid() const;

		friend class Edge;
		friend class Face;
		friend class VertexIterator;
		friend class EdgeIterator;
		friend class FaceIterator;
		friend class HalfEdgeGeometry;
	};

	class Edge {
	private:
		HalfEdgeGeometry* geo_;
		const HalfEdgeGeometry* cgeo_;
		int id_;

	public:
		explicit inline Edge(HalfEdgeGeometry* geo, const HalfEdgeGeometry* cgeo, int id) : geo_(geo), cgeo_(cgeo), id_(id) {}

		inline bool isConst() const { return geo_ == nullptr; }
		inline int id() const { return id_; }

		inline RawEdge* raw();
		inline void setOpposite(const Edge& e);
		inline void setHead(const Vertex& v);
		inline void setNext(const Edge& e);
		inline void setFace(const Face& f);
		inline Edge opposite() const;
		inline Vertex head() const;
		inline Vertex tail() const;
		inline Face face() const;
		inline Edge next() const;
		inline Edge nextById() const;
		inline bool isValid() const;
		inline vec3type direction() const;

		friend class Vertex;
		friend class Face;
		friend class VertexIterator;
		friend class EdgeIterator;
		friend class FaceIterator;
		friend class HalfEdgeGeometry;
	};

	class Face {
	private:
		HalfEdgeGeometry* geo_;
		const HalfEdgeGeometry* cgeo_;
		int id_;

	public:
		explicit inline Face(HalfEdgeGeometry* geo, const HalfEdgeGeometry* cgeo, int id) : geo_(geo), cgeo_(cgeo), id_(id) {}

		inline bool isConst() const { return geo_ == nullptr; }
		inline int id() const { return id_; }

		inline RawFace* raw();
		inline void setEdge(const Edge& e);
		inline Edge edge() const;
		inline EdgeIterator edges() const;
		inline FaceIterator adjacent() const;
		inline VertexIterator vertices() const;
		inline Face nextById() const;
		inline bool isValid() const;
		inline stype area() const;

		friend class Vertex;
		friend class Edge;
		friend class VertexIterator;
		friend class EdgeIterator;
		friend class FaceIterator;
		friend class HalfEdgeGeometry;
	};

	class VertexIterator {
	private:
		Edge currentEdge;
		Edge startEdge;

		void nextAdjacent() {
			currentEdge = currentEdge.opposite().next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

		void nextOnFace() {
			currentEdge = currentEdge.next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

		void(VertexIterator::* nextIt)();

	public:

		inline explicit VertexIterator(Edge startEdge, void(VertexIterator::* next)()) :
			currentEdge(startEdge), startEdge(startEdge), nextIt(next) {}

		inline void next() { (this->*nextIt)(); }

		inline bool done() const { return currentEdge.id_ < 0; }
		inline bool isValid() const { return !done(); }
		inline Vertex operator()() { return currentEdge.head(); }

		friend class Vertex;
		friend class Edge;
		friend class Face;
	};

	class EdgeIterator {
	private:
		Edge currentEdge;
		Edge startEdge;
		void(EdgeIterator::* nextIt)();

		void nextOnFace() {
			currentEdge = currentEdge.next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

		void nextOutgoing() {
			currentEdge = currentEdge.opposite().next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

		void nextIngoing() {
			currentEdge = currentEdge.next().opposite();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

	public:
		inline explicit EdgeIterator(Edge startEdge, void(EdgeIterator::* next)()) :
			currentEdge(startEdge), startEdge(startEdge), nextIt(next) {}

		inline void next() { (this->*nextIt)(); }

		inline Edge operator()() { return currentEdge; }
		inline bool done() const { return currentEdge.id_ < 0; }
		inline bool isValid() const { return !done(); }

		friend class Vertex;
		friend class Edge;
		friend class Face;
	};

	class FaceIterator {
	private:
		Edge currentEdge;
		Edge startEdge;
		void(FaceIterator::* nextIt)();

		void faceNextAdjacentFace() {
			currentEdge = currentEdge.next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

		void vertexNextAdjacentFace() {
			currentEdge = currentEdge.opposite().next();
			if (currentEdge.id_ == startEdge.id_)
				currentEdge.id_ = -1;
		}

	public:
		inline explicit FaceIterator(Edge startEdge, void(FaceIterator::* next)()) :
			currentEdge(startEdge), startEdge(startEdge), nextIt(next) {}

		inline void next() { (this->*nextIt)(); }

		inline bool done() const { return currentEdge.id_ < 0; }
		inline bool isValid() const { return !done(); }
		inline Face operator()() { return currentEdge.opposite().face(); }

		friend class Vertex;
		friend class Edge;
		friend class Face;
	};

	class HalfEdgeGeometry {
	public:
		typedef float basetype;

	private:
		std::vector<vec3type> vertexPositions;
		std::vector<vec2type> vertexUVs;
		std::vector<vec3type> vertexNormals;
		std::vector<vec3type> vertexTangents;
		std::vector<RawVertex> vertices;
		std::vector<RawEdge> edges;
		std::vector<RawFace> faces;

		BoundingBox aabb;

	public:
		explicit HalfEdgeGeometry() {}
		explicit HalfEdgeGeometry(const HalfEdgeGeometry& geo);

		inline BoundingBox getBoundingBox() const {
			return aabb;
		}
		inline bool hasPositions() const {
			return !vertexPositions.empty();
		}
		inline bool hasUVs() const {
			return !vertexUVs.empty();
		}
		inline bool hasNormals() const {
			return !vertexNormals.empty();
		}
		inline bool hasTangents() const {
			return !vertexTangents.empty();
		}
		inline Vertex getVertex(const int id) {
			return Vertex(this, this, id);
		}
		inline Edge getEdge(const int id) {
			return Edge(this, this, id);
		}
		inline Face getFace(const int id) {
			return Face(this, this, id);
		}
		inline Vertex constGetVertex(const int id) const {
			return Vertex(nullptr, this, id);
		}
		inline Edge constGetEdge(const int id) const {
			return Edge(nullptr, this, id);
		}
		inline Face constGetFace(const int id) const {
			return Face(nullptr, this, id);
		}
		inline size_t vertexCount() const {
			return vertices.size();
		}
		inline size_t edgeCount() const {
			return edges.size();
		}
		inline size_t faceCount() const {
			return faces.size();
		}
		void updateBoundingBox();

		void save(const std::string& path);
		void saveJson(const std::string& path);

		HalfEdgeGeometry* deepCopy() const;
		void copyTo(HalfEdgeGeometry* output) const;

		friend class Vertex;
		friend class Edge;
		friend class Face;

		friend HalfEdgeGeometry* load(const std::string& path);
		friend HalfEdgeGeometry* loadJson(const std::string& path);
	};

	HalfEdgeGeometry* load(const std::string& path);
	HalfEdgeGeometry* loadJson(const std::string& path);

	inline vec3type* Vertex::ptrPosition() {
		return &geo_->vertexPositions[id_];
	}
	inline vec2type* Vertex::ptrUV() {
		return &geo_->vertexUVs[id_];
	}
	inline vec3type* Vertex::ptrNormal() {
		return &geo_->vertexNormals[id_];
	}
	inline vec3type* Vertex::ptrTangent() {
		return &geo_->vertexTangents[id_];
	}
	inline vec3type Vertex::position() const {
		return cgeo_->vertexPositions[id_];
	}
	inline vec2type Vertex::uv() const {
		return cgeo_->vertexUVs[id_];
	}
	inline vec3type Vertex::normal() const {
		return cgeo_->vertexNormals[id_];
	}
	inline vec3type Vertex::tangent() const {
		return cgeo_->vertexTangents[id_];
	}
	inline void Vertex::setPosition(const stype x, 
		const stype y, 
		const stype z) {
		ptrPosition()->x = x;
		ptrPosition()->y = y;
		ptrPosition()->z = z;
	}
	inline void Vertex::setPosition(const vec3type& p) {
		*(ptrPosition()) = p;
	}
	inline void Vertex::setUV(const stype x, 
		const stype y) {
		ptrUV()->x = x;
		ptrUV()->y = y;
	}
	inline void Vertex::setUV(const vec2type& uv) {
		*(ptrUV()) = uv;
	}
	inline void Vertex::setNormal(const stype x, 
		const stype y, 
		const stype z) {
		ptrNormal()->x = x;
		ptrNormal()->y = y;
		ptrNormal()->z = z;
	}
	inline void Vertex::setNormal(const vec3type& normal) {
		*(ptrNormal()) = normal;
	}
	inline void Vertex::setTangent(const stype x, 
		const stype y, 
		const stype z) {
		ptrTangent()->x = x;
		ptrTangent()->y = y;
		ptrTangent()->z = z;
	}
	inline void Vertex::setTangent(const vec3type& tangent) {
		*(ptrTangent()) = tangent;
	}
	inline RawVertex* Vertex::raw() {
		return &geo_->vertices[id_];
	}
	inline void Vertex::setEdge(const Edge& e) {
		raw()->edge = e.id_;
	}
	inline Edge Vertex::edge() const {
		return Edge(geo_, cgeo_, cgeo_->vertices[id_].edge);
	}
	inline EdgeIterator Vertex::outgoing() const {
		return EdgeIterator(edge(), &EdgeIterator::nextOutgoing);
	}
	inline EdgeIterator Vertex::incoming() const {
		return EdgeIterator(edge().opposite(), &EdgeIterator::nextIngoing);
	}
	inline VertexIterator Vertex::neighbors() const {
		return VertexIterator(edge(), &VertexIterator::nextAdjacent);
	}
	inline FaceIterator Vertex::faces() const {
		return FaceIterator(edge(), &FaceIterator::vertexNextAdjacentFace);
	}
	inline Vertex Vertex::nextById() const {
		return Vertex(geo_, cgeo_, id_ + 1);
	}
	inline bool Vertex::isValid() const {
		return id_ >= 0 && id_ < (int)cgeo_->vertices.size();
	}

	inline RawEdge* Edge::raw() {
		return &geo_->edges[id_];
	}
	inline void Edge::setOpposite(const Edge& e) {
		raw()->opposite = e.id_;
	}
	inline void Edge::setHead(const Vertex& v) {
		raw()->head = v.id_;
	}
	inline void Edge::setNext(const Edge& e) {
		raw()->next = e.id_;
	}
	inline void Edge::setFace(const Face& f) {
		raw()->face = f.id_;
	}
	inline Edge Edge::opposite() const {
		return Edge(geo_, cgeo_, cgeo_->edges[id_].opposite);
	}
	inline Vertex Edge::head() const {
		return Vertex(geo_, cgeo_, cgeo_->edges[id_].head);
	}
	inline Vertex Edge::tail() const {
		return Vertex(geo_, cgeo_, opposite().head().id_);
	}
	inline Face Edge::face() const {
		return Face(geo_, cgeo_, cgeo_->edges[id_].face);
	}
	inline Edge Edge::next() const {
		return Edge(geo_, cgeo_, cgeo_->edges[id_].next);
	}
	inline Edge Edge::nextById() const {
		return Edge(geo_, cgeo_, id_ + 1);
	}
	inline bool Edge::isValid() const {
		return id_ >= 0 && id_ < (int)cgeo_->edges.size();
	}
	inline vec3type Edge::direction() const {
		return cgeo_->vertexPositions[head().id()] - cgeo_->vertexPositions[tail().id()];
	}

	inline RawFace* Face::raw() {
		return &geo_->faces[id_];
	}
	inline void Face::setEdge(const Edge& e) {
		raw()->edge = e.id_;
	}
	inline Edge Face::edge() const {
		return Edge(geo_, cgeo_, cgeo_->faces[id_].edge);
	}
	inline EdgeIterator Face::edges() const {
		return EdgeIterator(edge(), &EdgeIterator::nextOnFace);
	}
	inline FaceIterator Face::adjacent() const {
		return FaceIterator(edge(), &FaceIterator::faceNextAdjacentFace);
	}
	inline VertexIterator Face::vertices() const {
		return VertexIterator(edge(), &VertexIterator::nextOnFace);
	}
	inline Face Face::nextById() const {
		return Face(geo_, cgeo_, id_ + 1);
	}
	inline bool Face::isValid() const {
		return id_ >= 0 && id_ < (int)geo_->faces.size();
	}
	inline stype Face::area() const {
		// Assumes the face is a triangle
		return glm::cross(edge().direction(), edge().next().direction()).length() / 2.0f;
	}
}