#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/samplefunction.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/sphericalharmonics.hpp>

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace Morpheus;
using namespace std;
using namespace glm;

class MaterialGui;

// Set base types
namespace Morpheus {
	SET_BASE_TYPE(MaterialGui, GuiBase);
}

Morpheus::ref<Material> material;
ShaderUniform<GLfloat> roughness;
class MaterialGui : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Shader Settings");

		auto groupLabel = gui->addGroup("Material Properties");

		std::function<void(const float& v)> roughnessSetter = [](const float& v) { 
			float truR = std::min(std::max(v, 0.0f), 1.0f);
			material->uniformAssignments().set(roughness, truR); 
		};
		std::function<float()> roughnessGetter = []() { return material->uniformAssignments().get(roughness); };

		auto slider = new nanogui::Slider(window);
		slider->setRange(std::pair<float, float>(0.0, 1.0));
		slider->setCallback(roughnessSetter);

		gui->addWidget("Roughness", slider);

		mScreen->setVisible(true);
		mScreen->performLayout();
	}

public:
	void dispose() override {
		delete gui;

		GuiBase::dispose();
	}

	void update() {
		gui->refresh();
	}
};

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		// Create a scene
		auto scene = new Scene();
		auto sceneNode = addNode(scene, engine()->handle());
		setName(sceneNode, "__scene__");
		auto sceneHandle = issueHandle(sceneNode);

		// Create camera and camera controller
		auto camera = new Camera();
		auto cameraNode = addNode(camera, sceneNode);
		auto controller = new LookAtCameraController();
		auto cameraControllerNode = addNode(controller, cameraNode);

		setName(cameraControllerNode, "__camera_controller__");

		f_key_capture_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			if (key == GLFW_KEY_ESCAPE) {
				engine()->exit();
			}
			return false;
		};
		input()->registerTarget(&en, InputPriority::CRITICAL);
		input()->bindKeyEvent(&en, &keyHandler);

		auto staticMeshNode = load<StaticMesh>("content/spheremesh.json");
		auto geo = StaticMesh::getGeometry(staticMeshNode);

		auto aabb = geo->boundingBox();
		float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;
	
		controller->reset(distance);
		controller->setPhi(-pi<double>() / 2.0);

		auto transform = Transform::makeIdentity(sceneNode, sceneNode);
		transform.addChild(staticMeshNode);

		Morpheus::ref<Texture> tex = getFactory<Texture>()->loadGliUnmanaged("content/textures/skybox.ktx", GL_RGBA8);
		Node texNode = addNode<Texture>(tex, sceneNode);

		auto lambertKernel = new LambertComputeKernel();
		Node lambertKernelNode = addNode(lambertKernel, sceneNode);

		Morpheus::ref<Shader> ggxBackend;
		load<Shader>("content/ggx.comp", sceneHandle, &ggxBackend);
		auto ggxKernel = new GGXComputeKernel(ggxBackend, 32);
		Node ggxKernelNode = addNode(ggxKernel, sceneNode);

		material = StaticMesh::getMaterial(staticMeshNode);
		roughness.find(material->shader(), "roughness");
		material->uniformAssignments().add(roughness, 0.0f);

		// Create our GUI
		auto guiNode = addNode(new MaterialGui(), sceneNode);
		setName(guiNode, "__gui__");

		// Initialize the scene graph
		init(sceneNode);

		// Submit a compute job to the lambert kernel
		LambertComputeJob lambertJob;
		lambertJob.mInputImage = tex;
		lambertKernel->submit(lambertJob);
		lambertKernel->sync();

		// Submit a compute job to the ggx kernel
		
		GGXComputeJob ggxJob;
		ggxJob.mInputImage = tex;
		auto specularResult = ggxKernel->submit(ggxJob);
		ggxKernel->sync();

		ShaderUniform<glm::vec3[]> diffuseIrradianceSH(material->shader(), "diffuseIrradianceSH");
		ShaderUniform<Sampler> specularEnvMap(material->shader(), "specularEnvMap");

		Morpheus::ref<Sampler> sampler;
		load<Sampler>(MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC, sceneHandle, &sampler);

		material->uniformAssignments().add(diffuseIrradianceSH, lambertKernel->results(), lambertKernel->shCount());
		material->samplerAssignments().add(specularEnvMap, sampler, specularResult);

		// Game loop
		while (en.valid()) {
			en.update();
			en.render(sceneHandle);
			en.present();
		}
	}

	en.shutdown();
}