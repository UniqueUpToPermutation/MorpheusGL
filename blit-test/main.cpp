#include <engine/morpheus.hpp>

#include <iostream>
#include <GLFW/glfw3.h>

using namespace Morpheus;

int main() {
    Engine en;
    if (en.startup("config.json").isSuccess()) {
        auto sceneNode = en.makeScene();
        auto sceneHandle = issueHandle(sceneNode);

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

        ref<Texture> texture;
        en.content()->load<Texture>("content/brick.ktx", en.handle(), &texture);

        while (en.valid()) {
            en.update();
            en.render(sceneHandle);
            en.renderer()->debugBlit(texture, 
                glm::vec2(0.0, 0.0), 
                glm::vec2(512.0, 512.0));
            en.present();
        }
    }
    en.shutdown();
}