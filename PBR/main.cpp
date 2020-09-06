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
#include "samplefunction.hpp"
#include "lambert.hpp"
#include "sphericalharmonics.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>
#include <chrono>
#include <iomanip>

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

		auto transform = Transform::makeIdentity(sceneNode, sceneNode);
		transform.addChild(staticMeshNode);

		// Process the light field into an irradiance map
		auto light_field = StaticMesh::getMaterial(staticMeshNode)->samplerAssignments().mBindings[0].mTexture;

		FunctionSphere<glm::vec3> light_field_func;
		light_field_func.fromTexture(light_field);

		Eigen::MatrixXf light_field_mat;
		light_field_func.toSampleMatrix(&light_field_mat);

		Eigen::VectorXf X, Y, Z;
		Eigen::VectorXf mode;
		light_field_func.createGrid(&X, &Y, &Z);
		Eigen::MatrixXf modes;

		Eigen::MatrixXf coeffs;
		SphericalHarmonics::generateIrradianceModes(X, Y, Z, &modes);

		auto start = std::chrono::system_clock::now();
		Eigen::MatrixXf dual_modes;
		CubemapMetric::dual(modes, &dual_modes);

		Eigen::MatrixXf coeffs_gt = dual_modes.transpose() * light_field_mat;
		auto end = std::chrono::system_clock::now();

		std::cout << "Full Coeffs:" << std::endl;
		std::cout << coeffs_gt << std::endl;
		std::chrono::duration<double> duration = end - start;
		std::cout << "Time Required: " << std::setprecision(4) << duration.count() << " s" << std::endl << std::endl;

		LambertSphericalHarmonicsKernel kernel;
		Eigen::MatrixXf lambert_coeffs_gt;
		kernel.applySH(coeffs_gt, &lambert_coeffs_gt);

		Eigen::MatrixXf proj_light_field_mat = modes * lambert_coeffs_gt;

		light_field_func.transition(StorageMode::WRITE);
		light_field_func.fromSampleMatrix(proj_light_field_mat);
		light_field_func.transition(StorageMode::READ);

		Morpheus::ref<Texture> tex;
		auto texNode = light_field_func.toTexture(&tex);
		sceneNode.addChild(texNode); // Make sure the garbage collector doesn't come
		auto mat = StaticMesh::getMaterial(staticMeshNode)->samplerAssignments().mBindings[0].mTexture = tex;

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