#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/samplefunction.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/sphericalharmonics.hpp>
#include <engine/brdf.hpp>
#include <engine/skybox.hpp>
#include <engine/hdri2cube.hpp>

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>
#include <chrono>
#include <iomanip>

#include <glm/gtx/quaternion.hpp>

using namespace Morpheus;
using namespace std;
using namespace glm;

class MaterialGui;

// Set base types
namespace Morpheus {
	SET_BASE_TYPE(MaterialGui, GuiBase);
}

Material* material;

class MaterialGui : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Shader Settings");

		auto groupLabel = gui->addGroup("Material Properties");

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

		auto staticMesh = load<StaticMesh>("content/cerberus.json");
		auto geo = staticMesh->getGeometry();

		auto aabb = geo->boundingBox();
		float distance = length(aabb.mUpper - aabb.mLower) * 1.3f;
	
		controller->reset(distance);
		controller->setPhi(pi<double>() / 2.0);

		auto transform = new TransformNode();
		createNode(transform, scene);
		transform->addChild(staticMesh);
		transform->mTransform = Transform::makeRotation(glm::angleAxis(glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0)));

		ContentExtParams<Texture> params;
		params.bOverrideInternalFormat = true;
		params.mInternalFormat = GL_RGBA8;
		Texture* tex = loadEx<Texture>("content/textures/environment.hdr", params, scene);
 
		auto lambertKernel = new LambertComputeKernel();
		auto ggxKernel = new GGXComputeKernel();
		auto lutKernel = new CookTorranceLUTComputeKernel();
		auto hdri2cubeKernel = new HDRIToCubeKernel();

		createNode(lambertKernel, scene);
		createNode(ggxKernel, scene);
		createNode(lutKernel, scene);
		createNode(hdri2cubeKernel, scene);

		// Set material parameters
		material = staticMesh->getMaterial();

		// Initialize the scene graph
		init(scene);

		// Convert texture to a cube map
		HDRIToCubeComputeJob job;
		job.mHDRI = tex;
		job.mOutputFormat = GL_RGBA8;
		job.mTextureSize = 2048;
		Texture* envCube = hdri2cubeKernel->submit(job, scene);

		// Create a lookup texture with the BRDF kernel
		Texture* brdf = lutKernel->submit(scene);

		lutKernel->barrier();
		hdri2cubeKernel->barrier();

		// Submit a compute job to the lambert kernel
		LambertComputeJob lambertJob;
		lambertJob.mInputImage = envCube;
		lambertKernel->submit(lambertJob);

		// Submit a compute job to the ggx kernel
		GGXComputeJob ggxJob;
		ggxJob.mInputImage = envCube;
		auto specularResult = ggxKernel->submit(ggxJob, scene);

		lambertKernel->barrier();
		ggxKernel->barrier();

		ShaderUniform<glm::vec3[]> environmentDiffuseSH(material->shader(), "environmentDiffuseSH");
		ShaderUniform<Sampler> environmentSpecular(material->shader(), "environmentSpecular");
		ShaderUniform<Sampler> environmentBRDF(material->shader(), "environmentBRDF");

		Sampler* sampler = load<Sampler>(MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC, scene);
		Sampler* lutSampler = load<Sampler>(BILINEAR_CLAMP_SAMPLER_SRC, scene);

		material->uniformAssignments().add(environmentDiffuseSH, lambertKernel->results(), lambertKernel->shCount());
		material->samplerAssignments().add(environmentSpecular, sampler, specularResult);
		material->samplerAssignments().add(environmentBRDF, lutSampler, brdf);

		// Create a skybox from the texture
		Skybox* skybox = new Skybox(envCube);
		createNode(skybox, scene);
		init(skybox);

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