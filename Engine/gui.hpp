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
		f_cursor_pos_t mCursorPosHandler;
		f_mouse_button_t mMouseButtonHandler;
		f_key_t mKeyHandler;
		f_char_t mCharHandler;
		f_drop_t mDropHandler;
		f_scroll_t mScrollHandler;
		f_framebuffer_size_t mFramebufferSizeHandler;

		virtual void initGui() = 0;

	public:
		inline nanogui::Screen* screen() { return mScreen; }

		void init(Node& node) override;
		void dispose() override;
	};
	SET_NODE_ENUM(GuiBase, NANOGUI_SCREEN);
}