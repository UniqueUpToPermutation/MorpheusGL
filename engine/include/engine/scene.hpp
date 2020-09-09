#pragma once

#include <engine/core.hpp>
#include <engine/pool.hpp>
#include <engine/engine.hpp>
#include <engine/camera.hpp>

namespace Morpheus {
	class Scene : public IDisposable {
	private:
		NodeHandle mHandle;
		Pool<Transform> mTransformPool;
		Pool<BoundingBox> mBoundingBoxPool;
		Camera* mCamera;

	public:
		inline NodeHandle handle() const { return mHandle; }
		inline Node node() { return (*graph())[mHandle]; }
		inline Pool<Transform>& transformPool() { return mTransformPool; }
		inline Pool<BoundingBox>& boundingBoxPool() { return mBoundingBoxPool; }
		inline Camera* getActiveCamera() { return mCamera; }
		inline void setActiveCamera(Camera* camera) { mCamera = camera; }

		void dispose() override;

		inline Node makeTransform(ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			if (out)
				*out = trans;
			return node;
		}

		inline Node makeIdentityTransform(ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			trans->mRotation = glm::identity<glm::quat>();
			trans->mScale = glm::one<glm::vec3>();
			trans->mTranslation = glm::zero<glm::vec3>();
			if (out)
				*out = trans;
			return node;
		}

		inline Node makeTranslation(const glm::vec3& translate, ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			trans->mRotation = glm::identity<glm::quat>();
			trans->mScale = glm::one<glm::vec3>();
			trans->mTranslation = translate;
			if (out)
				*out = trans;
			return node;
		}

		inline Node makeRotation(const glm::quat& rotate, ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			trans->mRotation = rotate;
			trans->mScale = glm::one<glm::vec3>();
			trans->mTranslation = glm::zero<glm::vec3>();
			if (out)
				*out = trans;
			return node;
		}

		inline Node makeScale(const glm::vec3& scale, ref<Transform>* out = nullptr) {
			ref<Transform> trans(mTransformPool.alloc());
			auto node = graph()->addNode(trans);
			trans->mRotation = glm::identity<glm::quat>();
			trans->mScale = scale;
			trans->mTranslation = glm::zero<glm::vec3>();
			if (out)
				*out = trans;
			return node;
		}
	};
	SET_NODE_ENUM(Scene, SCENE_ROOT);
}