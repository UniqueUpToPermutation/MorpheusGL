#include <engine/morpheus.hpp>

#include <iostream>
#include <GLFW/glfw3.h>

using namespace Morpheus;

int main() {
    Engine en;

    if (en.startup("config.json").isSuccess()) {

        // When escape is pressed, shut the engine down.
        f_key_capture_t keyHandler = [&en](GLFWwindow* win, 
            int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE) {
                en.exit();
                return true;
            }
            return false;
        };
        en.input()->registerTarget(&en);
        en.input()->bindKeyEvent(&en, &keyHandler);

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