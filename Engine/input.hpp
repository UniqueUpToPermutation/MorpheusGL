#pragma once

#include <functional>
#include <set>

struct GLFWwindow;

namespace Morpheus {

	typedef std::function<void(GLFWwindow*, double, double)> f_cursor_pos_t;
	typedef std::function<void(GLFWwindow*, int, int, int)> f_mouse_button_t;
	typedef std::function<void(GLFWwindow*, int, int, int, int)> f_key_t;
	typedef std::function<void(GLFWwindow*, unsigned int)> f_char_t;
	typedef std::function<void(GLFWwindow*, int, const char**)> f_drop_t;
	typedef std::function<void(GLFWwindow*, double, double)> f_scroll_t;
	typedef std::function<void(GLFWwindow*, int, int)> f_framebuffer_size_t;

	typedef std::function<bool(GLFWwindow*, double, double)> f_cursor_pos_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int, int)> f_mouse_button_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int, int, int)> f_key_capture_t;
	typedef std::function<bool(GLFWwindow*, unsigned int)> f_char_capture_t;
	typedef std::function<bool(GLFWwindow*, int, const char**)> f_drop_capture_t;
	typedef std::function<bool(GLFWwindow*, double, double)> f_scroll_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int)> f_framebuffer_size_capture_t;

	class Input {
	private:
		std::set<const f_cursor_pos_t*> mCursorPosCallbacks;
		std::set<const f_mouse_button_t*> mMouseButtonCallbacks;
		std::set<const f_key_t*> mKeyCallbacks;
		std::set<const f_char_t*> mCharCallbacks;
		std::set<const f_drop_t*> mDropCallbacks;
		std::set<const f_scroll_t*> mScrollCallbacks;
		std::set<const f_framebuffer_size_t*> mFramebufferSizeCallbacks;

		std::set<const f_cursor_pos_capture_t*> mCursorPosCaptureCallbacks;
		std::set<const f_mouse_button_capture_t*> mMouseButtonCaptureCallbacks;
		std::set<const f_key_capture_t*> mKeyCaptureCallbacks;
		std::set<const f_char_capture_t*> mCharCaptureCallbacks;
		std::set<const f_drop_capture_t*> mDropCaptureCallbacks;
		std::set<const f_scroll_capture_t*> mScrollCaptureCallbacks;
		std::set<const f_framebuffer_size_capture_t*> mFramebufferSizeCaptureCallbacks;

		void glfwRegister();

	public:
		// Events that can be bound to
		void bindCursorPosEvent(const f_cursor_pos_t* f);
		void bindMouseButtonEvent(const f_mouse_button_t* f);
		void bindKeyEvent(const f_key_t* f);
		void bindCharEvent(const f_char_t* f);
		void bindDropEvent(const f_drop_t* f);
		void bindScrollEvent(const f_scroll_t* f);
		void bindFramebufferSizeEvent(const f_framebuffer_size_t* f);

		void unbindCursorPosEvent(const f_cursor_pos_t* f);
		void unbindMouseButtonEvent(const f_mouse_button_t* f);
		void unbindKeyEvent(const f_key_t* f);
		void unbindCharEvent(const f_char_t* f);
		void unbindDropEvent(const f_drop_t* f);
		void unbindScrollEvent(const f_scroll_t* f);
		void unbindFramebufferSizeEvent(const f_framebuffer_size_t* f);

		// Events that can be bound to
		void bindCursorPosCaptureEvent(const f_cursor_pos_capture_t* f);
		void bindMouseButtonCaptureEvent(const f_mouse_button_capture_t* f);
		void bindKeyCaptureEvent(const f_key_capture_t* f);
		void bindCharCaptureEvent(const f_char_capture_t* f);
		void bindDropCaptureEvent(const f_drop_capture_t* f);
		void bindScrollCaptureEvent(const f_scroll_capture_t* f);
		void bindFramebufferSizeCaptureEvent(const f_framebuffer_size_capture_t* f);

		void unbindCursorPosCaptureEvent(const f_cursor_pos_capture_t* f);
		void unbindMouseButtonCaptureEvent(const f_mouse_button_capture_t* f);
		void unbindKeyCaptureEvent(const f_key_capture_t* f);
		void unbindCharCaptureEvent(const f_char_capture_t* f);
		void unbindDropCaptureEvent(const f_drop_capture_t* f);
		void unbindScrollCaptureEvent(const f_scroll_capture_t* f);
		void unbindFramebufferSizeCaptureEvent(const f_framebuffer_size_capture_t* f);

		friend void cursorPosHandler(GLFWwindow* win, double x, double y);
		friend void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers);
		friend void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods);
		friend void charHandler(GLFWwindow* win, unsigned int codepoint);
		friend void dropHandler(GLFWwindow* win, int count, const char** filenames);
		friend void scrollHandler(GLFWwindow* win, double x, double y);
		friend void framebufferSizeHandler(GLFWwindow* win, int width, int height);

		friend void cursorPosCaptureHandler(GLFWwindow* win, double x, double y);
		friend void mouseButtonCaptureHandler(GLFWwindow* win, int button, int action, int modifiers);
		friend void keyCaptureHandler(GLFWwindow* win, int key, int scancode, int action, int mods);
		friend void charCaptureHandler(GLFWwindow* win, unsigned int codepoint);
		friend void dropCaptureHandler(GLFWwindow* win, int count, const char** filenames);
		friend void scrollCaptureHandler(GLFWwindow* win, double x, double y);
		friend void framebufferSizeCaptureHandler(GLFWwindow* win, int width, int height);

		friend class Engine;
	};
}