#pragma once

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"
#include "camera.hpp"

namespace Morpheus {
	class Scene : public IDisposable {
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

		void dispose() override;

		inline Node makeTransform(ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			if (out)
				*out = trans;
			return node;
		}

		Node makeIdentityTransform(ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			trans->mCache = glm::identity<glm::fmat4>();
			trans->mRotation = glm::identity<glm::quat>();
			trans->mScale = glm::one<glm::vec3>();
			trans->mTranslation = glm::zero<glm::vec3>();
			if (out)
				*out = trans;
			return node;
		}
	};
	SET_NODE_TYPE(Scene, SCENE_ROOT);
}