/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: geometry.hpp
*	Description: Defines the Geometry content type, which contains an index and vertex buffer, 
*	as well as a vertex array object.
*/

#pragma once

#include <engine/content.hpp>

#include <glad/glad.h>

namespace Assimp {
	class Importer;
}

namespace Morpheus {

	class HalfEdgeGeometry;

	// A piece of OpenGL geometry data in the engine.
	class Geometry : public INodeOwner {
	private:
		GLuint mVao;
		GLuint mVbo;
		GLuint mIbo;
		GLenum mElementType;
		GLsizei mElementCount;
		GLenum mIndexType;
		BoundingBox mAabb;

		inline Geometry() : INodeOwner(NodeType::GEOMETRY) { }
		inline Geometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb) :
			INodeOwner(NodeType::GEOMETRY), mVao(vao), mVbo(vbo), mIbo(ibo), mElementType(elementType),
			mElementCount(elementCount), mIndexType(indexType),
			mAabb(aabb) { }

	public:
		Geometry* toGeometry() override;

		inline GLuint vertexArray() const { return mVao; }
		inline GLuint vertexBuffer() const { return mVbo; }
		inline GLuint indexBuffer() const { return mIbo; }
		inline BoundingBox boundingBox() const { return mAabb; }
		inline GLenum elementType() const { return mElementType; }
		inline GLsizei elementCount() const { return mElementCount; }
		inline GLenum indexType() const { return mIndexType; }

		friend class ContentFactory<Geometry>;
	};
	SET_NODE_ENUM(Geometry, GEOMETRY);

	// Used for converting HalfEdgeGeometry into renderable Geometry
	struct HalfEdgeAttributes {
		GLint mPositionAttribute;
		GLint mUVAttribute;
		GLint mNormalAttribute;
		GLint mTangentAttribute;
		GLint mColorAttribute;

		static HalfEdgeAttributes defaults();
	};
	
	// A factory for loading Geometry using the open asset
	// import library.
	template <>
	class ContentFactory<Geometry> : public IContentFactory {
	private:
		Assimp::Importer* mImporter;

	public:
		ContentFactory();

		INodeOwner* load(const std::string& source, Node loadInto) override;
		void unload(INodeOwner* ref) override;
		void dispose() override;

		Geometry* makeGeometryUnmanaged(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb) const;
		Geometry* makeGeometryUnmanaged(const HalfEdgeGeometry* geo,
			const HalfEdgeAttributes& attrib) const;
		Geometry* makeGeometryUnmanaged(const HalfEdgeGeometry* geo) const;

		Geometry* makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb, const std::string& source) const;
		Geometry* makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aab) const;
		Geometry* makeGeometry(const HalfEdgeGeometry* geo, 
			const HalfEdgeAttributes& attrib,
			const std::string& source) const;
		Geometry* makeGeometry(const HalfEdgeGeometry* geo,
			const HalfEdgeAttributes& attrib) const;
		Geometry* makeGeometry(const HalfEdgeGeometry* geo) const;
		Geometry* makeGeometry(const HalfEdgeGeometry* geo,
			const std::string& source) const;
	
		std::string getContentTypeString() const override;
	};
}