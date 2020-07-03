#pragma once

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"
#include "camera.hpp"

namespace Morpheus {
	class Scene {
	private:
		NodeHandle mHandle;
		Pool<StaticTransform> mStaticTransformPool;
		Pool<DynamicTransform> mDynamicTransformPool;
		Pool<BoundingBox> mBoundingBoxPool;
		ICamera* mCamera;

	public:
		inline NodeHandle handle() const { return mHandle; }
		inline Node node() { return graph()[mHandle]; }
		inline Pool<StaticTransform>& staticTransformPool() { return mStaticTransformPool; }
		inline Pool<DynamicTransform>& dynamicTransformPool() { return mDynamicTransformPool; }
		inline Pool<BoundingBox>& boundingBoxPool() { return mBoundingBoxPool; }
		inline ICamera* getActiveCamera() { return mCamera; }
		inline void setActiveCamera(ICamera* camera) { mCamera = camera; }
	};
}