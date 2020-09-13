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

        GLuint local_size_x = 32;
        GLuint local_size_y = 32;

        ShaderUniform<float> timeUniform;
        timeUniform.mLoc = glGetUniformLocation(computeShader->id(), "Time");

        en.input()->bindFramebufferSizeEvent(&en, &resizeHandler);
        en.renderer()->setClearColor(0.0, 0.0, 0.0);

        while (en.valid()) {
            glfwGetFramebufferSize(en.window(), &width, &height);

            if (bRefresh) {
                GLuint target_texture_size_x = (width / local_size_x + 1) * local_size_x;
                GLuint target_texture_size_y = (height / local_size_y + 1) * local_size_y;

                // Resize if necessary
                if (output_texture->width() < target_texture_size_x ||
                    output_texture->height() < target_texture_size_y ||
                    output_texture->width() > target_texture_size_x + 2 * local_size_x ||
                    output_texture->height() > target_texture_size_y + 2 * local_size_y) {
                    output_texture->resize(target_texture_size_x, target_texture_size_y);
                }
                bRefresh = false;
            }

            GLuint compute_groups_x = output_texture->width() / local_size_x;
            GLuint compute_groups_y = output_texture->height() / local_size_y;

            // Perform computation
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, output_texture->id());
            glBindImageTexture(0, output_texture->id(), 0, false, 0, 
                GL_WRITE_ONLY, GL_RGBA32F);
            glUseProgram(computeShader->id());
            timeUniform.set(glfwGetTime());
            glDispatchCompute(compute_groups_x, compute_groups_y, 1);
            GL_ASSERT;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            en.update();
            en.render(sceneHandle);
            // Display texture
            en.renderer()->debugBlit(output_texture);
            en.present();
        }
    }
    en.shutdown();
}