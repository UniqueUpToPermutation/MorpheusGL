#pragma once

#include "core.hpp"

#include <glm/glm.hpp>

namespace Morpheus {
	class ICamera : public IDisposable {
	public:
		virtual glm::mat4 view() const = 0;
		virtual glm::mat4 projection() const = 0;
	};
	SET_NODE_TYPE(ICamera, CAMERA);

	class PerspectiveLookAtCamera : public ICamera {
	public:
		glm::vec3 mPosition;
		glm::vec3 mLookAt;
		glm::vec3 mUp;
		uint32_t mDisplayWidth;
		uint32_t mDisplayHeight;
		float mFarPlane;
		float mNearPlane;
		float mFieldOfView;

		glm::mat4 view() const override;
		glm::mat4 projection() const override;
		void dispose() override;
	};
	SET_BASE_TYPE(PerspectiveLookAtCamera, ICamera);
}