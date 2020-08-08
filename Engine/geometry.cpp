#include "geometry.hpp"
#include "halfedge.hpp"

#include <iostream>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

using namespace Assimp;
using namespace std;

namespace Morpheus {
	ContentFactory<Geometry>::ContentFactory() {
		mImporter = new Importer();
	}

	ref<void> ContentFactory<Geometry>::load(const std::string& source, Node& loadInto) {
		cout << "Loading geometry " << source << "..." << endl;

		const aiScene* pScene = mImporter->ReadFile(source.c_str(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
			aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Quality);

		if (!pScene) {
			cout << "Error: failed to load " << source << endl;
			return ref<void>(nullptr);
		}

		uint32_t nVerts;
		uint32_t nIndices;

		if (!pScene->HasMeshes()) {
			cout << "Error: " << source << " has no meshes!" << endl;
			return ref<void>(nullptr);
		}

		if (pScene->mNumMeshes > 1) {
			cout << "Warning: " << source << " has more than one mesh, we will just load the first." << endl;
		}

		aiMesh* mesh = pScene->mMeshes[0];

		nVerts = mesh->mNumVertices;
		nIndices = mesh->mNumFaces * 3;

		uint32_t stride = 3 + 2 + 3 + 3;

		float* vert_buffer = new float[nVerts * stride];
		uint32_t* indx_buffer = new uint32_t[nIndices];

		BoundingBox aabb;
		aabb.mLower = glm::vec3(std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity());
		aabb.mUpper = glm::vec3(-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity());

		bool bHasUVs = mesh->HasTextureCoords(0);
		bool bHasTangents = mesh->HasTangentsAndBitangents();

		for (uint32_t i = 0, bufindx = 0; i < nVerts; ++i, bufindx += stride) {
			vert_buffer[bufindx] = mesh->mVertices[i].x;
			vert_buffer[bufindx + 1] = mesh->mVertices[i].y;
			vert_buffer[bufindx + 2] = mesh->mVertices[i].z;

			vert_buffer[bufindx + 5] = mesh->mNormals[i].x;
			vert_buffer[bufindx + 6] = mesh->mNormals[i].y;
			vert_buffer[bufindx + 7] = mesh->mNormals[i].z;

			aabb.mLower = glm::min(aabb.mLower, glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
			aabb.mUpper = glm::max(aabb.mUpper, glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		}

		if (bHasTangents) {
			for (uint32_t i = 0, bufindx = 0; i < nVerts; ++i, bufindx += stride) {
				vert_buffer[bufindx + 8] = mesh->mTangents[i].x;
				vert_buffer[bufindx + 9] = mesh->mTangents[i].y;
				vert_buffer[bufindx + 10] = mesh->mTangents[i].z;
			}
		}
		else {
			for (uint32_t i = 0, bufindx = 0; i < nVerts; ++i, bufindx += stride) {
				vert_buffer[bufindx + 8] = 0.0;
				vert_buffer[bufindx + 9] = 0.0;
				vert_buffer[bufindx + 10] = 0.0;
			}
		}

		if (bHasUVs) {
			for (uint32_t i = 0, bufindx = 0; i < nVerts; ++i, bufindx += stride) {
				vert_buffer[bufindx + 3] = mesh->mTextureCoords[0][i].x;
				vert_buffer[bufindx + 4] = mesh->mTextureCoords[0][i].y;
			}
		}
		else {
			for (uint32_t i = 0, bufindx = 0; i < nVerts; ++i, bufindx += stride) {
				vert_buffer[bufindx + 3] = 0.0;
				vert_buffer[bufindx + 4] = 0.0;
			}
		}

		for (uint32_t i_face = 0, i = 0; i_face < mesh->mNumFaces; ++i_face) {
			indx_buffer[i++] = mesh->mFaces[i_face].mIndices[0];
			indx_buffer[i++] = mesh->mFaces[i_face].mIndices[1];
			indx_buffer[i++] = mesh->mFaces[i_face].mIndices[2];
		}

		GLuint bufs[2];
		glGenBuffers(2, bufs);

		glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * nVerts * stride, vert_buffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * nIndices, indx_buffer, GL_STATIC_DRAW);

		GLuint vao;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(8 * sizeof(float)));

		Geometry* geo = new Geometry();
		geo->mAabb = aabb;
		geo->mVbo = bufs[0];
		geo->mIbo = bufs[1];
		geo->mVao = vao;
		geo->mElementCount = nIndices;
		geo->mElementType = GL_TRIANGLES;
		geo->mIndexType = GL_UNSIGNED_INT;

		delete[] vert_buffer;
		delete[] indx_buffer;

		return ref<void>(geo);
	}
	void ContentFactory<Geometry>::unload(ref<void>& ref) {
		auto r = ref.reinterpretGet<Geometry>();
		GLuint bufs[2] = { r->mVbo, r->mIbo };
		GLuint vao = r->mVao;
		glDeleteBuffers(2, bufs);
		glDeleteVertexArrays(1, &vao);
		delete r;
	}
	void ContentFactory<Geometry>::dispose() {
		delete mImporter;
		delete this;
	}

	ref<Geometry> ContentFactory<Geometry>::makeGeometryUnmanaged(GLuint vao, GLuint vbo, GLuint ibo,
		GLenum elementType, GLsizei elementCount, GLenum indexType,
		BoundingBox aabb) const {
		Geometry* geo = new Geometry(vao, vbo, ibo, elementType, elementCount,
			indexType, aabb);
		return ref<Geometry>(geo);
	}

	ref<Geometry> ContentFactory<Geometry>::makeGeometryUnmanaged(const HalfEdgeGeometry* geo) const {
		return makeGeometryUnmanaged(geo, HalfEdgeAttributes::defaults());
	}

	ref<Geometry> ContentFactory<Geometry>::makeGeometryUnmanaged(const HalfEdgeGeometry* geo,
		const HalfEdgeAttributes& attrib) const {
		uint32_t current_off = 0;
		uint32_t position_off = 0;
		uint32_t uv_off = 0;
		uint32_t normal_off = 0;
		uint32_t tangent_off = 0;
		uint32_t color_off = 0;

		if (attrib.mPositionAttribute != -1) {
			position_off = current_off;
			current_off += 3;
		}
		if (attrib.mUVAttribute != -1) {
			uv_off = current_off;
			current_off += 2;
		}
		if (attrib.mNormalAttribute != -1) {
			normal_off = current_off;
			current_off += 3;
		}
		if (attrib.mTangentAttribute != -1) {
			tangent_off = current_off;
			current_off += 3;
		}
		if (attrib.mColorAttribute != -1) {
			color_off = current_off;
			current_off += 3;
		}

		uint32_t stride = current_off;
		uint32_t nVerts = static_cast<uint32_t>(geo->vertexCount());

		uint32_t nIndices = 0;
		for (auto face = geo->constGetFace(0); face.valid(); face = face.nextById()) {
			uint32_t vCount = face.vertexCount();
			nIndices += 3 * (vCount - 2);
		}

		uint32_t vert_buffer_size = nVerts * stride;
		float* vert_buffer = new float[vert_buffer_size];

		for (uint32_t i = 0; i < vert_buffer_size; ++i)
			vert_buffer[i] = 0.0;

		uint32_t* indx_buffer = new uint32_t[nIndices];

		BoundingBox aabb;
		aabb.mLower = glm::vec3(std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity());
		aabb.mUpper = glm::vec3(-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity());

		if (geo->hasPositions()) {
			uint32_t bufindx = position_off;
			for (auto v = geo->constGetVertex(0); v.valid(); v = v.nextById()) {
				auto pos = v.position();

				vert_buffer[bufindx] = pos.x;
				vert_buffer[bufindx + 1] = pos.y;
				vert_buffer[bufindx + 2] = pos.z;

				aabb.mLower = glm::min(aabb.mLower, pos);
				aabb.mUpper = glm::max(aabb.mUpper, pos);
				bufindx += stride;
			}
		}

		if (geo->hasUVs()) {
			uint32_t bufindx = uv_off;
			for (auto v = geo->constGetVertex(0); v.valid(); v = v.nextById()) {
				auto uv = v.uv();

				vert_buffer[bufindx] = uv.x;
				vert_buffer[bufindx + 1] = uv.y;

				bufindx += stride;
			}
		}

		if (geo->hasNormals()) {
			uint32_t bufindx = normal_off;
			for (auto v = geo->constGetVertex(0); v.valid(); v = v.nextById()) {
				auto normal = v.normal();

				vert_buffer[bufindx] = normal.x;
				vert_buffer[bufindx + 1] = normal.y;
				vert_buffer[bufindx + 2] = normal.z;

				bufindx += stride;
			}
		}

		if (geo->hasTangents()) {
			uint32_t bufindx = tangent_off;
			for (auto v = geo->constGetVertex(0); v.valid(); v = v.nextById()) {
				auto tangent = v.tangent();

				vert_buffer[bufindx] = tangent.x;
				vert_buffer[bufindx + 1] = tangent.y;
				vert_buffer[bufindx + 2] = tangent.z;

				bufindx += stride;
			}
		}

		if (geo->hasColors()) {
			uint32_t bufindx = color_off;
			for (auto v = geo->constGetVertex(0); v.valid(); v = v.nextById()) {
				auto clr = v.color();

				vert_buffer[bufindx] = clr.x;
				vert_buffer[bufindx + 1] = clr.y;
				vert_buffer[bufindx + 2] = clr.z;

				bufindx += stride;
			}
		}

		uint32_t j = 0;
		for (auto face = geo->constGetFace(0); face.valid(); face = face.nextById()) {
			int start_i;
			int last_i;

			auto vertIt = face.vertices();

			start_i = vertIt().id();
			vertIt.next();
			last_i = vertIt().id();
			vertIt.next();

			// Triangulate face if necessary
			for (int current_i = vertIt().id(); vertIt.valid(); vertIt.next(), last_i = current_i) {
				indx_buffer[j++] = static_cast<uint32_t>(start_i);
				indx_buffer[j++] = static_cast<uint32_t>(last_i);
				indx_buffer[j++] = static_cast<uint32_t>(current_i);
			}
		}

		GLuint bufs[2];
		glGenBuffers(2, bufs);

		glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert_buffer_size, vert_buffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * nIndices, indx_buffer, GL_STATIC_DRAW);

		GLuint vao;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);

