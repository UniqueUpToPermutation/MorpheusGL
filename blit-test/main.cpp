#include <engine/morpheus.hpp>

#include <iostream>
#include <GLFW/glfw3.h>

using namespace Morpheus;

int main() {
    Engine en;
    if (en.startup("config.json").isSuccess()) {
        auto scene = en.makeScene();

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

        Texture* texture = load<Texture>("content/brick.ktx", scene);

		init(scene);

        while (en.valid()) {
            en.update();
            en.render(scene);
            en.renderer()->debugBlit(texture, 
                glm::vec2(0.0, 0.0), 
                glm::vec2(512.0, 512.0));
            en.present();
        }
    }
    en.shutdown();
}