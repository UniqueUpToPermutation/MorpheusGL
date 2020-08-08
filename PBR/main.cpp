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
#include "meshlap.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

using namespace Morpheus;
using namespace std;
using namespace glm;

class MaterialGui : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	virtual void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Shader Settings");

		gui->addGroup("Mesh");
		gui->addButton("Do Something...", [&] {});

		mScreen->setVisible(true);
		mScreen->performLayout();
	}

public:
	virtual void dispose() override {
		delete gui;

		GuiBase::dispose();
	}

	void update() {
		gui->refresh();
	}
};
SET_BASE_TYPE(MaterialGui, GuiBase);

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		// Create a scene
		auto scene = new Scene();
		auto sceneNode = graph()->addNode(scene, engine()->handle());
		graph()->setName(sceneNode, "__scene__");
		auto sceneHandle = graph()->issueHandle(sceneNode);

		// Create our GUI
		auto guiNode = graph()->addNode(new MaterialGui(), sceneNode);
		graph()->setName(guiNode, "__gui__");

		// Create camera and camera controller
		auto camera = new Camera();
		auto cameraNode = graph()->addNode(camera, sceneNode);
		auto cameraControllerNode = graph()->addNode(new LookAtCameraController(0.0), cameraNode);

		graph()->setName(cameraControllerNode, "__camera_controller__");

		f_key_capture_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			if (key == GLFW_KEY_ESCAPE) {
				engine()->exit();
			}
			return false;
		};
		input()->registerTarget(&en, InputPriority::CRITICAL);
		input()->bindKeyEvent(&en, &keyHandler);

		auto staticMeshNode = content()->load<StaticMesh>("spheremesh.json");
		auto geo = StaticMesh::getGeometry(staticMeshNode);

		auto aabb = geo->boundingBox();
		float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;
		auto controller = desc("__camera_controller__")->owner.getAs<LookAtCameraController>();
		controller->reset(distance);
		controller->setPhi(-pi<double>() / 2.0);

		auto transform = scene->makeIdentityTransform();
		sceneNode.addChild(transform);
		transform.addChild(staticMeshNode);

		// Initialize the scene graph
		init(sceneNode);

		// Game loop
		while (en.valid()) {
			en.update();
			en.renderer()->draw((*graph())[sceneHandle]);
			glfwSwapBuffers(en.window());
		}
	}

	en.shutdown();
}