		if (attrib.mPositionAttribute != -1) {
			glEnableVertexAttribArray(attrib.mPositionAttribute);
			glVertexAttribPointer(attrib.mPositionAttribute, 3, GL_FLOAT, GL_FALSE,
				stride * sizeof(float), (void*)(position_off * sizeof(float)));
		}
		if (attrib.mUVAttribute != -1) {
			glEnableVertexAttribArray(attrib.mUVAttribute);
			glVertexAttribPointer(attrib.mUVAttribute, 2, GL_FLOAT, GL_FALSE,
				stride * sizeof(float), (void*)(uv_off * sizeof(float)));
		}
		if (attrib.mNormalAttribute != -1) {
			glEnableVertexAttribArray(attrib.mNormalAttribute);
			glVertexAttribPointer(attrib.mNormalAttribute, 3, GL_FLOAT, GL_FALSE,
				stride * sizeof(float), (void*)(normal_off * sizeof(float)));
		}
		if (attrib.mTangentAttribute != -1) {
			glEnableVertexAttribArray(attrib.mTangentAttribute);
			glVertexAttribPointer(attrib.mTangentAttribute, 3, GL_FLOAT, GL_FALSE,
				stride * sizeof(float), (void*)(tangent_off * sizeof(float)));
		}
		if (attrib.mColorAttribute != -1) {
			glEnableVertexAttribArray(attrib.mColorAttribute);
			glVertexAttribPointer(attrib.mColorAttribute, 3, GL_FLOAT, GL_FALSE,
				stride * sizeof(float), (void*)(color_off * sizeof(float)));
		}

