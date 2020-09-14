#include <engine/cameracontroller.hpp>
#include <iostream>

#include <GLFW/glfw3.h>

using namespace std;

namespace Morpheus {
	LookAtCameraController::LookAtCameraController(double distance) : bM1Captured(false), bM2Captured(false),
		mMoveMode(CameraMoveMode::NONE), mCamera(nullptr) {
		reset(distance);

		mInputToMode[CameraControllerMouseInput::NONE] = CameraMoveMode::NONE;
		mInputToMode[CameraControllerMouseInput::LEFT_MOUSE] = CameraMoveMode::ROTATE;
		mInputToMode[CameraControllerMouseInput::RIGHT_MOUSE] = CameraMoveMode::PAN;
		mInputToMode[CameraControllerMouseInput::LEFT_RIGHT_MOUSE] = CameraMoveMode::ZOOM;
	}

	void LookAtCameraController::reset(double distance) {
		bEnabled = true;
		mTheta = 0.0;
		mPhi = 0.0; 
		mLookAt = zero<vec3>(); 
		mDistance = distance;
		mThetaSpeed = 0.005;
		mPhiSpeed = 0.007;
		mDollySpeed = distance * 0.002;
		mPanSpeed = distance * 0.002;
		mZoomSpeed = 0.005;
		if (mCamera)
			applyTo(mCamera);
	}

	bool LookAtCameraController::isEnabled() const {
		return bEnabled;
	}

	void LookAtCameraController::setEnabled(const bool value) {
		bEnabled = value;
	}

	void LookAtCameraController::update(const double dt) {
	}

	vec3 LookAtCameraController::getLookDirection() const {
		vec3 dir(cos(mPhi) * cos(mTheta), sin(mTheta), sin(mPhi) * cos(mTheta));
		dir *= mDistance;
		return dir;
	}

	void LookAtCameraController::applyTo(Camera* camera) {
		camera->mUp = vec3(0.0f, 1.0f, 0.0f);
		camera->mLookAt = mLookAt;
		camera->mPosition = mLookAt - getLookDirection();
	}

	void LookAtCameraController::beginPan(GLFWwindow* window) {
		glfwGetCursorPos(window, &mLastPosX, &mLastPosY);

		vec3 dir = getLookDirection();
		mPanX = cross(mCamera->mUp, dir);
		mPanY = cross(mPanX, dir);
		mPanX = normalize(mPanX);
		mPanY = normalize(mPanY);

		bIgnoreMoveEvent = true;
	}

	void LookAtCameraController::endPan(GLFWwindow* window) {

	}

	void LookAtCameraController::beginDolly(GLFWwindow* window) {
		glfwGetCursorPos(window, &mLastPosX, &mLastPosY);
		bIgnoreMoveEvent = true;
		mDollyDir = normalize(getLookDirection());
	}

	void LookAtCameraController::endDolly(GLFWwindow* window) {

	}

	void LookAtCameraController::beginRotate(GLFWwindow* window) {
		glfwGetCursorPos(window, &mLastPosX, &mLastPosY);
		bIgnoreMoveEvent = true;
	}

	void LookAtCameraController::endRotate(GLFWwindow* window) {

	}

	void LookAtCameraController::beginZoom(GLFWwindow* window) {
		glfwGetCursorPos(window, &mLastPosX, &mLastPosY);
		bIgnoreMoveEvent = true;
	}

	void LookAtCameraController::endZoom(GLFWwindow* window) {

	}

	CameraControllerMouseInput LookAtCameraController::getCurrentInput()
	{
		if (bM1Captured) {
			if (bM2Captured) {
				return CameraControllerMouseInput::LEFT_RIGHT_MOUSE;
			}
			else {
				return CameraControllerMouseInput::LEFT_MOUSE;
			}
		}
		else {
			if (bM2Captured) {
				return CameraControllerMouseInput::RIGHT_MOUSE;
			}
			else {
				return CameraControllerMouseInput::NONE;
			}
		}
	}

	void LookAtCameraController::doPan(double dx, double dy) {
		mLookAt += mPanX * (float)dx * (float)mPanSpeed;
		mLookAt -= mPanY * (float)dy * (float)mPanSpeed;

		applyTo(mCamera);
	}

