#include <engine/morpheus.hpp>
#include <engine/gui.hpp>
#include <engine/cameracontroller.hpp>
#include <engine/samplefunction.hpp>
#include <engine/lambert.hpp>
#include <engine/ggx.hpp>
#include <engine/sphericalharmonics.hpp>
#include <engine/brdf.hpp>
#include <engine/skybox.hpp>
#include <engine/spritebatch.hpp>

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace Morpheus;
using namespace std;
using namespace glm;

int main() {
	Engine en;

	if (en.startup("config.json").isSuccess()) {

		// Create a scene
		auto scene = en.makeScene();
		setName(scene, "__scene__");

		f_key_capture_t keyHandler = [](GLFWwindow*, int key, int scancode, int action, int modifiers) {
			if (key == GLFW_KEY_ESCAPE) {
				engine()->exit();
			}
			return false;
		};
		input()->registerTarget(&en, InputPriority::CRITICAL);
		input()->bindKeyEvent(&en, &keyHandler);

		Texture* tex = load<Texture>("content/sprite.png", scene);

		SpriteBatch* spriteBatch = new SpriteBatch();
		createNode(spriteBatch, scene);
		
		init(scene);

		// Create all the objects on screen
		struct ObjInstance {
			glm::vec2 mPositionBase;
			float mRotation;
			glm::vec4 mColor;
			float mAngularVelocity;
			glm::vec2 mOscillatorVector;
			float mOscillatorVelocity;
			float mOscillatorX;
		};

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> distribution1(-1.0, 1.0);
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);

		constexpr uint obj_count = 350;
		std::vector<ObjInstance> insts(obj_count);
		for (auto& obj : insts) {
			obj.mPositionBase.x = distribution1(generator) * 400;
			obj.mPositionBase.y = distribution1(generator) * 300;
			obj.mRotation = distribution1(generator) * glm::pi<double>();
			obj.mColor = glm::vec4(distribution2(generator), distribution2(generator), distribution2(generator), 1.0);
			obj.mAngularVelocity = distribution1(generator) * 0.01;
			obj.mOscillatorVector.x = distribution1(generator) * 50.0;
			obj.mOscillatorVector.y = distribution1(generator) * 50.0;
			obj.mOscillatorVelocity = distribution1(generator) * 0.01;
			obj.mOscillatorX = distribution1(generator) * glm::pi<double>();
		}

		en.renderer()->setClearColor(glm::vec3(1.0, 1.0, 1.0));

		// Game loop
		while (en.valid()) {

			en.update();
			en.render(scene);

			for (auto& obj : insts) {
				obj.mOscillatorX += obj.mOscillatorVelocity;
				obj.mRotation += obj.mAngularVelocity;
			}

			// Make sure that (0,0) is at the center of the screen
			int width, height;
			glfwGetFramebufferSize(window(), &width, &height);
			auto cameraTransform = glm::translate(glm::identity<glm::mat4>(), glm::vec3(width / 2, height / 2, 0.0));

			// Draw all sprites
			spriteBatch->begin(SpriteBatchMode::IMMEDIATE, 
				BlendMode::ALPHA, 
				SpriteBatchSampler::LINEAR, 
				cameraTransform);

			for (auto& obj : insts) {
				spriteBatch->draw(tex, obj.mPositionBase + glm::cos(obj.mOscillatorX) * obj.mOscillatorVector,
					glm::vec2(64, 64), UVRect{glm::ivec2(0, 0), glm::ivec2(tex->width(), tex->height())}, obj.mRotation,
					glm::vec2(32, 32), obj.mColor);
			}

			spriteBatch->end();

			en.present();
		}
	}

	en.shutdown();
}