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
#include <Eigen/Sparse>

#include <Spectra/MatOp/SparseGenMatProd.h>
#include <Spectra/SymEigsSolver.h>

using namespace Morpheus;
using namespace glm;
using namespace std;
using namespace Spectra;

nanogui::Color clr(0.5f, 0.5f, 1.0f, 1.0f);
uint32_t modeCount = 10;

Eigen::SparseMatrix<double> lap;
Eigen::MatrixXd modes;

void computeModes() {
	SparseGenMatProd<double> lapOp(lap);

	auto numModes = std::min<uint32_t>(modeCount, lap.rows() - 2);

	SymEigsSolver<double, SMALLEST_MAGN,
		SparseGenMatProd<double>> eigs(&lapOp, numModes+1, std::min<uint32_t>(7 * (numModes + 1), lap.rows()));
	eigs.init();
	eigs.compute();
	if (eigs.info() == SUCCESSFUL)
	{
		modes = eigs.eigenvectors();
		modes = modes.block(0, 1, modes.rows(), modes.cols() - 1);
	}
	else {
		cout << "Error: Eigenvalue Failure!" << endl;
	}
}

void updateCurrentMode() {

}

void loadGeo(const string& source) {
	Node subSceneNode;
	if (graph()->names().tryFind("__sub_scene__", &subSceneNode)) {
		prune(subSceneNode);
		subSceneNode = Node::invalid();
	}

	// Collect unnused data hanging around
	content()->collectGarbage();

	// Create a sub scene
	subSceneNode = graph()->addNode(new Scene(), "__scene__");
	graph()->setName(subSceneNode, "__sub_scene__");

	// Load and add color to the a bunny
	Morpheus::ref<HalfEdgeGeometry> rawGeo;
	auto rawGeoNode = content()->load<HalfEdgeGeometry>(source, &rawGeo);
	rawGeo->createColors(vec3(1.0f, 1.0f, 1.0f));
	graph()->setName(rawGeoNode, "__raw_geo__");

	// Compute the laplacian
	laplacian(*rawGeo.get(), &lap);
	
	// Load material node if necessary
	Node matNode = content()->load<Material>("funcvizmaterial.json");

	// Convert half edge into renderable geometry
	Morpheus::ref<Geometry> geo;
	Node geoNode = content()->getFactory<Geometry>()->makeGeometry(rawGeo.get(), "__geo__", &geo);

	auto scene = desc("__scene__")->owner.reinterpretGet<Scene>();
	auto meshNode = content()->getFactory<StaticMesh>()->makeStaticMesh(matNode, geoNode, "__static_mesh__");

	Node transformNode = scene->makeTranslation(vec3(0.0f, 0.0f, 0.0f));
	subSceneNode.addChild(transformNode);
	transformNode.addChild(meshNode);

	auto aabb = geo->boundingBox();
	float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;

	auto controller = desc("__camera_controller__")->owner.reinterpret<ILogic>().getAs<LookAtCameraController>();
	controller->reset(distance);
	controller->setPhi(- pi<double>() / 2.0);
	
	// Initialize this sub scene
	init(subSceneNode);

	cout << endl;
	print(engine()->node());
}

class GuiTest : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	virtual void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Laplace Tool");

		gui->addGroup("Mesh");
		gui->addButton("Load...", [&] {
			auto load_str = nanogui::file_dialog({ {"obj", "OBJ Wavefront File"} }, false);
			if (load_str.length() > 0) {
				loadGeo(load_str);
			}
		});
		gui->addButton("Compute Modes", [&] { computeModes(); });
		gui->addVariable("Mode Count", modeCount);

		gui->addGroup("Appearance");
		gui->addVariable("Background Color", clr);

		mScreen->setVisible(true);
		mScreen->performLayout();
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

		// Load default cook-torrance material
		Node matNode = content()->load<Material>("funcvizmaterial.json");

		// Create a scene
		auto sceneNode = graph()->addNode(new Scene(), engine()->handle());
		graph()->setName(sceneNode, "__scene__");

		// Create our GUI
		graph()->addNode(new GuiTest(), sceneNode);

		// Make sure the material node is not unloaded
		sceneNode.addChild(matNode);
	
		// Create camera and camera controller
		auto camera = new Camera();
		auto cameraNode = graph()->addNode(camera, sceneNode);
		auto cameraController = graph()->addNode(new LookAtCameraController(0.0), cameraNode);

		graph()->setName(cameraController, "__camera_controller__");

		f_key_capture_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			if (key == GLFW_KEY_ESCAPE) {
				engine()->exit();
			}
			return false;
		};
		input()->registerTarget(&en, InputPriority::CRITICAL);
		input()->bindKeyEvent(&en, &keyHandler);

		// Initialize the scene graph
		init(sceneNode);

		// Load the bunny!
		loadGeo("bunny.obj");

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