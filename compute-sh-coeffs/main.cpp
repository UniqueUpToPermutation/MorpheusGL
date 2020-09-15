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

        ShaderUniform<unsigned int> len(computeShader, "data_in_length");

        unsigned int length = 128;
        std::vector<unsigned int> in_data;
        for (unsigned int i = 0; i < length; ++i)
            in_data.emplace_back(i);

        // Make input buffer
        GLuint in_ssbo;
        glGenBuffers(1, &in_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, in_data.size() * sizeof(unsigned int), &in_data[0], GL_STREAM_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, in_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
        GL_ASSERT;

        glUseProgram(computeShader->id());
        len.set(in_data.size());

        glDispatchCompute(in_data.size(), 1, 1);

        GL_ASSERT;

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_ssbo);
        void* buf_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        unsigned int* out_data = static_cast<unsigned int*>(buf_data);

        std::cout << "Output: " << std::endl;
        std::cout << out_data[0] << std::endl;

        unsigned int tru = 0;
        for (auto dat : in_data)
            tru += dat;
        std::cout << "True Value:" << std::endl;
        std::cout << tru << std::endl;


        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &in_ssbo);
    }
    en.shutdown();
}