		Geometry* result = new Geometry();
		result->mAabb = aabb;
		result->mVbo = bufs[0];
		result->mIbo = bufs[1];
		result->mVao = vao;
		result->mElementCount = nIndices;
		result->mElementType = GL_TRIANGLES;
		result->mIndexType = GL_UNSIGNED_INT;

		delete[] vert_buffer;
		delete[] indx_buffer;

		return ref<Geometry>(result);
	}

	Node ContentFactory<Geometry>::makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
		GLenum elementType, GLsizei elementCount, GLenum indexType,
		BoundingBox aabb, const std::string& source, ref<Geometry>* refOut) const
	{
		Geometry* geo = new Geometry(vao, vbo, ibo, elementType, elementCount,
			indexType, aabb);

		if (refOut)
			*refOut = ref<Geometry>(geo);

		// Add geometry to content
		auto node = graph()->addNode(geo);
		content()->addContentNode(node, source);
		return node;
	}

	Node ContentFactory<Geometry>::makeGeometry(GLuint vao, GLuint vbo, GLuint ibo,
		GLenum elementType, GLsizei elementCount, GLenum indexType,
		BoundingBox aabb, ref<Geometry>* refOut) const
	{
		Geometry* geo = new Geometry(vao, vbo, ibo, elementType, elementCount,
			indexType, aabb);

		if (refOut)
			*refOut = ref<Geometry>(geo);

		// Add geometry to content
		auto node = graph()->addNode(geo);
		content()->addContentNode(node);
		return node;
	}

	Node ContentFactory<Geometry>::makeGeometry(const HalfEdgeGeometry* geo,
		const HalfEdgeAttributes& attrib,
		ref<Geometry>* refOut) const {
		return makeGeometry(geo, attrib, "", refOut);
	}

	Node ContentFactory<Geometry>::makeGeometry(const HalfEdgeGeometry* geo,
		const std::string& source,
		ref<Geometry>* refOut) const {
		return makeGeometry(geo, HalfEdgeAttributes::defaults(), source, refOut);
	}

	Node ContentFactory<Geometry>::makeGeometry(const HalfEdgeGeometry* geo,
		ref<Geometry>* refOut) const {
		return makeGeometry(geo, HalfEdgeAttributes::defaults(), "", refOut);
	}

	Node ContentFactory<Geometry>::makeGeometry(const HalfEdgeGeometry* geo,
		const HalfEdgeAttributes& attrib,
		const std::string& source,
		ref<Geometry>* refOut) const {

		auto result = makeGeometryUnmanaged(geo, attrib);

		if (refOut)
			*refOut = ref<Geometry>(result);

		// Add geometry to content
		Node node;
		node = graph()->addNode(result);
		if (source.length() > 0)
			content()->addContentNode(node, source);
		else
			content()->addContentNode(node);

		return node;
	}

	HalfEdgeAttributes HalfEdgeAttributes::defaults()
	{
		HalfEdgeAttributes attrib;
		attrib.mPositionAttribute = 0;
		attrib.mUVAttribute = 1;
		attrib.mNormalAttribute = 2;
		attrib.mTangentAttribute = 3;
		attrib.mColorAttribute = 4;
		return attrib;
	}
}