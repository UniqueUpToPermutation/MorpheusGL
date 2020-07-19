#pragma once

#include "core.hpp"
#include "engine.hpp"
#include "camera.hpp"

#include <GLFW/glfw3.h>

using namespace glm;

namespace Morpheus {
	class LookAtCameraController : public ILogic {
	private:
		bool bEnabled;
		Camera* mCamera;
		f_mouse_button_t mMouseHandler;
		f_cursor_pos_t mCursorPos;

		double mLastPosX;
		double mLastPosY;

		double mTheta;
		double mPhi;
		double mDistance;
		vec3 mLookAt;
		bool bM1Captured;
		bool bM2Captured;
		double mThetaSpeed;
		double mPhiSpeed;

	public:

		double& thetaSpeed() { return mThetaSpeed; }
		double& phiSpeed() { return mPhiSpeed; }

		LookAtCameraController(double distance);

		bool isEnabled() const override;
		void setEnabled(const bool value) override;
		void update(const double dt) override;
		void applyTo(Camera* camera);
		void init(Node& node) override;
		void dispose() override;
	};
	SET_BASE_TYPE(LookAtCameraController, ILogic);
}