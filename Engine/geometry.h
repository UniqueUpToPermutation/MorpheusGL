#pragma once

#include "content.hpp"

#include <glad/glad.h>

namespace Assimp {
	class Importer;
}

namespace Morpheus {

	class Geometry {
	private:
		GLuint vao;
		GLuint vbo;
		GLuint ibo;
		BoundingBox aabb;

	public:
		inline GLuint vertexArray() const { return vao; }
		inline GLuint vertexBuffer() const { return vbo; }
		inline GLuint indexBuffer() const { return ibo; }
		inline BoundingBox boundingBox() const { return aabb; }

		friend class ContentFactory<Geometry>;
	};
	REGISTER_NODE_TYPE(Geometry, NodeType::GEOMETRY);
	REGISTER_CONTENT_BASE_TYPE(Geometry, Geometry);

	template <>
	class ContentFactory<Geometry> : public IContentFactory {
	private:
		Assimp::Importer* importer;

	public:
		ContentFactory();
		~ContentFactory();

		ref<void> load(const std::string& source) override;
		void unload(ref<void>& ref) override;
	};
}