	void LookAtCameraController::doDolly(double dx, double dy) {
		mLookAt += mDollyDir * (float)dy * (float)mDollySpeed;
		applyTo(mCamera);
	}

	void LookAtCameraController::doRotate(double dx, double dy) {

		mPhi += dx * mPhiSpeed;
		mTheta -= dy * mThetaSpeed;

		mTheta = std::min<double>(glm::pi<double>() / 2.5, mTheta);
		mTheta = std::max<double>(-glm::pi<double>() / 2.5, mTheta);

		applyTo(mCamera);
	}

	void LookAtCameraController::doZoom(double dx, double dy) {
		mDistance = exp(log(mDistance) + dy * mZoomSpeed);

		applyTo(mCamera);
	}

	void LookAtCameraController::onInputStateChanged(GLFWwindow* window) {
		CameraMoveMode newMode = CameraMoveMode::NONE;

		newMode = mInputToMode[getCurrentInput()];

		if (newMode != mMoveMode) {
			switch (mMoveMode) {
			case CameraMoveMode::ROTATE:
				endRotate(window);
				break;
			case CameraMoveMode::PAN:
				endPan(window);
				break;
			case CameraMoveMode::DOLLY:
				endDolly(window);
				break;
			case CameraMoveMode::ZOOM:
				endZoom(window);
				break;
			default:
				break;
			}

			switch (newMode) {
			case CameraMoveMode::ROTATE:
				beginRotate(window);
				break;
			case CameraMoveMode::PAN:
				beginPan(window);
				break;
			case CameraMoveMode::DOLLY:
				beginDolly(window);
				break;
			case CameraMoveMode::ZOOM:
				beginZoom(window);
				break;
			default:
				break;
			}
		}

		if (newMode != CameraMoveMode::NONE && mMoveMode == CameraMoveMode::NONE)
			input()->grab(this);
		if (newMode == CameraMoveMode::NONE && mMoveMode != CameraMoveMode::NONE)
			input()->ungrab(this);

		mMoveMode = newMode;
	}

	void LookAtCameraController::init(Node node) {
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
					}
					else if (action == GLFW_RELEASE) {
						if (!bM2Captured)
							glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
						bM1Captured = false;
					}
					onInputStateChanged(window);
					break;
				case GLFW_MOUSE_BUTTON_2:
					if (action == GLFW_PRESS) {
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
						bM2Captured = true;
					}
					else if (action == GLFW_RELEASE) {
						if (!bM1Captured)
							glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
						bM2Captured = false;
					}
					onInputStateChanged(window);
					break;
				}

				return (mMoveMode != CameraMoveMode::NONE);
			}
			return false;
		};

		mCursorPosHandler = [this](GLFWwindow* window, double x, double y) {
			if (bEnabled) {
				if (mMoveMode != CameraMoveMode::NONE) {
					if (!bIgnoreMoveEvent) {
						double dx = x - mLastPosX;
						double dy = y - mLastPosY;

						switch (mMoveMode) {
						case CameraMoveMode::DOLLY:
							doDolly(dx, dy);
							break;
						case CameraMoveMode::ZOOM:
							doZoom(dx, dy);
							break;
						case CameraMoveMode::PAN:
							doPan(dx, dy);
							break;
						case CameraMoveMode::ROTATE:
							doRotate(dx, dy);
							break;
						default:
							break;
						}
					}
					else
						bIgnoreMoveEvent = false;

					mLastPosX = x;
					mLastPosY = y;
					return true;
				}
			}
			return false;
		};

		mScrollHandler = [this](GLFWwindow* window, double dx, double dy) {
			return false;
		};

		auto input_ = input();
		input_->bindMouseButtonEvent(this, &mMouseHandler);
		input_->bindCursorPosEvent(this, &mCursorPosHandler);
		input_->bindScrollEvent(this, &mScrollHandler);

		applyTo(mCamera);
	}

	void LookAtCameraController::dispose() {
		input()->unregisterTarget(this);
		delete this;
	}
}