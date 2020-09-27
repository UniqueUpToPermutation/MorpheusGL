#pragma once

#include <engine/core.hpp>
#include <engine/pool.hpp>
#include <engine/engine.hpp>
#include <engine/camera.hpp>

namespace Morpheus {
	class Scene : public INodeOwner {
	private:;
		Camera* mCamera;

	public:
		inline Scene() : INodeOwner(NodeType::SCENE_ROOT), mCamera(nullptr) {
		}

		Scene* toScene() override;

		inline Camera* getActiveCamera() { return mCamera; }
		inline void setActiveCamera(Camera* camera) { mCamera = camera; }
	};
	SET_NODE_ENUM(Scene, SCENE_ROOT);
}