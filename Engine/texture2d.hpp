#pragma once

#include <glad/glad.h>
#include <iostream>

#include "content.hpp"

namespace Morpheus {
	class Texture2D {
	private:
		GLuint mId;

	public:
		inline GLuint id() const { return mId; }

		friend class ContentFactory<Texture2D>;
	};
	SET_NODE_ENUM(Texture2D, TEXTURE_2D);

	class ContentFactory<Texture2D> : public IContentFactory {
	public:
		ref<void> load(const std::string& source, Node& loadInto) override;
		void unload(ref<void>& ref) override;
		void dispose() override;
	};
}