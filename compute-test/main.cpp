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

        std::ifstream f("content/test.comp");
        std::string code;
        if (f.is_open()) {
            f.seekg(0, std::ios::end);
            code.reserve((size_t)f.tellg());
            f.seekg(0, std::ios::beg);

            code.assign((std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>());
            f.close();
        }
        else {
            throw std::ios_base::failure("Could not find compute shader!");
        }

        // Compile and link shader program
        GLuint shader = compileShader(code, ShaderType::COMPUTE);
        GLuint computeProgram = glCreateProgram();
        glAttachShader(computeProgram, shader);
        glLinkProgram(computeProgram);
        printProgramLinkerOutput(computeProgram);

        // Make output texture
        uint32_t width = 512;
        uint32_t height = 512;
        ref<Texture> output_texture;
        content()->getFactory<Texture>()->makeTexture2D(&output_texture, 
            en.handle(), width, height, GL_RGBA32F);

        // Perform computation
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, output_texture->id());
        glBindImageTexture(0, output_texture->id(), 0, false, 0, 
            GL_WRITE_ONLY, GL_RGBA32F);
        glUseProgram(computeProgram);
        glDispatchCompute((GLuint)width, (GLuint)height, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        while (en.valid()) {
            en.update();
            en.render(sceneHandle);
            en.renderer()->debugBlit(output_texture, glm::vec2(0.0, 0.0), glm::vec2(width, height));
            en.present();
        }
    }
    en.shutdown();
}