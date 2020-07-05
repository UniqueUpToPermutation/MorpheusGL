#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"
#include "gui.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

using namespace Morpheus;

nanogui::Color clr(1.0f, 1.0f, 1.0f, 1.0f);

class GuiTest : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	virtual void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Set Background Color");

		gui->addVariable("Background Color", clr);

		mScreen->setVisible(true);
		mScreen->performLayout();
		window->center();
	}

public:
	virtual void dispose() override {
		delete gui;

		GuiBase::dispose();
	}
};
SET_BASE_TYPE(GuiTest, GuiBase);

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		GuiTest* gui = new GuiTest();
		gui->init();

		Node guiNode = graph()->addNode(gui, engine()->handle());

		// Make a thing
		while (en.valid()) {
			en.update();

			glClearColor(clr.r(), clr.g(), clr.b(), 1.0f);
			en.renderer()->draw(guiNode);

			glfwSwapBuffers(en.window());
		}
	}

	en.shutdown();
}