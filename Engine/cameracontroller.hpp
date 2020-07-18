#pragma once

#include "core.hpp"
#include "engine.hpp"
#include "camera.hpp"

#include <GLFW/glfw3.h>

namespace Morpheus {
	class LookAtCameraController : public ILogic {
	private:
		bool bEnabled;
		PerspectiveLookAtCamera* mCamera;
		f_mouse_button_t mMouseHandler;

	public:
		LookAtCameraController() : bEnabled(true) {
		}

		bool isEnabled() const override {
			return bEnabled;
		}

		void setEnabled(const bool value) override {
			bEnabled = value;
		}

		void update(const double dt) override {

		}

		void init(Node& node) override {
			mMouseHandler = [this](GLFWwindow* window, int scancode, int action, int modifiers) {
				switch (scancode) {
				case GLFW_MOUSE_BUTTON_1:

					if (action == GLFW_PRESS)
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					else if (action == GLFW_RELEASE)
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

					break;
				case GLFW_MOUSE_BUTTON_2:

					if (action == GLFW_PRESS)
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					else if (action == GLFW_RELEASE)
						glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

					break;
				}
			};
			engine()->bindMouseButtonEvent(&mMouseHandler);
		}

		void dispose() override {
			engine()->unbindMouseButtonEvent(&mMouseHandler);
			delete this;
		}
	};
	SET_BASE_TYPE(LookAtCameraController, ILogic);
}