#include "gui.h"

#include <glad/glad.h>

#include <nanogui/nanogui.h>

namespace Morpheus {
	void GuiBase::init() {
		auto en = engine();

		mScreen = new nanogui::Screen();
		mScreen->initialize(en.window(), false);

		mCursorPosHandler = [this](GLFWwindow*, double x, double y) {
			mScreen->cursorPosCallbackEvent(x, y);
		};

		mMouseButtonHandler = [this](GLFWwindow*, int button, int action, int modifiers) {
			mScreen->mouseButtonCallbackEvent(button, action, modifiers);
		};

		mKeyHandler = [this](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			mScreen->keyCallbackEvent(key, scancode, action, modifiers);
		};

		mCharHandler = [this](GLFWwindow*, unsigned int codepoint) {
			mScreen->charCallbackEvent(codepoint);
		};

		mDropHandler = [this](GLFWwindow*, int count, const char** filenames) {
			mScreen->dropCallbackEvent(count, filenames);
		};

		mScrollHandler = [this](GLFWwindow*, double x, double y) {
			mScreen->scrollCallbackEvent(x, y);
		};

		mFramebufferSizeHandler = [this](GLFWwindow*, int width, int height) {
			mScreen->resizeCallbackEvent(width, height);
		};

		en.bindCursorPosEvent(&mCursorPosHandler);
		en.bindMouseButtonEvent(&mMouseButtonHandler);
		en.bindKeyEvent(&mKeyHandler);
		en.bindCharEvent(&mCharHandler);
		en.bindDropEvent(&mDropHandler);
		en.bindScrollEvent(&mScrollHandler);
		en.bindFramebufferSizeEvent(&mFramebufferSizeHandler);

		initGui();
	}

	void GuiBase::dispose() {
		auto en = engine();

		en.unbindCursorPosEvent(&mCursorPosHandler);
		en.unbindMouseButtonEvent(&mMouseButtonHandler);
		en.unbindKeyEvent(&mKeyHandler);
		en.unbindCharEvent(&mCharHandler);
		en.unbindDropEvent(&mDropHandler);
		en.unbindScrollEvent(&mScrollHandler);
		en.unbindFramebufferSizeEvent(&mFramebufferSizeHandler);

		delete mScreen;
		delete this;
	}
}