#include <engine/morpheus.hpp>

#include <iostream>
#include <fstream>
#include <glad/glad.h>
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

        // Load shader program
        ref<Shader> computeShader;
        load<Shader>("content/test.comp", sceneHandle, &computeShader);

        // Make output texture
        int width;
        int height;
        glfwGetFramebufferSize(en.window(), &width, &height);
        ref<Texture> output_texture;
        getFactory<Texture>()->makeTexture2D(&output_texture, 
            sceneHandle, width, height, GL_RGBA32F);

        // Resizing the window causes a texture resize
        bool bRefresh = true;
        f_framebuffer_size_capture_t resizeHandler = [&bRefresh](GLFWwindow*, int, int) {
            bRefresh = true;
            return false;
        };

        ShaderUniform<float> timeUniform;
        timeUniform.mLoc = glGetUniformLocation(computeShader->id(), "Time");

        en.input()->bindFramebufferSizeEvent(&en, &resizeHandler);

        while (en.valid()) {
            glfwGetFramebufferSize(en.window(), &width, &height);

            if (bRefresh) {
                // Resize if necessary
                if (output_texture->width() != width ||
                    output_texture->height() != height) {
                    output_texture->resize(width, height);
                }
                bRefresh = false;
            }

            // Perform computation
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, output_texture->id());
            glBindImageTexture(0, output_texture->id(), 0, false, 0, 
                GL_WRITE_ONLY, GL_RGBA32F);
            glUseProgram(computeShader->id());
            timeUniform.set(glfwGetTime());
            glDispatchCompute((GLuint)width, (GLuint)height, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            en.update();
            en.render(sceneHandle);
            // Display texture
            en.renderer()->debugBlit(output_texture, 
                glm::vec2(0.0, 0.0), 
                glm::vec2(width, height));
            en.present();
        }
    }
    en.shutdown();
}