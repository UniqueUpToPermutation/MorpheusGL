#include "camera.hpp"
#include "engine.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace Morpheus {
	PerspectiveLookAtCamera::PerspectiveLookAtCamera() :
		mPosition(vec3(0.0f, 0.0f, -1.0f)),
		mLookAt(zero<vec3>()),
		mUp(vec3(0.0f, 1.0f, 0.0f)),
		mFarPlane(100.0f),
		mNearPlane(0.1f),
		mFieldOfView(pi<float>() / 4.0f)
	{
		auto displayParams = engine()->displayParams();
		mDisplayWidth = displayParams.mFramebufferWidth;
		mDisplayHeight = displayParams.mFramebufferHeight;

		mResizeHandler = [this](GLFWwindow*, int width, int height) {
			mDisplayWidth = width;
			mDisplayHeight = height;
		};
		engine()->bindFramebufferSizeEvent(&mResizeHandler);
	}

	glm::mat4 PerspectiveLookAtCamera::view() const {
		return glm::lookAt(mPosition, mLookAt, mUp);
	}
	glm::mat4 PerspectiveLookAtCamera::projection() const {
		return glm::perspectiveFov(mFieldOfView, (float)mDisplayWidth, (float)mDisplayHeight, mNearPlane, mFarPlane);
	}
	glm::vec3 PerspectiveLookAtCamera::eye() const {
		return mPosition;
	}
	void PerspectiveLookAtCamera::dispose()
	{
		engine()->unbindFramebufferSizeEvent(&mResizeHandler);
		delete this;
	}
}