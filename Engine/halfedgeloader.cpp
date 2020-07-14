#include "halfedgeloader.hpp"
#include "halfedge.hpp"
#include "core.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/vector2.h>

#include <string>
#include <iostream>

using namespace std;

struct pair_hash
{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& pair) const
	{
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

namespace Morpheus {

	vec3type conv(const aiVector3D& vec) {
		return vec3type(vec.x, vec.y, vec.z);
	}

	vec2type conv(const aiVector2D& vec) {
		return vec2type(vec.x, vec.y);
	}

	HalfEdgeLoader::HalfEdgeLoader()
	{
		mImporter = new Assimp::Importer();
	}

	HalfEdgeGeometry* HalfEdgeLoader::load(const std::string& source) {

		HalfEdgeGeometry* geo;

		const aiScene* pScene = mImporter->ReadFile(source.c_str(),
			aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
			aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Quality);

		if (!pScene) {
			cout << "Error: failed to load " << source << endl;
			return nullptr;
		}

		uint32_t nVerts;
		uint32_t nIndicesGuess;
		uint32_t nFaces;

		if (!pScene->HasMeshes()) {
			cout << "Error: " << source << " has no meshes!" << endl;
			return nullptr;
		}

		if (pScene->mNumMeshes > 1) {
			cout << "Warning: " << source << " has more than one mesh, we will just load the first." << endl;
		}

		aiMesh* mesh = pScene->mMeshes[0];

		nVerts = mesh->mNumVertices;
		nIndicesGuess = mesh->mNumFaces * 3;
		nFaces = mesh->mNumFaces;

		bool bUVs = mesh->HasTextureCoords(0);
		bool bNormals = mesh->HasNormals();
		bool bTangents = mesh->HasTangentsAndBitangents();

		BoundingBox aabb;
		aabb.mLower = glm::vec3(std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity());
		aabb.mUpper = glm::vec3(-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity());

		geo = new HalfEdgeGeometry();
		geo->vertexPositions.reserve(nVerts);

		if (bNormals)
			geo->vertexNormals.reserve(nVerts);
		if (bTangents)
			geo->vertexTangents.reserve(nVerts);
		if (bUVs)
			geo->vertexUVs.reserve(nVerts);

		geo->vertices.reserve(nVerts);
		geo->edges.reserve(nIndicesGuess);
		geo->faces.reserve(nFaces);

		for (uint32_t i = 0; i < nVerts; ++i)
			geo->vertexPositions.push_back(conv(mesh->mVertices[i]));

		if (bNormals)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexNormals.push_back(conv(mesh->mNormals[i]));

		if (bUVs)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexUVs.push_back(conv(mesh->mTextureCoords[0][i]));

		if (bTangents)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexUVs.push_back(conv(mesh->mTangents[i]));

		unordered_map<pair<int, int>, int, pair_hash> vertexToEdgeMap;

		// Make faces and half edges
		for (uint32_t i = 0; i < nFaces; ++i) {
			auto& face = mesh->mFaces[i];
			RawFace f;
			int faceId = static_cast<int>(geo->faces.size());
			int newEdgesIdStart = static_cast<int>(geo->edges.size());

			for (uint32_t vi = 1; vi <= face.mNumIndices; ++vi) {
				unsigned int headId = vi == face.mNumIndices ? face.mIndices[0] : face.mIndices[vi];
				unsigned int tailId = face.mIndices[vi - 1];

				int newEdgeId = static_cast<int>(geo->edges.size());

				vertexToEdgeMap[make_pair(tailId, headId)] = newEdgeId;

				RawEdge e;
				e.face = faceId;
				e.head = headId;
				e.next = vi == face.mNumIndices ? newEdgesIdStart : newEdgeId + 1;
				e.opposite = -1;
				geo->edges.push_back(e);
			}

			geo->faces.push_back(f);
		}

		vector<int> unlinkedEdges;

		// Link half edges
		for (auto& item : vertexToEdgeMap) {
			if (geo->edges[item.second].opposite == -1) {
				auto reverse = make_pair(item.first.second, item.first.first);

				auto it = vertexToEdgeMap.find(reverse);
				if (it != vertexToEdgeMap.end()) {
					geo->edges[item.second].opposite = it->second;
					geo->edges[it->second].opposite = item.second;
				}
				else
					unlinkedEdges.push_back(item.second);
			}
		}

		vector<int> dummyEdges;

		// Create dummy edges for unlinked half edges
		for (auto edgeId : unlinkedEdges) {
			auto& edge = geo->edges[edgeId];

			int newEdgeId = static_cast<int>(geo->edges.size());
			edge.opposite = newEdgeId;

			Edge oldEdge(geo, geo, edgeId);
			Edge lastEdge = oldEdge;
			for (auto eIt = oldEdge.edgesOnFace(); !eIt.done(); eIt.next())
				lastEdge = eIt();

			// Circle around the face to get the tail
			RawEdge e;
			e.face = -1;
			e.opposite = edgeId;
			e.next = -1;
			e.head = lastEdge.head().id();

			geo->edges.push_back(e);
			dummyEdges.push_back(newEdgeId);
		}

		// Link up dummy edges
		// NOTE Mesh cannot have a vertex adjacent to multiple holes!
		for (auto edgeId : dummyEdges) {
			auto& edge = geo->edges[edgeId];
			int traverseEdgeId = edge.opposite;
			assert(edge.opposite != -1);
			auto& traverseEdge = geo->edges[traverseEdgeId];

			while (traverseEdge.face != -1) {
				traverseEdgeId = traverseEdge.opposite;
				traverseEdgeId = geo->edges[traverseEdgeId].next;
				traverseEdge = geo->edges[traverseEdgeId];
			}

			geo->edges[traverseEdgeId].next = edgeId;
		}

		return geo;
	}
	HalfEdgeLoader::~HalfEdgeLoader()
	{
		delete mImporter;
	}
}
