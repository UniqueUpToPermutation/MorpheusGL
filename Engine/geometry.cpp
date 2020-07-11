#include "geometry.hpp"

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
	ContentFactory<Geometry>::~ContentFactory() {
		delete mImporter;
	}

	ref<void> ContentFactory<Geometry>::load(const std::string& source, Node& loadInto) {
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

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
		auto r = ref.as<Geometry>();
		GLuint bufs[2] = { r->mVbo, r->mIbo };
		GLuint vao = r->mVao;
		glDeleteBuffers(2, bufs);
		glDeleteVertexArrays(1, &vao);
		delete r.get();
	}
	void ContentFactory<Geometry>::dispose() {
		delete mImporter;
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
}