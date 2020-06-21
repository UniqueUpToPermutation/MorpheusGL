#include <iostream>
#include "digraph.hpp"
#include "core.hpp"

using namespace std;
using namespace Morpheus;

int main() {
	Engine engine;

	if (engine.startup("config.json").isSuccess()) {
		while (engine.valid()) {
			engine.update();
			engine.render();
		}
	}

	engine.shutdown();
}