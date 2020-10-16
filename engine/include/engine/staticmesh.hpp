#pragma once

#include <engine/content.hpp>
#include <engine/material.hpp>
#include <engine/geometry.hpp>

namespace Morpheus {
	class StaticMesh : public INodeOwner {
	private:
		Geometry* mGeometry;
		Material* mMaterial;

	public:
		inline StaticMesh() : INodeOwner(NodeType::STATIC_MESH), mGeometry(nullptr), mMaterial(nullptr) {
		}

		StaticMesh* toStaticMesh() override;
		void setGeometry(Geometry* geo);
		void setMaterial(Material* mat);

		Geometry* getGeometry() {
			return mGeometry;
		}

		Material* getMaterial() {
			return mMaterial;
		}

		const Geometry* getGeometry() const {
			return mGeometry;
		}

		const Material* getMaterial() const {
			return mMaterial;
		}

		friend class ContentFactory<StaticMesh>;
	};
	SET_NODE_ENUM(StaticMesh, STATIC_MESH);

	template <>
	class ContentFactory<StaticMesh> : public IContentFactory {
	public:
		INodeOwner* load(const std::string& source, Node loadInto) override;
		void unload(INodeOwner* ref) override;

		~ContentFactory();

		std::string getContentTypeString() const override;

		StaticMesh* makeStaticMesh(Material* material, Geometry* geometry);
		StaticMesh* makeStaticMesh(Material* material, Geometry* geometry,
			const std::string& source);
	};
}