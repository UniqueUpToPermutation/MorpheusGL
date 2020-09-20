#include <engine/halfedgeloader.hpp>
#include <engine/halfedge.hpp>
#include <engine/core.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/vector2.h>

#include <string>
#include <iostream>

using namespace std;
using namespace glm;

#define DEFAULT_RELATIVE_JOIN_EPSILON 0.0001f

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

	ContentFactory<HalfEdgeGeometry>::ContentFactory()
	{
		mImporter = new Assimp::Importer();
	}

	HalfEdgeGeometry* ContentFactory<HalfEdgeGeometry>::loadUnmanaged(const std::string& source) {
		HalfEdgeLoadParameters params;
		params.mRelativeJoinEpsilon = DEFAULT_RELATIVE_JOIN_EPSILON;
		return loadUnmanaged(source, params);
	}

	HalfEdgeGeometry* ContentFactory<HalfEdgeGeometry>::loadUnmanaged(const std::string& source, const HalfEdgeLoadParameters& params) {

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

		for (uint32_t i = 0; i < nVerts; ++i) {
			auto v = conv(mesh->mVertices[i]);
			geo->vertexPositions.push_back(v);
			aabb.mLower = glm::min(v, aabb.mLower);
			aabb.mUpper = glm::max(v, aabb.mUpper);
		}
		stype join_eps = params.mRelativeJoinEpsilon * glm::length(aabb.mUpper - aabb.mLower);
			
		if (bNormals)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexNormals.push_back(conv(mesh->mNormals[i]));

		if (bUVs)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexUVs.push_back(conv(mesh->mTextureCoords[0][i]));

		if (bTangents)
			for (uint32_t i = 0; i < nVerts; ++i)
				geo->vertexUVs.push_back(conv(mesh->mTangents[i]));

		vector<vec3*> posPtrs;
		posPtrs.reserve(geo->vertexPositions.size());
		for (auto& v : geo->vertexPositions)
			posPtrs.push_back(&v);

		sort(posPtrs.begin(), posPtrs.end(), [](const vec3* a, const vec3* b) {
			return a->x < b->x;
		});

		// Compute join map
		vector<uint32_t> joinMap;
		joinMap.reserve(geo->vertexPositions.size());
		for (uint32_t i = 0; i < geo->vertexPositions.size(); ++i)
			joinMap.push_back(i);
		
		bool bJoinFound = false;
		for (uint32_t i = 0; i < geo->vertexPositions.size(); ++i) {
			for (uint32_t j = i + 1; j < geo->vertexPositions.size() &&
				posPtrs[j]->x - posPtrs[i]->x < join_eps; ++j) {
				auto dist = glm::length(*(posPtrs[j]) - *(posPtrs[i]));
				if (dist < join_eps) {
					joinMap[posPtrs[j] - &geo->vertexPositions[0]] = static_cast<uint32_t>(posPtrs[i] - &geo->vertexPositions[0]);
					bJoinFound = true;
				}
			}
		}

		vector<uint32_t> srcToCompressedMap;
		srcToCompressedMap.resize(joinMap.size());

		if (bJoinFound) {
			// Flatten join map
			for (uint32_t i = 0; i < joinMap.size(); ++i) {
				uint32_t j = i;
				while (joinMap[j] != j) {
					j = joinMap[j];
				}
				joinMap[i] = j;
			}

			multimap<uint32_t, uint32_t> compressedToSrcMultiMap;
			vector<uint32_t> compressedToSrcCount;
			
			for (uint32_t i = 0; i < joinMap.size(); ++i) {
				// Save this vertex
				if (joinMap[i] == i) {
					auto compressedId = static_cast<uint32_t>(compressedToSrcCount.size());
					srcToCompressedMap[i] = compressedId;
					compressedToSrcMultiMap.emplace(compressedId, i);
					compressedToSrcCount.push_back(1);
				}
			}
			for (uint32_t i = 0; i < joinMap.size(); ++i) {
				if (joinMap[i] != i) {
					auto dest = srcToCompressedMap[joinMap[i]];
					compressedToSrcMultiMap.emplace(dest, i);
					compressedToSrcCount[dest]++;
					srcToCompressedMap[i] = dest;
				}
			}

			// Compress everything
			vector<vec3> newPos;
			vector<vec2> newUv;
			vector<vec3> newNormal;
			vector<vec3> newTangent;

			if (geo->vertexPositions.size() > 0) {
				newPos.reserve(compressedToSrcCount.size());
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newPos.push_back(zero<vec3>());
				for (auto& it : compressedToSrcMultiMap) {
					vec3 vpos = geo->vertexPositions[it.second];
					newPos[it.first] += vpos;
				}
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newPos[i] /= static_cast<stype>(compressedToSrcCount[i]);
				geo->vertexPositions = newPos;
			}
			if (geo->vertexUVs.size() > 0) {
				newUv.reserve(compressedToSrcCount.size());
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newUv.push_back(zero<vec3>());
				for (auto& it : compressedToSrcMultiMap)
					newUv[it.first] += geo->vertexUVs[it.second];
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newUv[i] /= static_cast<stype>(compressedToSrcCount[i]);
				geo->vertexUVs = newUv;
			}
			if (geo->vertexNormals.size() > 0) {
				newNormal.reserve(compressedToSrcCount.size());
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newNormal.push_back(zero<vec3>());
				for (auto& it : compressedToSrcMultiMap)
					newNormal[it.first] += geo->vertexNormals[it.second];
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newNormal[i] = glm::normalize(newNormal[i]);
				geo->vertexNormals = newNormal;
			}
			if (geo->vertexTangents.size() > 0) {
				newTangent.reserve(compressedToSrcCount.size());
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newTangent.push_back(zero<vec3>());
				for (auto& it : compressedToSrcMultiMap)
					newTangent[it.first] += geo->vertexTangents[it.second];
				for (uint32_t i = 0; i < compressedToSrcCount.size(); ++i)
					newTangent[i] = glm::normalize(newTangent[i]);
				geo->vertexTangents = newTangent;
			}
		}
		else {
			for (uint32_t i = 0; i < srcToCompressedMap.size(); ++i)
				srcToCompressedMap[i] = i;
		}

		unordered_map<pair<int, int>, int, pair_hash> vertexToEdgeMap;

		// Make faces and half edges
		for (uint32_t i = 0; i < nFaces; ++i) {
			auto& face = mesh->mFaces[i];
			RawFace f;
			int faceId = static_cast<int>(geo->faces.size());
			int newEdgesIdStart = static_cast<int>(geo->edges.size());

			for (uint32_t vi = 1; vi <= face.mNumIndices; ++vi) {
				unsigned int headId = srcToCompressedMap[vi == face.mNumIndices ? face.mIndices[0] : face.mIndices[vi]];
				unsigned int tailId = srcToCompressedMap[face.mIndices[vi - 1]];

				int newEdgeId = static_cast<int>(geo->edges.size());

				f.edge = newEdgeId;
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

		geo->vertices.resize(geo->vertexPositions.size());

		// Link half edges
		for (auto& item : vertexToEdgeMap) {
			// Link tail vertex to this edge
			geo->vertices[item.first.first].edge = item.second;

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
			auto traverseEdgeId = geo->edges[edgeId].opposite;
			assert(traverseEdgeId != -1);

			while (geo->edges[traverseEdgeId].face != -1) {
				traverseEdgeId = geo->edges[traverseEdgeId].next;
				traverseEdgeId = geo->edges[traverseEdgeId].opposite;
			}

			geo->edges[traverseEdgeId].next = edgeId;

			// Link vertices if they haven't already been
			geo->vertices[geo->edges[traverseEdgeId].head].edge = edgeId;
		}

		return geo;
	}

	ref<void> ContentFactory<HalfEdgeGeometry>::load(const std::string& source, Node& loadInto) {
		return ref<void>(loadUnmanaged(source));
	}

	void ContentFactory<HalfEdgeGeometry>::unload(ref<void> ref) {
		delete ref.reinterpretGet<HalfEdgeGeometry>();
	}

	void ContentFactory<HalfEdgeGeometry>::dispose()
	{
		delete mImporter;
		delete this;
	}

	std::string ContentFactory<HalfEdgeGeometry>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(HalfEdgeGeometry);
	}
}
