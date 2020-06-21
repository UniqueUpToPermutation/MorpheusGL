#ifndef CORE_H_
#define CORE_H_

#include "json.hpp"
#include "digraph.hpp"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

struct GLFWwindow;

namespace Morpheus {
	enum class NodeType {
		ENGINE = 0,
		SCENE = 1,
		ENTITY = 2,
		TRANSFORM = 3,
		REGION = 4,
		GEOMETRY = 5,
		MATERIAL = 6,
		SHADER = 7,
		TEXTURE_2D = 8,
		TEXTURE_1D = 9,
		TEXTURE_3D = 10,
		TEXTURE_2D_ARRAY = 11,
		CUBE_MAP = 12,
		STATIC_MESH = 13
	};

	struct EngineGraphData {
		NodeType type;
		void* ptr;
		uint32_t poolOffset;
	};

	struct Transform {
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rotation;
		glm::mat4 cache;
	};
	
	enum ErrorCode {
		SUCCESS,
		FAIL_GLFW_INIT,
		FAIL_GLFW_WINDOW_INIT
	};

	class Error {
	public:
		ErrorCode code;
		std::string message;
		std::string source;

		bool isSuccess() const {
			return code == ErrorCode::SUCCESS;
		}

		std::string str() const {
			return source + ": " + message;
		}

		inline explicit Error(const ErrorCode code) : code(code) { }
	};

	class Engine {
	private:
		GLFWwindow* window;
		nlohmann::json config_;
		Digraph graph_;
		DigraphDataView<EngineGraphData> nodeData_;
		bool bValid;

	public:
		inline nlohmann::json& config() { return config_; }
		inline Digraph& graph() { return graph_; }
		inline DigraphDataView<EngineGraphData>& nodeData() { return nodeData_; }

		Engine();

		Error startup(const std::string& configPath);
		void update();
		void render();
		void shutdown();
		void exit();
		bool valid() const;
	};
}

#endif