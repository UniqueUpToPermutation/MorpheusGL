#include <engine/halfedge.hpp>
#include <engine/json.hpp>

#include <iostream>
#include <fstream>

using namespace std;
using namespace nlohmann;

namespace Morpheus {
	HalfEdgeGeometry* loadBinary(const std::string& path)
	{
		auto geo = new HalfEdgeGeometry();

		ifstream f(path, ios::in | ios::binary);

		unsigned char hasPositions;
		unsigned char hasUvs;
		unsigned char hasNormals;
		unsigned char hasTangents;

		f.read(reinterpret_cast<char*>(&hasPositions), sizeof(unsigned char));
		f.read(reinterpret_cast<char*>(&hasUvs), sizeof(unsigned char));
		f.read(reinterpret_cast<char*>(&hasNormals), sizeof(unsigned char));
		f.read(reinterpret_cast<char*>(&hasTangents), sizeof(unsigned char));

		int vertexCount;
		int edgeCount;
		int faceCount;

		f.read(reinterpret_cast<char*>(&vertexCount), sizeof(int));
		f.read(reinterpret_cast<char*>(&edgeCount), sizeof(int));
		f.read(reinterpret_cast<char*>(&faceCount), sizeof(int));

		geo->vertices.reserve(vertexCount);
		geo->edges.reserve(edgeCount);
		geo->faces.reserve(faceCount);

		if (hasPositions)
			geo->vertexPositions.reserve(vertexCount);
		if (hasUvs)
			geo->vertexUVs.reserve(vertexCount);
		if (hasNormals)
			geo->vertexNormals.reserve(vertexCount);
		if (hasTangents)
			geo->vertexTangents.reserve(vertexCount);

		char* buf = new char[sizeof(float) * 3 * vertexCount];
		auto floatBuf = reinterpret_cast<float*>(buf);
		if (hasPositions) {
			f.read(buf, sizeof(float) * 3 * vertexCount);
			for (int i = 0, j = 0; i < vertexCount; ++i) {
				geo->vertexPositions.emplace_back(
					vec3type{ floatBuf[j++], floatBuf[j++], floatBuf[j++] }
				);
			}
		}
		if (hasUvs) {
			f.read(buf, sizeof(float) * 2 * vertexCount);
			for (int i = 0, j = 0; i < vertexCount; ++i) {
				geo->vertexUVs.emplace_back(
					vec2type{ floatBuf[j++], floatBuf[j++] }
				);
			}
		}
		if (hasNormals) {
			f.read(buf, sizeof(float) * 3 * vertexCount);
			for (int i = 0, j = 0; i < vertexCount; ++i) {
				geo->vertexNormals.emplace_back(
					vec3type{ floatBuf[j++], floatBuf[j++], floatBuf[j++] }
				);
			}
		}
		if (hasTangents) {
			f.read(buf, sizeof(float) * 3 * vertexCount);
			for (int i = 0, j = 0; i < vertexCount; ++i) {
				geo->vertexTangents.emplace_back(
					vec3type{ floatBuf[j++], floatBuf[j++], floatBuf[j++] }
				);
			}
		}
		delete[] buf;

		buf = new char[sizeof(int) * vertexCount];
		auto intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < vertexCount; ++i)
			geo->vertices.emplace_back(RawVertex{ intBuf[j++] });
		delete[] buf;

