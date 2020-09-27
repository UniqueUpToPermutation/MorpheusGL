#pragma once

#include <engine/core.hpp>
#include <engine/engine.hpp>
#include <engine/camera.hpp>

using namespace glm;

struct GLFWwindow;

namespace Morpheus {
	enum class CameraMoveMode {
		NONE,
		ROTATE,
		PAN,
		DOLLY,
		ZOOM
	};

	enum class CameraControllerMouseInput {
		NONE,
		LEFT_MOUSE,
		RIGHT_MOUSE,
		LEFT_RIGHT_MOUSE
	};

	class LookAtCameraController : public Entity {
	private:
		bool bEnabled;
		Camera* mCamera;
		f_mouse_button_capture_t mMouseHandler;
		f_cursor_pos_capture_t mCursorPosHandler;
		f_scroll_capture_t mScrollHandler;
		CameraMoveMode mMoveMode;
		std::map<CameraControllerMouseInput, CameraMoveMode> mInputToMode;

		double mLastPosX;
		double mLastPosY;

		double mTheta;
		double mPhi;
		double mDistance;
		vec3 mLookAt;
		vec3 mPanX;
		vec3 mPanY;
		vec3 mDollyDir;
		bool bM1Captured;
		bool bM2Captured;
		double mThetaSpeed;
		double mPhiSpeed;
		double mDollySpeed;
		double mZoomSpeed;
		double mPanSpeed;
		bool bIgnoreMoveEvent;

		CameraControllerMouseInput getCurrentInput();
		void doPan(double dx, double dy);
		void doDolly(double dx, double dy);
		void doRotate(double dx, double dy);
		void doZoom(double dx, double dy);
		void beginPan(GLFWwindow*);
		void endPan(GLFWwindow*);
		void beginDolly(GLFWwindow*);
		void endDolly(GLFWwindow*);
		void beginRotate(GLFWwindow*);
		void endRotate(GLFWwindow*);
		void beginZoom(GLFWwindow*);
		void endZoom(GLFWwindow*);
		void onInputStateChanged(GLFWwindow*);

		vec3 getLookDirection() const;

	public:
		~LookAtCameraController() override;

		inline double& thetaRotateSpeed() { return mThetaSpeed; }
		inline double& phiRotateSpeed() { return mPhiSpeed; }
		inline double& dollySpeed() { return mDollySpeed; }
		inline double& panSpeed() { return mPanSpeed; }
		inline double& zoomSpeed() { return mZoomSpeed; }
		inline double getTheta() { return mTheta; }
		inline double getPhi() { return mPhi; }
		inline void setTheta(double value) { mTheta = value; if (mCamera) applyTo(mCamera); }
		inline void setPhi(double value) { mPhi = value; if (mCamera) applyTo(mCamera); }

		LookAtCameraController(double distance = 1.0);
		void reset(double distance = 1.0);

		bool isEnabled() const override;
		void setEnabled(const bool value) override;
		void update(const double dt) override;
		void applyTo(Camera* camera);
		void init() override;
	};
	SET_BASE_TYPE(LookAtCameraController, ILogic);
}