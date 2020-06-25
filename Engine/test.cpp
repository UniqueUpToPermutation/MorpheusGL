#include <iostream>
#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"

using namespace std;
using namespace Morpheus;

int main() {
	Engine engine;

	if (engine.startup("config.json").isSuccess()) {

		auto shader = content().load<IShader>("shaders/basic.json");

		// Make a thing
		while (engine.valid()) {
			engine.update();
			engine.render();
		}
	}

	engine.shutdown();
}