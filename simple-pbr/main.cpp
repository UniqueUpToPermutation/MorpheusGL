#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/samplefunction.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/sphericalharmonics.hpp>
#include <engine/brdf.hpp>
#include <engine/skybox.hpp>

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

Material* material;
ShaderUniform<GLfloat> roughness;
ShaderUniform<GLfloat> metalness;
ShaderUniform<glm::vec3> albedo;
ShaderUniform<glm::vec3> FDialectric;

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
	~MaterialGui() {
		delete gui;
	}

	void update() {
		gui->refresh();
	}
};

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		// Create a scene
		auto scene = en.makeScene();
		setName(scene, "__scene__");

		// Create camera and camera controller
		auto camera = new Camera();
		createNode(camera, scene);
		auto controller = new LookAtCameraController();
		createNode(controller, camera);

		setName(controller, "__camera_controller__");

		f_key_capture_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			if (key == GLFW_KEY_ESCAPE) {
				engine()->exit();
			}
			return false;
		};
		input()->registerTarget(&en, InputPriority::CRITICAL);
		input()->bindKeyEvent(&en, &keyHandler);

		auto staticMesh = load<StaticMesh>("content/spheremesh.json");
		auto geo = staticMesh->getGeometry();

		auto aabb = geo->boundingBox();
		float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;
	
		controller->reset(distance);
		controller->setPhi(-pi<double>() / 2.0);

		auto transform = new TransformNode();
		createNode(transform, scene);
		transform->addChild(staticMesh);

		Texture* tex = getFactory<Texture>()->loadGliUnmanaged("content/textures/env.ktx", GL_RGBA8);
		createContentNode(tex, scene);

		// Create a skybox from the texture
		Skybox* skybox = new Skybox(tex);
		createNode(skybox, scene);
 
		auto lambertKernel = new LambertComputeKernel();
		auto ggxKernel = new GGXComputeKernel();
		auto lutKernel = new CookTorranceLUTComputeKernel();

		createNode(lambertKernel, scene);
		createNode(ggxKernel, scene);
		createNode(lutKernel, scene);

		// Set material parameters
		material = staticMesh->getMaterial();
		roughness.find(material->shader(), "roughness");
		metalness.find(material->shader(), "metalness");
		albedo.find(material->shader(), "albedo");
		FDialectric.find(material->shader(), "Fdialectric");
		material->uniformAssignments().add(roughness, 0.0f);
		material->uniformAssignments().add(metalness, 0.0f);
		material->uniformAssignments().add(albedo, glm::vec3(1.0f, 1.0f, 1.0f));

		// Create our GUI
		auto gui = new MaterialGui();
		createNode(gui, scene);
		setName(gui, "__gui__");

		// Initialize the scene graph
		init(scene);

		// Submit a compute job to the lambert kernel
		LambertComputeJob lambertJob;
		lambertJob.mInputImage = tex;
		lambertKernel->submit(lambertJob);

		// Submit a compute job to the ggx kernel
		GGXComputeJob ggxJob;
		ggxJob.mInputImage = tex;
		auto specularResult = ggxKernel->submit(ggxJob);

		// Create a lookup texture with the BRDF kernel
		Texture* brdf = lutKernel->submit();
		createContentNode(brdf, scene);

		lambertKernel->barrier();
		ggxKernel->barrier();
		lutKernel->barrier();

		ShaderUniform<glm::vec3[]> environmentDiffuseSH(material->shader(), "environmentDiffuseSH");
		ShaderUniform<Sampler> environmentSpecular(material->shader(), "environmentSpecular");
		ShaderUniform<Sampler> environmentBRDF(material->shader(), "environmentBRDF");

		Sampler* sampler = load<Sampler>(MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC, scene);
		Sampler* lutSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, scene);

		material->uniformAssignments().add(environmentDiffuseSH, lambertKernel->results(), lambertKernel->shCount());
		material->samplerAssignments().add(environmentSpecular, sampler, specularResult);
		material->samplerAssignments().add(environmentBRDF, lutSampler, brdf);

		print(&en);

		// Game loop
		while (en.valid()) {
			en.update();
			en.render(scene);
			en.present();
		}
	}

	en.shutdown();
}