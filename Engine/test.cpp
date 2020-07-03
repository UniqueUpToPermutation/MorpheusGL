#include <iostream>
#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"
#include "cooktorrance.hpp"
#include <GLFW/glfw3.h>

using namespace Morpheus;

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		float geo[] = { 
			-1.0f, -1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};

		GLuint geoBuf;
		GLuint vertexArray;

		glGenBuffers(1, &geoBuf);
		glBindBuffer(GL_ARRAY_BUFFER, geoBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(geo), geo, GL_STATIC_DRAW);

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, geoBuf);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

		ref<CookTorranceShader> shader(nullptr);
		content().load<CookTorranceShader>("shaders/cooktorrance.json", &shader);

		glm::mat4 id = glm::identity<glm::mat4>();

		// Make a thing
		while (en.valid()) {
			en.update();
			
			int width, height;
			glfwGetFramebufferSize(en.window(), &width, &height);
			glViewport(0, 0, width, height);
			glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(shader->id());
			shader->mWorld.set(id);
			shader->mView.set(id);
			shader->mProjection.set(id);
			shader->mWorldInverseTranspose.set(id);
			glBindVertexArray(vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			glfwSwapBuffers(en.window());
		}

		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &geoBuf);
	}

	en.shutdown();
}