#pragma once

#include <engine/core.hpp>
#include <engine/engine.hpp>

namespace nanogui {
	class Screen;
}

namespace Morpheus {
	class GuiBase : public INodeOwner {
	protected:
		nanogui::Screen* mScreen;
		f_cursor_pos_capture_t mCursorPosHandler;
		f_mouse_button_capture_t mMouseButtonHandler;
		f_key_capture_t mKeyHandler;
		f_char_capture_t mCharHandler;
		f_drop_capture_t mDropHandler;
		f_scroll_capture_t mScrollHandler;
		f_framebuffer_size_capture_t mFramebufferSizeHandler;

		virtual void initGui() = 0;

	public:
		inline GuiBase() : INodeOwner(NodeType::NANOGUI_SCREEN) {
		}

		inline nanogui::Screen* screen() { return mScreen; }

		GuiBase* toGui() override;

		void init() override;
		~GuiBase() override;
	};
	SET_NODE_ENUM(GuiBase, NANOGUI_SCREEN);
}