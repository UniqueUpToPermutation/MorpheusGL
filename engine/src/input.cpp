#include <engine/input.hpp>
#include <engine/engine.hpp>

#include <iostream>

#include <GLFW/glfw3.h>

namespace Morpheus {
	void cursorPosHandler(GLFWwindow* win, double x, double y) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mCursorPosCallback)
				if ((*f.mCursorPosCallback)(win, x, y))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mMouseButtonCallback)
				if ((*f.mMouseButtonCallback)(win, button, action, modifiers))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mKeyCallback)
				if ((*f.mKeyCallback)(win, key, scancode, action, mods))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void charHandler(GLFWwindow* win, unsigned int codepoint) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mCharCallback)
				if ((*f.mCharCallback)(win, codepoint))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void dropHandler(GLFWwindow* win, int count, const char** filenames) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mDropCallback)
				if ((*f.mDropCallback)(win, count, filenames))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void scrollHandler(GLFWwindow* win, double x, double y) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mScrollCallback)
				if ((*f.mScrollCallback)(win, x, y))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	void framebufferSizeHandler(GLFWwindow* win, int width, int height) {
		auto input_ = input();
		input_->bInEvent = true;
		for (auto& f : input_->mTargets) {
			if (f.mFramebufferSizeCallback)
				if ((*f.mFramebufferSizeCallback)(win, width, height))
					break;
		}
		input_->bInEvent = false;
		input_->resolveGrabs();
	}

	Input::Input() : bInEvent(false) {

	}

	void Input::glfwRegister() {
		auto window = engine()->window();

		if (window) {
			glfwSetCursorPosCallback(window, &cursorPosHandler);
			glfwSetMouseButtonCallback(window, &mouseButtonHandler);
			glfwSetKeyCallback(window, &keyHandler);
			glfwSetCharCallback(window, &charHandler);
			glfwSetDropCallback(window, &dropHandler);
			glfwSetScrollCallback(window, &scrollHandler);
			glfwSetFramebufferSizeCallback(window, &framebufferSizeHandler);
		}
	}

	void Input::glfwUnregster() {
		auto window = engine()->window();

		if (window) {
			glfwSetCursorPosCallback(window, nullptr);
			glfwSetMouseButtonCallback(window, nullptr);
			glfwSetKeyCallback(window, nullptr);
			glfwSetCharCallback(window, nullptr);
			glfwSetDropCallback(window, nullptr);
			glfwSetScrollCallback(window, nullptr);
			glfwSetFramebufferSizeCallback(window, nullptr);
		}
	}

	// Target registration
	void Input::registerTarget(void* owner, InputPriority priority) {
		InputTarget target;
		target.mCharCallback = nullptr;
		target.mCursorPosCallback = nullptr;
		target.mDropCallback = nullptr;
		target.mFramebufferSizeCallback = nullptr;
		target.mKeyCallback = nullptr;
		target.mMouseButtonCallback = nullptr;
		target.mScrollCallback = nullptr;
		target.mOwner = owner;
		target.mPriority = priority;
		target.mDefaultPriority = priority;

		auto result = mTargets.emplace(target);
		mOwnerToTargetMap[owner] = result.first;
	}
	void Input::unregisterTarget(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end()) {
			auto setIt = it->second;
			mOwnerToTargetMap.erase(it);
			mTargets.erase(setIt);
		}
	}
	
	void Input::actuallyGrab(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end()) {
			auto targetStruct = *it->second;
			mTargets.erase(it->second);
			targetStruct.mPriority = InputPriority::GRABBED;
			auto it = mTargets.emplace(targetStruct);
			mOwnerToTargetMap[owner] = it.first;
		}
		else {
			std::cout << "Failed to grab, could not find owner!" << std::endl;
		}
	}

	void Input::grab(void* owner) {
		if (bInEvent) {
			mNewGrabers.push_back(owner);
		}
		else {
			actuallyGrab(owner);
		}
	}

	void Input::resolveGrabs() {
		for (auto g : mNewGrabers)
			actuallyGrab(g);
		for (auto ug : mUngrabers)
			actuallyUngrab(ug);
		mNewGrabers.clear();
		mUngrabers.clear();
	}

	void Input::actuallyUngrab(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end()) {
			auto targetStruct = *it->second;
			mTargets.erase(it->second);
			targetStruct.mPriority = targetStruct.mDefaultPriority;
			auto it = mTargets.emplace(targetStruct);
			mOwnerToTargetMap[owner] = it.first;
		}
		else {
			std::cout << "Failed to ungrab, could not find owner!" << std::endl;
		}
	}
	void Input::ungrab(void* owner) {
		if (bInEvent) {
			mUngrabers.push_back(owner);
		}
		else {
			actuallyUngrab(owner);
		}
	}

	// Events that can be bound to
	void Input::bindCursorPosEvent(void* owner, const f_cursor_pos_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mCursorPosCallback = f;
		else {
			registerTarget(owner);
			bindCursorPosEvent(owner, f);
		}
	}
	void Input::bindMouseButtonEvent(void* owner, const f_mouse_button_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mMouseButtonCallback = f;
		else {
			registerTarget(owner);
			bindMouseButtonEvent(owner, f);
		}
	}
	void Input::bindKeyEvent(void* owner, const f_key_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mKeyCallback = f;
		else {
			registerTarget(owner);
			bindKeyEvent(owner, f);
		}
	}
	void Input::bindCharEvent(void* owner, const f_char_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mCharCallback = f;
		else {
			registerTarget(owner);
			bindCharEvent(owner, f);
		}
	}
	void Input::bindDropEvent(void* owner, const f_drop_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mDropCallback = f;
		else {
			registerTarget(owner);
			bindDropEvent(owner, f);
		}
	}
	void Input::bindScrollEvent(void* owner, const f_scroll_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mScrollCallback = f;
		else {
			registerTarget(owner);
			bindScrollEvent(owner, f);
		}
	}
	void Input::bindFramebufferSizeEvent(void* owner, const f_framebuffer_size_capture_t* f) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mFramebufferSizeCallback = f;
		else {
			registerTarget(owner);
			bindFramebufferSizeEvent(owner, f);
		}
	}

	// Events that can be bound to
	void Input::unbindCursorPosEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mCursorPosCallback = nullptr;
	}
	void Input::unbindMouseButtonEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mMouseButtonCallback = nullptr;
	}
	void Input::unbindKeyEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mKeyCallback = nullptr;
	}
	void Input::unbindCharEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mCharCallback = nullptr;
	}
	void Input::unbindDropEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mDropCallback = nullptr;
	}
	void Input::unbindScrollEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mScrollCallback = nullptr;
	}
	void Input::unbindFramebufferSizeEvent(void* owner) {
		auto it = mOwnerToTargetMap.find(owner);
		if (it != mOwnerToTargetMap.end())
			it->second->mFramebufferSizeCallback = nullptr;
	}
}