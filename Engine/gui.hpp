#pragma once

#include "core.hpp"
#include "engine.hpp"

namespace nanogui {
	class Screen;
}

namespace Morpheus {
	class GuiBase : public IInitializable, public IDisposable {
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
		inline nanogui::Screen* screen() { return mScreen; }

		void init(Node& node) override;
		void dispose() override;
	};
	SET_NODE_ENUM(GuiBase, NANOGUI_SCREEN);
}