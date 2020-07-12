#include "engine.hpp"
#include "content.hpp"
#include "shader.hpp"
#include "gui.hpp"
#include "material.hpp"
#include "staticmesh.hpp"
#include "scene.hpp"

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

using namespace Morpheus;
using namespace glm;

nanogui::Color clr(0.5f, 0.5f, 1.0f, 1.0f);

class GuiTest : public GuiBase {
protected:
	nanogui::FormHelper* gui;

	virtual void initGui() override {
		gui = new nanogui::FormHelper(mScreen);
		nanogui::ref<nanogui::Window> window = gui->addWindow(nanogui::Vector2i(10, 10), "Set Background Color");

		gui->addVariable("Background Color", clr);

		mScreen->setVisible(true);
		mScreen->performLayout();
		window->center();
	}

public:
	virtual void dispose() override {
		delete gui;

		GuiBase::dispose();
	}
};
SET_BASE_TYPE(GuiTest, GuiBase);

void makeTriangle(Node& sceneNode, Scene* scene) {
	Node materialNode = content()->load<Material>("basicmat.json");
	Node staticMesh = content()->load<StaticMesh>("staticmesh.json");

	// Make a triangle
	float geo_verts[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	uint32_t idx[] = {
		0, 1, 2
	};

	GLuint vbo;
	GLuint ibo;
	GLuint vao;

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geo_verts), geo_verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	auto geoFactory = content()->getFactory<Geometry>();
	auto geoNode = geoFactory->makeGeometry(vao, vbo, ibo, GL_TRIANGLES, 3, GL_UNSIGNED_INT,
		BoundingBox{ zero<vec3>(), zero<vec3>() });
	auto meshFactory = content()->getFactory<StaticMesh>();
	auto meshNode = meshFactory->makeStaticMesh(materialNode, geoNode);
	auto transformNode = scene->makeIdentityTransform();

	// Add the mesh to the scene
	sceneNode.addChild(transformNode);
	// Every mesh must be the child of a transform
	transformNode.addChild(meshNode);
}

int main() {

	Engine en;

	if (en.startup("config.json").isSuccess()) {

		auto* scene = new Scene();
		Node sceneNode = graph()->addNode(scene, engine()->handle());

		//auto* gui = new GuiTest();
		//gui->init();

		//Node guiNode = graph()->addNode(gui, sceneNode);
		Node meshNode = content()->load<StaticMesh>("staticmesh.json");
		Node transform = scene->makeIdentityTransform();

		sceneNode.addChild(transform);
		transform.addChild(meshNode);

		auto geometry = StaticMesh::getGeometry(meshNode);
		auto aabb = geometry->boundingBox();
		float len = length(aabb.mUpper - aabb.mLower);
		auto camera = new PerspectiveLookAtCamera();
		
		graph()->addNode(camera, sceneNode);

		print(engine()->node());

		size_t k = 0;
		// Make a thing
		while (en.valid()) {
			en.update();

			glClearColor(clr.r(), clr.g(), clr.b(), 1.0f);
			en.renderer()->draw(sceneNode);

			double theta = (double)k / 100.0;
			camera->mPosition = len * vec3(cos(theta), 0.0f, sin(theta));

			glfwSwapBuffers(en.window());
			++k;
		}
	}

	en.shutdown();
}