		buf = new char[sizeof(int) * 4 * edgeCount];
		intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < edgeCount; ++i)
			geo->edges.emplace_back(RawEdge{
				intBuf[j++],
				intBuf[j++],
				intBuf[j++],
				intBuf[j++]
				});
		delete[] buf;

		buf = new char[sizeof(int) * faceCount];
		intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < faceCount; ++i)
			geo->faces.emplace_back(RawFace{
				intBuf[j++]
				});
		delete[] buf;

		f.close();

		geo->updateBoundingBox();

		return geo;
	}

	void HalfEdgeGeometry::updateBoundingBox()
	{
		vec3type lower = vec3type{ std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity() };
		vec3type upper = vec3type{ -std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity() };

		if (hasPositions()) {
			for (uint32_t i = 0; i < vertexCount(); ++i) {
				lower.x = min(lower.x, vertexPositions[i].x);
				lower.y = min(lower.x, vertexPositions[i].y);
				lower.z = min(lower.x, vertexPositions[i].z);

				upper.x = max(upper.x, vertexPositions[i].x);
				upper.y = max(upper.y, vertexPositions[i].y);
				upper.z = max(upper.z, vertexPositions[i].z);
			}
			aabb.mLower = lower;
			aabb.mUpper = upper;
		}
	}

	void HalfEdgeGeometry::saveBinary(const std::string& path)
	{
		ofstream f(path, ios::out | ios::binary);

		char hasPositions = this->hasPositions();
		char hasUvs = this->hasUVs();
		char hasNormals = this->hasNormals();
		char hasTangents = this->hasTangents();

		f.write(&hasPositions, sizeof(char));
		f.write(&hasUvs, sizeof(char));
		f.write(&hasNormals, sizeof(char));
		f.write(&hasTangents, sizeof(char));

		size_t vertexCount = vertices.size();
		size_t edgeCount = edges.size();
		size_t faceCount = faces.size();

		f.write(reinterpret_cast<char*>(&vertexCount), sizeof(int));
		f.write(reinterpret_cast<char*>(&edgeCount), sizeof(int));
		f.write(reinterpret_cast<char*>(&faceCount), sizeof(int));

		char* buf = new char[sizeof(float) * 3 * vertexCount];
		auto floatBuf = reinterpret_cast<float*>(buf);
		if (hasPositions) {
			for (size_t i = 0, j = 0; i < vertexCount; ++i) {
				floatBuf[j++] = (float)vertexPositions[i].x;
				floatBuf[j++] = (float)vertexPositions[i].y;
				floatBuf[j++] = (float)vertexPositions[i].z;
			}
			f.write(buf, sizeof(float) * 3 * vertexCount);
		}
		if (hasUvs) {
			for (size_t i = 0, j = 0; i < vertexCount; ++i) {
				floatBuf[j++] = (float)vertexUVs[i].x;
				floatBuf[j++] = (float)vertexUVs[i].y;
			}
			f.write(buf, sizeof(float) * 2 * vertexCount);
		}
		if (hasNormals) {
			for (size_t i = 0, j = 0; i < vertexCount; ++i) {
				floatBuf[j++] = (float)vertexNormals[i].x;
				floatBuf[j++] = (float)vertexNormals[i].y;
				floatBuf[j++] = (float)vertexNormals[i].z;
			}
			f.write(buf, sizeof(float) * 3 * vertexCount);
		}
		if (hasTangents) {
			for (size_t i = 0, j = 0; i < vertexCount; ++i) {
				floatBuf[j++] = (float)vertexTangents[i].x;
				floatBuf[j++] = (float)vertexTangents[i].y;
				floatBuf[j++] = (float)vertexTangents[i].z;
			}
			f.write(buf, sizeof(float) * 3 * vertexCount);
		}
		delete[] buf;

		buf = new char[sizeof(int) * vertexCount];
		auto intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < vertexCount; ++i)
			intBuf[j++] = vertices[i].edge;
		f.write(buf, sizeof(int) * vertexCount);
		delete[] buf;

		buf = new char[sizeof(int) * 4 * edgeCount];
		intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < edgeCount; ++i) {
			intBuf[j++] = edges[i].face;
			intBuf[j++] = edges[i].head;
			intBuf[j++] = edges[i].next;
			intBuf[j++] = edges[i].opposite;
		}
		f.write(buf, sizeof(int) * 4 * edgeCount);
		delete[] buf;

		buf = new char[sizeof(int) * faceCount];
		intBuf = reinterpret_cast<int*>(buf);
		for (int i = 0, j = 0; i < faceCount; ++i)
			intBuf[j++] = faces[i].edge;
		f.write(buf, sizeof(int) * faceCount);
		delete[] buf;

		f.close();
	}

	void HalfEdgeGeometry::saveJson(const std::string& path)
	{
		ofstream f(path);
		json j;

		if (hasPositions()) {
			std::vector<float> positions;
			positions.reserve(3 * vertexCount());
			for (auto& v : vertexPositions) {
				positions.emplace_back((float)v.x);
				positions.emplace_back((float)v.y);
				positions.emplace_back((float)v.z);
			}
			j["vertexPositions"] = positions;
		}
		else
			j["vertexPositions"] = nullptr;

		if (hasUVs()) {
			std::vector<float> uvs;
			uvs.reserve(2 * vertexCount());
			for (auto& v : vertexUVs) {
				uvs.emplace_back((float)v.x);
				uvs.emplace_back((float)v.y);
			}
			j["vertexUVs"] = uvs;
		}
		else
			j["vertexUVs"] = nullptr;

		if (hasNormals()) {
			std::vector<float> normals;
			normals.reserve(3 * vertexCount());
			for (auto& v : vertexNormals) {
				normals.emplace_back((float)v.x);
				normals.emplace_back((float)v.y);
				normals.emplace_back((float)v.z);
			}
			j["vertexNormals"] = normals;
		}
		else
			j["vertexNormals"] = nullptr;

		if (hasTangents()) {
			std::vector<float> tangents;
			tangents.reserve(3 * vertexCount());
			for (auto& v : vertexTangents) {
				tangents.emplace_back((float)v.x);
				tangents.emplace_back((float)v.y);
				tangents.emplace_back((float)v.z);
			}
			j["vertexTangents"] = tangents;
		}
		else
			j["vertexTangents"] = nullptr;

		std::vector<int> vertexAttrEdge;
		vertexAttrEdge.reserve(vertexCount());
		std::vector<int> edgeAttrNext;
		edgeAttrNext.reserve(edgeCount());
		std::vector<int> edgeAttrHead;
		edgeAttrHead.reserve(edgeCount());
		std::vector<int> edgeAttrOpposite;
		edgeAttrOpposite.reserve(edgeCount());
		std::vector<int> edgeAttrFace;
		edgeAttrFace.reserve(edgeCount());
		std::vector<int> faceAttrEdge;
		faceAttrEdge.reserve(faceCount());

		for (auto& v : vertices)
			vertexAttrEdge.emplace_back(v.edge);
		for (auto& e : edges) {
			edgeAttrFace.emplace_back(e.face);
			edgeAttrHead.emplace_back(e.head);
			edgeAttrOpposite.emplace_back(e.opposite);
			edgeAttrNext.emplace_back(e.next);
		}
		for (auto& f_ : faces)
			faceAttrEdge.emplace_back(f_.edge);

		j["vertexAttrEdge"] = vertexAttrEdge;
		j["edgeAttrNext"] = edgeAttrNext;
		j["edgeAttrOpposite"] = edgeAttrOpposite;
		j["edgeAttrHead"] = edgeAttrHead;
		j["edgeAttrFace"] = edgeAttrFace;
		j["faceAttrEdge"] = faceAttrEdge;

		f << j;
		f.close();
	}

	HalfEdgeGeometry* loadJson(const std::string& filename) {
		ifstream f(filename);

		if (!f.is_open())
			throw "Error: bad path!";

		json j;
		f >> j;

		auto geo = new HalfEdgeGeometry();

		std::vector<float> vertexPositions;
		std::vector<float> vertexUvs;
		std::vector<float> vertexNormals;
		std::vector<float> vertexTangents;

		if (!j["vertexPositions"].is_null())
			vertexPositions = j["vertexPositions"].get<std::vector<float>>();
		if (!j["vertexUVs"].is_null())
			vertexUvs = j["vertexUVs"].get<std::vector<float>>();
		if (!j["vertexNormals"].is_null())
			vertexNormals = j["vertexNormals"].get<std::vector<float>>();
		if (!j["vertexTangents"].is_null())
			vertexTangents = j["vertexTangents"].get<std::vector<float>>();

		auto vertexAttrEdge = j["vertexAttrEdge"].get<std::vector<int>>();
		auto edgeAttrNext = j["edgeAttrNext"].get<std::vector<int>>();
		auto edgeAttrOpposite = j["edgeAttrOpposite"].get<std::vector<int>>();
		auto edgeAttrHead = j["edgeAttrHead"].get<std::vector<int>>();
		auto edgeAttrFace = j["edgeAttrFace"].get<std::vector<int>>();
		auto faceAttrEdge = j["faceAttrEdge"].get<std::vector<int>>();

		bool hasPositions = vertexPositions.size() > 0;
		bool hasUvs = vertexUvs.size() > 0;
		bool hasNormals = vertexNormals.size() > 0;
		bool hasTangents = vertexTangents.size() > 0;

		size_t vertexCount = vertexAttrEdge.size();
		size_t edgeCount = edgeAttrNext.size();
		size_t faceCount = faceAttrEdge.size();

		if (hasPositions) {
			geo->vertexPositions.reserve(vertexCount);
			for (size_t i = 0; i < vertexCount * 3; i += 3)
				geo->vertexPositions.emplace_back(vec3type{ vertexPositions[i], vertexPositions[i + 1], vertexPositions[i + 2] });
		}
		if (hasUvs) {
			geo->vertexUVs.reserve(vertexCount);
			for (size_t i = 0; i < vertexCount * 2; i += 2)
				geo->vertexUVs.emplace_back(vec2type{ vertexUvs[i], vertexUvs[i + 1] });
		}
		if (hasNormals) {
			geo->vertexNormals.reserve(vertexCount);
			for (size_t i = 0; i < vertexCount * 3; i += 3)
				geo->vertexNormals.emplace_back(vec3type{ vertexNormals[i], vertexNormals[i + 1], vertexNormals[i + 2] });
		}
		if (hasTangents) {
			geo->vertexTangents.reserve(vertexCount);
			for (int i = 0; i < vertexCount * 3; i += 3)
				geo->vertexTangents.emplace_back(vec3type{ vertexTangents[i], vertexTangents[i + 1], vertexTangents[i + 2] });
		}			geo->vertices.reserve(vertexCount);

		geo->vertices.reserve(vertexCount);
		geo->edges.reserve(edgeCount);
		geo->faces.reserve(faceCount);

		for (int i = 0; i < vertexCount; ++i) {
			geo->vertices.emplace_back(RawVertex(vertexAttrEdge[i]));
		}
		for (int i = 0; i < edgeCount; ++i) {
			geo->edges.emplace_back(RawEdge(edgeAttrFace[i], edgeAttrHead[i], edgeAttrNext[i], edgeAttrOpposite[i]));
		}
		for (int i = 0; i < faceCount; ++i) {
			geo->faces.emplace_back(RawFace(faceAttrEdge[i]));
		}

		f.close();

		geo->updateBoundingBox();

		return geo;
	}

	HalfEdgeGeometry::HalfEdgeGeometry(const HalfEdgeGeometry& geo) {
		geo.copyTo(this);
	}

	void HalfEdgeGeometry::copyTo(HalfEdgeGeometry* output) const {
		output->vertexPositions = vertexPositions;
		output->vertexUVs = vertexUVs;
		output->vertexNormals = vertexNormals;
		output->vertexTangents = vertexTangents;
		output->vertices = vertices;
		output->edges = edges;
		output->faces = faces;
		output->aabb = aabb;
	}

	HalfEdgeGeometry* HalfEdgeGeometry::deepCopy() const {
		return new HalfEdgeGeometry(*this);
	}
}