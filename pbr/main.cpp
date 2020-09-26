#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/samplefunction.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/sphericalharmonics.hpp>
#include <engine/brdf.hpp>

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
ShaderUniform<GLfloat> metalness;
ShaderUniform<glm::vec3> albedo;

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

		std::function<void(const float& v)> metalnessSetter = [](const float& v) {
			float truM = std::min(std::max(v, 0.0f), 1.0f);
			material->uniformAssignments().set(metalness, truM);
		};

		auto roughnessSlider = new nanogui::Slider(window);
		roughnessSlider->setRange(std::pair<float, float>(0.0, 1.0));
		roughnessSlider->setCallback(roughnessSetter);

		gui->addWidget("Roughness", roughnessSlider);

		auto metalnessSlider = new nanogui::Slider(window);
		metalnessSlider->setRange(std::pair<float, float>(0.0, 1.0));
		metalnessSlider->setCallback(metalnessSetter);

		gui->addWidget("Metalness", metalnessSlider);

		std::function<void(const nanogui::Color& v)> albedoSetter = [](const nanogui::Color& v) {
			material->uniformAssignments().set(albedo, glm::vec3(v.r(), v.g(), v.b()));
		};
		std::function<nanogui::Color()> albedoGetter = []() {
			glm::vec3 v = material->uniformAssignments().get<glm::vec3>(albedo);
			return nanogui::Color(v.r, v.g, v.b, 1.0);
		};

		gui->addVariable("Albedo", albedoSetter, albedoGetter);

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

		Morpheus::ref<Shader> lutBackend;
		load<Shader>("content/brdf.comp", sceneHandle, &lutBackend);
		auto lutKernel = new CookTorranceLUTComputeKernel(lutBackend, 32);
		Node lutKernelNode = addNode(lutKernel, sceneNode);

		material = StaticMesh::getMaterial(staticMeshNode);
		roughness.find(material->shader(), "roughness");
		metalness.find(material->shader(), "metalness");
		albedo.find(material->shader(), "albedo");
		material->uniformAssignments().add(roughness, 0.0f);
		material->uniformAssignments().add(metalness, 0.0f);
		material->uniformAssignments().add(albedo, glm::vec3(1.0f, 1.0f, 1.0f));

		// Create our GUI
		auto guiNode = addNode(new MaterialGui(), sceneNode);
		setName(guiNode, "__gui__");

		// Initialize the scene graph
		init(sceneNode);

		// Submit a compute job to the lambert kernel
		LambertComputeJob lambertJob;
		lambertJob.mInputImage = tex;
		lambertKernel->submit(lambertJob);

		// Submit a compute job to the ggx kernel
		GGXComputeJob ggxJob;
		ggxJob.mInputImage = tex;
		auto specularResult = ggxKernel->submit(ggxJob);

		Morpheus::ref<Texture> lut = lutKernel->submit();
		addNode<Texture>(lut, sceneNode);

		lambertKernel->barrier();
		ggxKernel->barrier();
		lutKernel->barrier();

		ShaderUniform<glm::vec3[]> environmentDiffuseSH(material->shader(), "environmentDiffuseSH");
		ShaderUniform<Sampler> environmentSpecular(material->shader(), "environmentSpecular");
		ShaderUniform<Sampler> environmentBRDF(material->shader(), "environmentBRDF");

		Morpheus::ref<Sampler> sampler;
		load<Sampler>(MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC, sceneHandle, &sampler);

		Morpheus::ref<Sampler> lutSampler;
		load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, sceneHandle, &lutSampler);

		material->uniformAssignments().add(environmentDiffuseSH, lambertKernel->results(), lambertKernel->shCount());
		material->samplerAssignments().add(environmentSpecular, sampler, specularResult);
		material->samplerAssignments().add(environmentBRDF, lutSampler, lut);

		// Game loop
		while (en.valid()) {
			en.update();
			en.render(sceneHandle);
			//en.renderer()->debugBlit(lut, glm::vec2(0.0, 0.0), glm::vec2(256, 256));
			en.present();
		}
	}

	en.shutdown();
}