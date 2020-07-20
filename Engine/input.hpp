#pragma once

#include <functional>
#include <set>

struct GLFWwindow;

namespace Morpheus {

	typedef std::function<bool(GLFWwindow*, double, double)> f_cursor_pos_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int, int)> f_mouse_button_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int, int, int)> f_key_capture_t;
	typedef std::function<bool(GLFWwindow*, unsigned int)> f_char_capture_t;
	typedef std::function<bool(GLFWwindow*, int, const char**)> f_drop_capture_t;
	typedef std::function<bool(GLFWwindow*, double, double)> f_scroll_capture_t;
	typedef std::function<bool(GLFWwindow*, int, int)> f_framebuffer_size_capture_t;

	enum class InputPriority {
		LOW = 0,
		NORMAL = 1,
		ELEVATED = 2,
		GUI = 3,
		GRABBED = 4,
		CRITICAL = 5
	};

	struct InputTarget {
		void* mOwner;
		InputPriority mPriority;
		InputPriority mDefaultPriority;
		mutable const f_cursor_pos_capture_t* mCursorPosCallback;
		mutable const f_mouse_button_capture_t* mMouseButtonCallback;
		mutable const f_key_capture_t* mKeyCallback;
		mutable const f_char_capture_t* mCharCallback;
		mutable const f_drop_capture_t* mDropCallback;
		mutable const f_scroll_capture_t* mScrollCallback;
		mutable const f_framebuffer_size_capture_t* mFramebufferSizeCallback;
	};

	struct InputTargetCompare {
		bool operator() (const InputTarget& lhs, const InputTarget& rhs) const {
			return lhs.mPriority > rhs.mPriority;
		}
	};

	class Input {
	private:
		bool bInEvent;
		std::vector<void*> mNewGrabers;
		std::vector<void*> mUngrabers;
		std::unordered_map<void*, std::set<InputTarget, InputTargetCompare>::iterator> mOwnerToTargetMap;
		std::set<InputTarget, InputTargetCompare> mTargets;

		void glfwRegister();
		void glfwUnregster();

		void actuallyGrab(void* owner);
		void actuallyUngrab(void* owner);
		void resolveGrabs();

	public:
		Input(); 

		// Target registration
		void registerTarget(void* owner, InputPriority priority = InputPriority::NORMAL);
		void unregisterTarget(void* owner);
		void grab(void* owner);
		void ungrab(void* owner);

		// Events that can be bound to
		void bindCursorPosEvent(void* owner, const f_cursor_pos_capture_t* f);
		void bindMouseButtonEvent(void* owner, const f_mouse_button_capture_t* f);
		void bindKeyEvent(void* owner, const f_key_capture_t* f);
		void bindCharEvent(void* owner, const f_char_capture_t* f);
		void bindDropEvent(void* owner, const f_drop_capture_t* f);
		void bindScrollEvent(void* owner, const f_scroll_capture_t* f);
		void bindFramebufferSizeEvent(void* owner, const f_framebuffer_size_capture_t* f);

		void unbindCursorPosEvent(void* owner);
		void unbindMouseButtonEvent(void* owner);
		void unbindKeyEvent(void* owner);
		void unbindCharEvent(void* owner);
		void unbindDropEvent(void* owner);
		void unbindScrollEvent(void* owner);
		void unbindFramebufferSizeEvent(void* owner);

		friend void cursorPosHandler(GLFWwindow* win, double x, double y);
		friend void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers);
		friend void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods);
		friend void charHandler(GLFWwindow* win, unsigned int codepoint);
		friend void dropHandler(GLFWwindow* win, int count, const char** filenames);
		friend void scrollHandler(GLFWwindow* win, double x, double y);
		friend void framebufferSizeHandler(GLFWwindow* win, int width, int height);

		friend class Engine;
	};
}