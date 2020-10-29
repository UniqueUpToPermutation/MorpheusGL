#pragma once

#include <engine/engine.hpp>
#include <engine/blend.hpp>
#include <engine/shader.hpp>

namespace Morpheus {
	// A geometry batcher for rendering lines for debugging
	class LineBatch : public INodeOwner {
	private:
		GLuint mBatchBuffer;
		GLuint mVertexArray;


	};

	// A geometry batcher for rendering triangles for debugging
	class TriangleBatch : public INodeOwner {

	};

	// A geometry batch for rendering points for debugging
	class PointBatch : public INodeOwner {

	};

	// A combination line / triangle / point batch
	class DebugBatch : public INodeOwner { 

	};
}