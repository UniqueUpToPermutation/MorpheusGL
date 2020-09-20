#include <engine/morpheus.hpp>

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace Morpheus;

int main() {
    Engine en;
    if (en.startup("config.json").isSuccess()) {
        // Load shader program
        ref<Shader> computeShader;
        load<Shader>("content/test.comp", en.handle(), &computeShader);

        auto texture = getFactory<Texture>()->loadGliUnmanaged("content/skybox.ktx", GL_RGBA8);

        std::cout << "Texture Size: (" << 
            texture->width() << ", " << texture->height() 
            << ")" << std::endl;

        // Make work and output buffers
        GLuint local_size_x = 64;
        GLuint num_groups_x = texture->height() / local_size_x;

        GLuint output_buffer;
        glGenBuffers(1, &output_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * 9 * num_groups_x, nullptr, GL_STREAM_READ);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
        GL_ASSERT;

        glUseProgram(computeShader->id());

        GL_ASSERT;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, output_buffer);

        GL_ASSERT;

        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->id());
        glBindImageTexture(0, texture->id(), 0, false, 0, 
            GL_READ_WRITE, GL_RGBA8);

        GL_ASSERT;

        glDispatchCompute(num_groups_x, 1, 1);

        GL_ASSERT;

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        void* buf_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        float* out_data = static_cast<float*>(buf_data);

        std::cout << "Output: " << std::endl;
        for (uint i = 0; i < 9; ++i) {
            for (uint j = 0; j < 4; ++j) {
                float result = 0.0;
                for (uint k = 0; k < num_groups_x; ++k) {
                    result += out_data[9 * 4 * k + i * 4 + j];
                }

                std::cout << result << std::endl;
            }
            std::cout << std::endl;
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &output_buffer);

        getFactory<Texture>()->unload(texture);
    }
    en.shutdown();
}