#include <engine/camera.hpp>
#include <engine/engine.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

using namespace glm;

namespace Morpheus {

	Camera* Camera::toCamera() {
		return this;
	}

	Camera::Camera() :
		INodeOwner(NodeType::CAMERA),
		mPosition(vec3(0.0f, 0.0f, -1.0f)),
		mLookAt(zero<vec3>()),
		mUp(vec3(0.0f, 1.0f, 0.0f)),
		mFarPlane(100.0f),
		mNearPlane(0.1f),
		mFieldOfView(pi<float>() / 4.0f),
		mCameraType(CameraType::PERSPECTIVE_LOOK_AT)
	{
	}

	glm::mat4 Camera::view() const {
		return glm::lookAt(mPosition, mLookAt, mUp);
	}
	glm::mat4 Camera::projection() const {
		int width;
		int height;
		glfwGetFramebufferSize(window(), &width, &height);

		return glm::perspectiveFov(mFieldOfView, (float)width, (float)height, mNearPlane, mFarPlane);
	}
	glm::vec3 Camera::eye() const {
		return mPosition;
	}
}