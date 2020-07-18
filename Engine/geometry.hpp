/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: geometry.hpp
*	Description: Defines the Geometry content type, which contains an index and vertex buffer, 
*	as well as a vertex array object.
*/

#pragma once

#include "content.hpp"

#include <glad/glad.h>

namespace Assimp {
	class Importer;
}

namespace Morpheus {

	class HalfEdgeGeometry;

	/// <summary>
	/// A piece of OpenGL geometry data in the engine.
	/// </summary>
	class Geometry {
	private:
		GLuint mVao;
		GLuint mVbo;
		GLuint mIbo;
		GLenum mElementType;
		GLsizei mElementCount;
		GLenum mIndexType;
		BoundingBox mAabb;

		inline Geometry() { }
		inline Geometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb) :
			mVao(vao), mVbo(vbo), mIbo(ibo), mElementType(elementType),
			mElementCount(elementCount), mIndexType(indexType),
			mAabb(aabb) { }

	public:
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

	/// <summary>
	/// Used for converting HalfEdgeGeometry into renderable Geometry
	/// </summary>
	struct HalfEdgeAttributes {
		GLint mPositionAttribute;
		GLint mUVAttribute;
		GLint mNormalAttribute;
		GLint mTangentAttribute;
		GLint mColorAttribute;
	};

	/// <summary>
	/// A factory for loading Geometry using the open asset
	/// import library.
	/// </summary>
	template <>
	class ContentFactory<Geometry> : public IContentFactory {
	private:
		Assimp::Importer* mImporter;

	public:
		ContentFactory();
		~ContentFactory();

		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;

		Node makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb, const std::string& source, ref<Geometry>* refOut = nullptr) const;
		Node makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
			GLenum elementType, GLsizei elementCount, GLenum indexType,
			BoundingBox aabb, ref<Geometry>* refOut = nullptr) const;
		Node makeGeometry(const HalfEdgeGeometry* geo, 
			const HalfEdgeAttributes& attrib,
			const std::string& source,
			ref<Geometry>* refOut = nullptr) const;
		Node makeGeometry(const HalfEdgeGeometry* geo,
			const HalfEdgeAttributes& attrib,
			ref<Geometry>* refOut = nullptr) const;
		Node makeGeometry(const HalfEdgeGeometry* geo,
			ref<Geometry>* refOut = nullptr) const;
	};
}