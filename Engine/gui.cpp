#include "gui.hpp"

#include <glad/glad.h>
#include <nanogui/nanogui.h>

namespace Morpheus {

	void GuiBase::init(Node& node)
	{
		auto input_ = input();
		auto engine_ = engine();

		mScreen = new nanogui::Screen();
		mScreen->initialize(engine_->window(), false);

		mCursorPosHandler = [this](GLFWwindow*, double x, double y) {
			return mScreen->cursorPosCallbackEvent(x, y);
		};

		mMouseButtonHandler = [this](GLFWwindow*, int button, int action, int modifiers) {
			return mScreen->mouseButtonCallbackEvent(button, action, modifiers);
		};

		mKeyHandler = [this](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			return mScreen->keyCallbackEvent(key, scancode, action, modifiers);
		};

		mCharHandler = [this](GLFWwindow*, unsigned int codepoint) {
			return mScreen->charCallbackEvent(codepoint);
		};

		mDropHandler = [this](GLFWwindow*, int count, const char** filenames) {
			return mScreen->dropCallbackEvent(count, filenames);
		};

		mScrollHandler = [this](GLFWwindow*, double x, double y) {
			return mScreen->scrollCallbackEvent(x, y);
		};

		mFramebufferSizeHandler = [this](GLFWwindow*, int width, int height) {
			return mScreen->resizeCallbackEvent(width, height);
		};

		input_->bindCursorPosCaptureEvent(&mCursorPosHandler);
		input_->bindMouseButtonCaptureEvent(&mMouseButtonHandler);
		input_->bindKeyCaptureEvent(&mKeyHandler);
		input_->bindCharCaptureEvent(&mCharHandler);
		input_->bindDropCaptureEvent(&mDropHandler);
		input_->bindScrollCaptureEvent(&mScrollHandler);
		input_->bindFramebufferSizeCaptureEvent(&mFramebufferSizeHandler);

		initGui();
	}

	void GuiBase::dispose() {
		auto input_ = input();

		input_->unbindCursorPosCaptureEvent(&mCursorPosHandler);
		input_->unbindMouseButtonCaptureEvent(&mMouseButtonHandler);
		input_->unbindKeyCaptureEvent(&mKeyHandler);
		input_->unbindCharCaptureEvent(&mCharHandler);
		input_->unbindDropCaptureEvent(&mDropHandler);
		input_->unbindScrollCaptureEvent(&mScrollHandler);
		input_->unbindFramebufferSizeCaptureEvent(&mFramebufferSizeHandler);

		delete this;
	}
}