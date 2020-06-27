#pragma once

#include "content.hpp"

#include <glad/glad.h>

namespace Assimp {
	class Importer;
}

namespace Morpheus {

	class Geometry {
	private:
		GLuint mVao;
		GLuint mVbo;
		GLuint mIbo;
		BoundingBox mAabb;

	public:
		inline GLuint vertexArray() const { return mVao; }
		inline GLuint vertexBuffer() const { return mVbo; }
		inline GLuint indexBuffer() const { return mIbo; }
		inline BoundingBox boundingBox() const { return mAabb; }

		friend class ContentFactory<Geometry>;
	};
	REGISTER_NODE_TYPE(Geometry, NodeType::GEOMETRY);
	REGISTER_CONTENT_BASE_TYPE(Geometry, Geometry);

	template <>
	class ContentFactory<Geometry> : public IContentFactory {
	private:
		Assimp::Importer* mImporter;

	public:
		ContentFactory();
		~ContentFactory();

		ref<void> load(const std::string& source) override;
		void unload(ref<void>& ref) override;
		void dispose() override;
	};
}