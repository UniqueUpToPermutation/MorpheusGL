#pragma once

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"
#include "camera.hpp"

namespace Morpheus {
	class Scene {
	private:
		NodeHandle mHandle;
		Pool<Transform> mTransformPool;
		Pool<BoundingBox> mBoundingBoxPool;
		ICamera* mCamera;

	public:
		inline NodeHandle handle() const { return mHandle; }
		inline Node node() { return (*graph())[mHandle]; }
		inline Pool<Transform>& transformPool() { return mTransformPool; }
		inline Pool<BoundingBox>& boundingBoxPool() { return mBoundingBoxPool; }
		inline ICamera* getActiveCamera() { return mCamera; }
		inline void setActiveCamera(ICamera* camera) { mCamera = camera; }
	};
}