#pragma once

#include <engine/core.hpp>
#include <engine/input.hpp>

#include <glm/glm.hpp>

namespace Morpheus {
	enum class CameraType {
		PERSPECTIVE_LOOK_AT
	};

	class Camera : public IDisposable {
	private:
		f_framebuffer_size_capture_t mResizeHandler;
		uint32_t mDisplayWidth;
		uint32_t mDisplayHeight;

	public:
		Camera();

		CameraType mType;

		glm::vec3 mPosition;
		glm::vec3 mLookAt;
		glm::vec3 mUp;

		float mFarPlane;
		float mNearPlane;
		float mFieldOfView;

		glm::mat4 view() const;
		glm::mat4 projection() const;
		glm::vec3 eye() const;

		void dispose() override;
	};
	SET_NODE_ENUM(Camera, CAMERA);
}