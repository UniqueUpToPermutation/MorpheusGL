#include <iostream>

#include <engine/morpheus.hpp>

using namespace Morpheus;

int main() {
    Engine en;
    if (en.startup("config.json").isSuccess()) {
        // Create a scene
		auto sceneNode = en.makeScene();

        while (en.valid()) {
            en.update();
            en.render(sceneNode);
            en.present();
        }
    }
    en.shutdown();
}