#include "input.hpp"
#include "engine.hpp"

#include <GLFW/glfw3.h>

namespace Morpheus {
	void cursorPosHandler(GLFWwindow* win, double x, double y) {
		auto input_ = input();

		for (auto f : input_->mCursorPosCaptureCallbacks) {
			auto result = (*f)(win, x, y);
			if (result)
				return;
		}

		for (auto f : input_->mCursorPosCallbacks)
			(*f)(win, x, y);
	}

	void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers) {
		auto input_ = input();

		for (auto f : input_->mMouseButtonCaptureCallbacks) {
			auto result = (*f)(win, button, action, modifiers);
			if (result)
				return;
		}

		for (auto f : input_->mMouseButtonCallbacks)
			(*f)(win, button, action, modifiers);
	}

	void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods) {
		auto input_ = input();

		for (auto f : input_->mKeyCaptureCallbacks) {
			auto result = (*f)(win, key, scancode, action, mods);
			if (result)
				return;
		}

		for (auto f : input_->mKeyCallbacks)
			(*f)(win, key, scancode, action, mods);
	}

	void charHandler(GLFWwindow* win, unsigned int codepoint) {
		auto input_ = input();

		for (auto f : input_->mCharCaptureCallbacks) {
			auto result = (*f)(win, codepoint);
			if (result)
				return;
		}

		for (auto f : input_->mCharCallbacks)
			(*f)(win, codepoint);
	}

	void dropHandler(GLFWwindow* win, int count, const char** filenames) {
		auto input_ = input();

		for (auto f : input_->mDropCaptureCallbacks) {
			auto result = (*f)(win, count, filenames);
			if (result)
				return;
		}

		for (auto f : input_->mDropCallbacks)
			(*f)(win, count, filenames);
	}

	void scrollHandler(GLFWwindow* win, double x, double y) {
		auto input_ = input();

		for (auto f : input_->mScrollCaptureCallbacks) {
			auto result = (*f)(win, x, y);
			if (result)
				return;
		}

		for (auto f : input_->mScrollCallbacks)
			(*f)(win, x, y);
	}

	void framebufferSizeHandler(GLFWwindow* win, int width, int height) {
		auto input_ = input();

		for (auto f : input_->mFramebufferSizeCaptureCallbacks) {
			auto result = (*f)(win, width, height);
			if (result)
				return;
		}

		for (auto f : input_->mFramebufferSizeCallbacks)
			(*f)(win, width, height);
	}

	void Input::glfwRegister() {
		auto window = engine()->window();

		glfwSetCursorPosCallback(window, &cursorPosHandler);
		glfwSetMouseButtonCallback(window, &mouseButtonHandler);
		glfwSetKeyCallback(window, &keyHandler);
		glfwSetCharCallback(window, &charHandler);
		glfwSetDropCallback(window, &dropHandler);
		glfwSetScrollCallback(window, &scrollHandler);
		glfwSetFramebufferSizeCallback(window, &framebufferSizeHandler);
	}

	void Input::bindCursorPosEvent(const f_cursor_pos_t* f) {
		mCursorPosCallbacks.insert(f);
	}
	void Input::bindMouseButtonEvent(const f_mouse_button_t* f) {
		mMouseButtonCallbacks.insert(f);
	}
	void Input::bindKeyEvent(const f_key_t* f) {
		mKeyCallbacks.insert(f);
	}
	void Input::bindCharEvent(const f_char_t* f) {
		mCharCallbacks.insert(f);
	}
	void Input::bindDropEvent(const f_drop_t* f) {
		mDropCallbacks.insert(f);
	}
	void Input::bindScrollEvent(const f_scroll_t* f) {
		mScrollCallbacks.insert(f);
	}
	void Input::bindFramebufferSizeEvent(const f_framebuffer_size_t* f) {
		mFramebufferSizeCallbacks.insert(f);
	}

	void Input::unbindCursorPosEvent(const f_cursor_pos_t* f) {
		mCursorPosCallbacks.erase(f);
	}
	void Input::unbindMouseButtonEvent(const f_mouse_button_t* f) {
		mMouseButtonCallbacks.erase(f);
	}
	void Input::unbindKeyEvent(const f_key_t* f) {
		mKeyCallbacks.erase(f);
	}
	void Input::unbindCharEvent(const f_char_t* f) {
		mCharCallbacks.erase(f);
	}
	void Input::unbindDropEvent(const f_drop_t* f) {
		mDropCallbacks.erase(f);
	}
	void Input::unbindScrollEvent(const f_scroll_t* f) {
		mScrollCallbacks.erase(f);
	}
	void Input::unbindFramebufferSizeEvent(const f_framebuffer_size_t* f) {
		mFramebufferSizeCallbacks.erase(f);
	}

	void Input::bindCursorPosCaptureEvent(const f_cursor_pos_capture_t* f) {
		mCursorPosCaptureCallbacks.insert(f);
	}
	void Input::bindMouseButtonCaptureEvent(const f_mouse_button_capture_t* f) {
		mMouseButtonCaptureCallbacks.insert(f);
	}
	void Input::bindKeyCaptureEvent(const f_key_capture_t* f) {
		mKeyCaptureCallbacks.insert(f);
	}
	void Input::bindCharCaptureEvent(const f_char_capture_t* f) {
		mCharCaptureCallbacks.insert(f);
	}
	void Input::bindDropCaptureEvent(const f_drop_capture_t* f) {
		mDropCaptureCallbacks.insert(f);
	}
	void Input::bindScrollCaptureEvent(const f_scroll_capture_t* f) {
		mScrollCaptureCallbacks.insert(f);
	}
	void Input::bindFramebufferSizeCaptureEvent(const f_framebuffer_size_capture_t* f) {
		mFramebufferSizeCaptureCallbacks.insert(f);
	}

	void Input::unbindCursorPosCaptureEvent(const f_cursor_pos_capture_t* f) {
		mCursorPosCaptureCallbacks.erase(f);
	}
	void Input::unbindMouseButtonCaptureEvent(const f_mouse_button_capture_t* f) {
		mMouseButtonCaptureCallbacks.erase(f);
	}
	void Input::unbindKeyCaptureEvent(const f_key_capture_t* f) {
		mKeyCaptureCallbacks.erase(f);
	}
	void Input::unbindCharCaptureEvent(const f_char_capture_t* f) {
		mCharCaptureCallbacks.erase(f);
	}
	void Input::unbindDropCaptureEvent(const f_drop_capture_t* f) {
		mDropCaptureCallbacks.erase(f);
	}
	void Input::unbindScrollCaptureEvent(const f_scroll_capture_t* f) {
		mScrollCaptureCallbacks.erase(f);
	}
	void Input::unbindFramebufferSizeCaptureEvent(const f_framebuffer_size_capture_t* f) {
		mFramebufferSizeCaptureCallbacks.erase(f);
	}
}