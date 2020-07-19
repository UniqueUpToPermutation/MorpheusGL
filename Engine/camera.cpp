#include "camera.hpp"
#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace Morpheus {

	Camera::Camera() :
		mPosition(vec3(0.0f, 0.0f, -1.0f)),
		mLookAt(zero<vec3>()),
		mUp(vec3(0.0f, 1.0f, 0.0f)),
		mFarPlane(100.0f),
		mNearPlane(0.1f),
		mFieldOfView(pi<float>() / 4.0f),
		mType(CameraType::PERSPECTIVE_LOOK_AT)
	{
		auto displayParams = engine()->displayParams();
		mDisplayWidth = displayParams.mFramebufferWidth;
		mDisplayHeight = displayParams.mFramebufferHeight;

		mResizeHandler = [this](GLFWwindow*, int width, int height) {
			mDisplayWidth = width;
			mDisplayHeight = height;
		};
		input()->bindFramebufferSizeEvent(&mResizeHandler);
	}

	glm::mat4 Camera::view() const {
		return glm::lookAt(mPosition, mLookAt, mUp);
	}
	glm::mat4 Camera::projection() const {
		return glm::perspectiveFov(mFieldOfView, (float)mDisplayWidth, (float)mDisplayHeight, mNearPlane, mFarPlane);
	}
	glm::vec3 Camera::eye() const {
		return mPosition;
	}
	void Camera::dispose()
	{
		input()->unbindFramebufferSizeEvent(&mResizeHandler);
		delete this;
	}
}