#include "shader.hpp"
#include "basic_shader.hpp"
#include "json.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace nlohmann;
using namespace std;

namespace Morpheus {

	IShader* makeShader(const std::string& className);
	GLuint compileShader(const std::string& code, const ShaderType type);

	void preprocessor(const string& path, vector<string>& paths, stringstream& builder) {

		if (std::count(paths.begin(), paths.end(), path) > 0) {
			cout << path << ": WARNING: cyclical include detected!" << endl;
			return;
		}

		ifstream f(path);

		if (!f.is_open()) {
			cout << path << ": ERROR: could not open!" << endl;
			return;
		}

		std::string code;

		f.seekg(0, std::ios::end);
		code.reserve((size_t)f.tellg());
		f.seekg(0, std::ios::beg);

		code.assign((std::istreambuf_iterator<char>(f)),
			std::istreambuf_iterator<char>());
		f.close();

		uint32_t start = 0;

		while (start < code.length()) {
			auto found = code.find("#include", start);

			if (found == string::npos) {
				builder << &code[start];
				break;
			}
			else {
				builder << code.substr(start, found - start);
				auto path_start = code.find("\"", found);
				auto path_end = code.find("\"", path_start + 1);

				string include_path = code.substr(path_start + 1, path_end - path_start - 1);
				string prefix_include_path = "";

				auto extract_ptr = path.rfind('/');
				if (extract_ptr != string::npos)
					prefix_include_path = path.substr(0, extract_ptr+1);

				include_path = prefix_include_path + include_path;

				paths.push_back(path);
				preprocessor(include_path, paths, builder);
				paths.pop_back();

				start = path_end + 1;
			}
		}
	}

	ContentFactory<IShader>::ContentFactory()
	{
		REGISTER_SHADER(BasicShader, "basic");
	}

	ref<void> ContentFactory<IShader>::load(const std::string& source) {
		json j;
		ifstream f(source);

		cout << "Loading Shader " << source << endl;

		if (!f.is_open()) {
			ref<void> r(nullptr);
			return r;
		}

		f >> j;
		f.close();

		// Instantiate the C++ code surrounding the shader
		IShader* shader = makeShader(j["shader_class"]);

		string prefix_include_path = "";

		auto extract_ptr = source.rfind('/');
		if (extract_ptr != string::npos)
			prefix_include_path = source.substr(0, extract_ptr+1);

		// Compile an Open GL shader
		string vertex_src = j["vertex_shader"];
		string frag_src = j["fragment_shader"];
		vertex_src = prefix_include_path + vertex_src;
		frag_src = prefix_include_path + frag_src;

		vector<string> paths;
		stringstream vertex_ss;
		stringstream frag_ss;

		preprocessor(vertex_src, paths, vertex_ss);
		preprocessor(frag_src, paths, frag_ss);

		cout << "Compiling Vertex Shader: " << vertex_src << endl;
		GLuint vertex_id = compileShader(vertex_ss.str(), ShaderType::VERTEX);
		cout << "Compiling Fragment Shader: " << frag_src << endl;
		GLuint frag_id = compileShader(frag_ss.str(), ShaderType::FRAGMENT);

		// Link the program and spit any errors to stdout
		GLuint id = glCreateProgram();
		glAttachShader(id, vertex_id);
		glAttachShader(id, frag_id);
		glLinkProgram(id);

		GLint len;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			GLchar* compiler_log = new GLchar[len];
			glGetProgramInfoLog(id, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}

		// Shader no longer needed
		glDetachShader(id, vertex_id);
		glDetachShader(id, frag_id);

		glDeleteShader(vertex_id);
		glDeleteShader(frag_id);

		// Set the shader ID!
		shader->mId = id;

		ref<void> r(shader);
		return r;
	}

	void ContentFactory<IShader>::unload(ref<void>& ref) {
		IShader* shad = ref.as<IShader>().get();
		glDeleteProgram(shad->mId);
		delete shad;
	}

	void ContentFactory<IShader>::dispose() {

	}

	GLuint compileShader(const std::string& code, const ShaderType type) {
		GLuint id;
		GLenum shader_type;

		switch (type) {
		case ShaderType::VERTEX:
			shader_type = GL_VERTEX_SHADER;
			break;
		case ShaderType::FRAGMENT:
			shader_type = GL_FRAGMENT_SHADER;
			break;
		}

		id = glCreateShader(shader_type);

		const GLchar* ptr = code.c_str();
		glShaderSource(id, 1, &ptr, nullptr);
		glCompileShader(id);

		GLint len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			GLchar* compiler_log = new GLchar[len];
			glGetShaderInfoLog(id, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}
		return id;
	}

	std::unordered_map<std::string, std::function<IShader*(void)>> registry;
	std::unordered_map<std::string, std::function<IShader*(void)>>& shaderRegistry()
	{
		return registry;
	}

	IShader* makeShader(const std::string& className) {
		return registry[className]();
	}
}