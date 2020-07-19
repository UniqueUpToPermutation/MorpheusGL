#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"
#include "gui.hpp"
#include "material.hpp"
#include "staticmesh.hpp"
#include "scene.hpp"
#include "halfedge.hpp"
#include "halfedgeloader.hpp"
#include "cameracontroller.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

using namespace Morpheus;
using namespace glm;

nanogui::Color clr(0.5f, 0.5f, 1.0f, 1.0f);

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

		auto* scene = new Scene();
		Node sceneNode = graph()->addNode(scene, engine()->handle());
		graph()->addNode(new GuiTest(), sceneNode);

		HalfEdgeLoader loader;
		HalfEdgeGeometry* geo = loader.load("bunny.obj");

		// Add color to the bunny
		geo->createColors(vec3(1.0f, 1.0f, 1.0f));

		// Convert half edge geometry to renderable geometry
		ref<Geometry> hfeGeo;
		Node hfeGeoNode = content()->getFactory<Geometry>()->makeGeometry(geo, &hfeGeo);

		// Material for function visualization
		Node hfeMatNode = content()->load<Material>("funcvizmaterial.json");
		Node hfeMesh = content()->getFactory<StaticMesh>()->makeStaticMesh(hfeMatNode, hfeGeoNode);
		Node transformNode = scene->makeTranslation(vec3(0.0f, 0.0f, 0.0f));
		sceneNode.addChild(transformNode);
		transformNode.addChild(hfeMesh);

		auto aabb = hfeGeo->boundingBox();
		float len = length(aabb.mUpper - aabb.mLower) * 1.3f;
		auto camera = new Camera();
		
		auto cameraNode = graph()->addNode(camera, sceneNode);
		graph()->addNode(new LookAtCameraController(len), cameraNode);

		f_key_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			engine()->exit();
		};
		input()->bindKeyEvent(&keyHandler);

		// Initialize the scene subgraph
		init(sceneNode);

		// Game loop
		while (en.valid()) {
			en.update();
			glClearColor(clr.r(), clr.g(), clr.b(), 1.0f);
			en.renderer()->draw(sceneNode);
			glfwSwapBuffers(en.window());
		}
	}

	en.shutdown();
}