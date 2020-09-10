#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/meshlap.hpp>

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

#include <Eigen/Sparse> 
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/Dense>

#include <Spectra/GenEigsRealShiftSolver.h>
#include <Spectra/MatOp/SparseGenRealShiftSolve.h>

using namespace Morpheus;
using namespace glm;
using namespace std;
using namespace Spectra;

nanogui::Color clr(0.5f, 0.5f, 1.0f, 1.0f);
int requestedModeCount = 10;
int currentMode = 0;

Eigen::SparseMatrix<double> lap;
Eigen::MatrixXd modes;
Eigen::SparseMatrix<double> lifter;

void loadGeo(const string& source);
void updateGui();
void updateCurrentMode();
void nextMode();
void previousMode();
void computeModes();

class LaplacianGui;

namespace Morpheus {
	SET_BASE_TYPE(LaplacianGui, GuiBase);
}

class LaplacianGui : public GuiBase {
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

		gui->addGroup("Laplacian");
		gui->addButton("Compute Modes", [&] { computeModes(); });
		gui->addVariable("Requested Modes", requestedModeCount);
		gui->addVariable("Current Mode", currentMode, false);
		gui->addButton("Next Mode", [&] { nextMode(); });
		gui->addButton("Previous Mode", [&] { previousMode(); });

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

	void update() {
		gui->refresh();
	}
};

void updateGui() {
	auto gui = desc("__gui__")->owner.getAs<LaplacianGui>();
	gui->update();
}

void updateCurrentMode() {
	uint32_t mode_to_get = std::min<uint32_t>(std::max<uint32_t>(0, 
		static_cast<uint32_t>(modes.cols()) - currentMode - 1), static_cast<uint32_t>(modes.cols()));

	Eigen::VectorXd mode = lifter * modes.col(mode_to_get);

	double maxVal = 0.0;
	for (uint32_t i = 0; i < mode.size(); ++i)
		maxVal = std::max<double>(maxVal, std::abs(mode(i)));

	// Make adjustments to raw geometry
	auto raw_geo = desc("__raw_geo__")->owner.reinterpretGet<HalfEdgeGeometry>();
	vec3 positive_color(1.0f, 0.0f, 0.0f);
	vec3 negative_color(0.0f, 0.0f, 1.0f);
	vec3 zero_color(1.0f, 1.0f, 1.0f);

	for (auto it = raw_geo->getVertex(0); it.valid(); it = it.nextById()) {
		vec3 color;

		double val = mode(it.id());
		if (val >= 0.0f)
			color = (float)(val / maxVal) * positive_color + (float)(1 - val / maxVal) * zero_color;
		else
			color = (float)(-val / maxVal) * negative_color + (float)(1 + val / maxVal) * zero_color;

		it.setColor(color);
	}

	// Unload the owner of the node and replace it with new geometry
	auto geo_factory = content()->getFactory<Geometry>();

	geo_factory->unload(desc("__geo__")->owner);
	desc("__geo__")->owner = geo_factory->makeGeometryUnmanaged(raw_geo);

	updateGui();
}

void nextMode() {
	if (currentMode < modes.cols() - 1) {
		++currentMode;
		updateCurrentMode();
	}
}

void previousMode() {
	if (currentMode > 0) {
		--currentMode;
		updateCurrentMode();
	}
}

void computeModes() {
	SparseGenRealShiftSolve<double> lapOp(lap);

	auto numModes = std::min<uint32_t>(requestedModeCount, lap.rows() - 3);

	GenEigsRealShiftSolver<double, LARGEST_MAGN,
		SparseGenRealShiftSolve<double>> eigs(&lapOp, numModes+1, std::min<uint32_t>(2 * (numModes + 1), lap.rows()), 0.01);

	eigs.init();
	eigs.compute();
	if (eigs.info() == SUCCESSFUL) {
		modes = eigs.eigenvectors().real();
	}
	else {
		cout << "Error: Eigenvalue Failure!" << endl;
	}

	currentMode = 0;
	updateCurrentMode();
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

	// Compute the laplacian minor of the interior of the object
	laplacianInteriorMinor(*rawGeo.get(), &lap, &lifter);
	lifter = lifter.transpose();

	// Load material node if necessary
	Node matNode = content()->load<Material>("content/funcvizmaterial.json");

	// Convert half edge into renderable geometry
	Morpheus::ref<Geometry> geo;
	Node geoNode = content()->getFactory<Geometry>()->makeGeometry(rawGeo.get(), "__geo__", &geo);
	graph()->setName(geoNode, "__geo__");

	auto scene = desc("__scene__")->owner.reinterpretGet<Scene>();
	auto meshNode = content()->getFactory<StaticMesh>()->makeStaticMesh(matNode, geoNode, "__static_mesh__");

	Node transformNode = scene->makeTranslation(vec3(0.0f, 0.0f, 0.0f));
	subSceneNode.addChild(transformNode);
	transformNode.addChild(meshNode);

	auto aabb = geo->boundingBox();
	float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;

	auto controller = desc("__camera_controller__")->owner.getAs<LookAtCameraController>();
	controller->reset(distance);
	controller->setPhi(- pi<double>() / 2.0);
	
	// Initialize this sub scene
	init(subSceneNode);
}

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		// Load default cook-torrance material
		Node matNode = content()->load<Material>("content/funcvizmaterial.json");

		// Create a scene
		auto sceneNode = graph()->addNode(new Scene(), engine()->handle());
		graph()->setName(sceneNode, "__scene__");
		auto sceneHandle = graph()->issueHandle(sceneNode);

		// Create our GUI
		auto guiNode = graph()->addNode(new LaplacianGui(), sceneNode);
		graph()->setName(guiNode, "__gui__");

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
		loadGeo("content/bunny.obj");

		// Game loop
		while (en.valid()) {
			en.update();
			en.renderer()->setClearColor(clr.r(), clr.g(), clr.b());
			en.renderer()->draw((*graph())[sceneHandle]);
			glfwSwapBuffers(en.window());
		}
	}

	en.shutdown();
}