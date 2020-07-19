#include "cameracontroller.hpp"

namespace Morpheus {
	LookAtCameraController::LookAtCameraController(double distance) : bEnabled(true), mTheta(0.0),
		mPhi(0.0), mLookAt(zero<vec3>()), mDistance(distance), bM1Captured(false),
		bM2Captured(false), mThetaSpeed(0.1), mPhiSpeed(0.1) {
	}

	bool LookAtCameraController::isEnabled() const {
		return bEnabled;
	}

	void LookAtCameraController::setEnabled(const bool value) {
		bEnabled = value;
	}

	void LookAtCameraController::update(const double dt) {
	}

	void LookAtCameraController::applyTo(Camera* camera) {
		camera->mUp = vec3(0.0f, 1.0f, 0.0f);
		camera->mLookAt = mLookAt;
		vec3 dir(cos(mPhi) * cos(mTheta), sin(mTheta), sin(mPhi) * cos(mTheta));
		dir *= mDistance;
		camera->mPosition = mLookAt + dir;
	}

	void LookAtCameraController::init(Node& node) {
		auto parentDesc = desc(node.parents()());
		assert(parentDesc->type == NodeType::CAMERA);
		mCamera = parentDesc->owner.reinterpretGet<Camera>();

		mMouseHandler = [this](GLFWwindow* window, int scancode, int action, int modifiers) {
			if (bEnabled) {
				switch (scancode) {
				case GLFW_MOUSE_BUTTON_1:
					if (action == GLFW_PRESS) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
						bM1Captured = true;
						glfwGetCursorPos(window, &mLastPosX, &mLastPosY);
					}
					else if (action == GLFW_RELEASE) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
						bM1Captured = false;
					}
					break;
				case GLFW_MOUSE_BUTTON_2:
					if (action == GLFW_PRESS) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
						bM2Captured = true;
						glfwGetCursorPos(window, &mLastPosX, &mLastPosY);
					}
					else if (action == GLFW_RELEASE) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
						bM2Captured = false;
					}
					break;
				}
			}
		};

		mCursorPos = [this](GLFWwindow* window, double x, double y) {
			if (bEnabled) {
				if (bM1Captured) {
					mPhi += (x - mLastPosX) * mPhiSpeed;
					mTheta += (y - mLastPosY) * mThetaSpeed;
					applyTo(mCamera);
				}
				else if (bM2Captured) {

				}
			}
		};

		input()->bindMouseButtonEvent(&mMouseHandler);

		applyTo(mCamera);
	}

	void LookAtCameraController::dispose() {
		input()->unbindMouseButtonEvent(&mMouseHandler);
		delete this;
	}
}