#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Morpheus {
	glm::mat4 PerspectiveLookAtCamera::view() const {
		return glm::lookAt(mPosition, mLookAt, mUp);
	}
	glm::mat4 PerspectiveLookAtCamera::projection() const {
		return glm::perspectiveFov(mFieldOfView, (float)mDisplayWidth, (float)mDisplayHeight, mNearPlane, mFarPlane);
	}
	void PerspectiveLookAtCamera::dispose()
	{
		delete this;
	}
}