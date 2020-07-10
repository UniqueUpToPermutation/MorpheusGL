#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"
#include "gui.hpp"
#include "material.hpp"
#include "staticmesh.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

using namespace Morpheus;
using namespace glm;

nanogui::Color clr(1.0f, 1.0f, 1.0f, 1.0f);

class CookTorranceView : public ShaderView {
public:
	ShaderUniform<vec3> mSpecularColor;
	ShaderUniform<float> mF0;
	ShaderUniform<float> mRoughness;
	ShaderUniform<float> mK;
	ShaderUniform<vec3> mLightColor;
	ShaderUniform<float> mAmbientStrength;
	ShaderUniform<float> mLightIntensity;
	ShaderUniform<vec3> mEyePosition;
	ShaderUniform<vec3> mLightPosition;

	inline CookTorranceView(ref<Shader>& shader_) : ShaderView(shader_) 
	{ 
		link(mSpecularColor, "specularColor");
		link(mF0, "F0");
		link(mRoughness, "roughness");
		link(mK, "k");
		link(mLightColor, "lightColor");
		link(mAmbientStrength, "ambientStrength");
		link(mLightIntensity, "lightIntensity");
		link(mEyePosition, "eyePosition");
		link(mLightPosition, "lightPosition");
	}
};

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

		print(engine()->node());

		Node staticMesh = content()->load<StaticMesh>("staticmesh.json");

		print(engine()->node());

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