#include "shader.hpp"
#include "json.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace nlohmann;
using namespace std;

namespace Morpheus {

	ContentFactory<Shader>::ContentFactory() {
	}

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

		size_t start = 0;

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

				auto extract_ptr = path.find_last_of("\\/");
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

	void substitute(const ShaderVecUniformAssignment& assign, std::vector<ShaderVecUniformAssignment>* vec) {
		bool bFound = false;
		for (auto& oldAssign : *vec) {
			if (oldAssign.mUniformLocation == assign.mUniformLocation) {
				oldAssign = assign;
				bFound = true;
				break;
			}
		}

		if (!bFound)
			vec->push_back(assign);
	}
	
	void readUniformDefaults(const nlohmann::json& j, const Shader* shad, 
		ShaderUniformAssignments* out, bool bAppend) {
		for (auto& unif : j.items()) {
			std::string name = unif.key();
			GLint a = glGetUniformLocation(shad->id(), name.c_str());
			if (a >= 0) {
				GLint size;
				GLenum type;

				glGetActiveUniform(shad->id(), a, 0, nullptr, &size, &type, nullptr);

				switch (type) {
				case GL_FLOAT:
				case GL_DOUBLE:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					float dat;
					unif.value().get_to(dat);
					assign.mUniformValue.vf1 = dat;

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
					break;
				}
				case GL_FLOAT_VEC2:
				case GL_DOUBLE_VEC2:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					std::vector<float> dat;
					unif.value().get_to(dat);
					assign.mUniformValue.vf2.x = dat[0];
					assign.mUniformValue.vf2.y = dat[1];

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
				}
					break;
				case GL_FLOAT_VEC3:
				case GL_DOUBLE_VEC3:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					std::vector<float> dat;
					unif.value().get_to(dat);
					assign.mUniformValue.vf3.x = dat[0];
					assign.mUniformValue.vf3.y = dat[1];
					assign.mUniformValue.vf3.z = dat[2];

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
				}
					break;
				case GL_FLOAT_VEC4:
				case GL_DOUBLE_VEC4:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					std::vector<float> dat;
					unif.value().get_to(dat);
					assign.mUniformValue.vf4.x = dat[0];
					assign.mUniformValue.vf4.y = dat[1];
					assign.mUniformValue.vf4.z = dat[2];
					assign.mUniformValue.vf4.w = dat[3];

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
				}
					break;
				case GL_BOOL:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					bool dat;
					unif.value().get_to(dat);
					assign.mUniformValue.b = dat;

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
				}
					break;
				case GL_INT:
				{
					ShaderVecUniformAssignment assign;
					assign.mUniformLocation = a;
					assign.mUniformType = type;
					int dat;
					unif.value().get_to(dat);
					assign.mUniformValue.i = dat;

					if (bAppend)
						substitute(assign, &out->mVecs);
					else
						out->mVecs.push_back(assign);
				}
					break;
				default:
					cout << "Warning: uniform_default bindings don't support type " << a << endl;
					break;
				}
			}
			else
				cout << "Warning: could not find uniform " << unif.key().c_str() << endl;
		}
	}

	void readRenderUniforms(const nlohmann::json& j, const Shader* shad, ShaderRenderView* out) {
		out->init();

		for (auto& unif : j.items()) {
			std::string name = unif.key();
			std::string unifName = unif.value();
			GLint a = glGetUniformLocation(shad->id(), unifName.c_str());
			if (a >= 0) {
				if (name == "eye_position") 
					out->mEyePosition.mLoc = a;
				else if (name == "world") 
					out->mWorld.mLoc = a;
				else if (name == "world_inverse_transpose")
					out->mWorldInverseTranspose.mLoc = a;
				else if (name == "view") 
					out->mView.mLoc = a;
				else if (name == "projection") 
					out->mProjection.mLoc = a;
				else 
					cout << "Warning: renderer uniform binding " << name << " not recognized!" << endl;
				
			}
			else
				cout << "Warning: could not find renderer uniform " << unif.key().c_str() << endl;
		}
	}

	void readEditorUniforms(const nlohmann::json& j, const Shader* shad, ShaderEditorView* out) {
		std::vector<std::string> unifNames;
		j.get_to(unifNames);
		for (auto& name : unifNames) {
			GLint a = glGetUniformLocation(shad->id(), name.c_str());
			if (a >= 0)
				out->mEditorUniforms.push_back(a);
			else
				cout << "Warning: could not find editor uniform " << name << endl;
		}
	}

	void ContentFactory<Shader>::readJsonMetadata(const nlohmann::json& j, Shader* shad) {
		auto jsonEdUnif = j["editor_uniforms"];
		auto jsonUnifDefaults = j["uniform_defaults"];
		auto jsonRendererUnif = j["renderer_uniforms"];

		readEditorUniforms(jsonEdUnif, shad, &shad->mEditorView);
		readUniformDefaults(jsonUnifDefaults, shad, &shad->mDefaultAssignments);
		readRenderUniforms(jsonRendererUnif, shad, &shad->mRenderView);
	}

	ref<void> ContentFactory<Shader>::load(const std::string& source, Node& loadInto) {
		json j;
		ifstream f(source);

		cout << "Loading Shader " << source << "..." << endl;

		if (!f.is_open()) {
			ref<void> r(nullptr);
			return r;
		}

		f >> j;
		f.close();

		// Instantiate the C++ code surrounding the shader
		Shader* shader = new Shader();

		string prefix_include_path = "";

		auto extract_ptr = source.find_last_of("\\/");
		if (extract_ptr != string::npos)
			prefix_include_path = source.substr(0, extract_ptr + 1);

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
		readJsonMetadata(j, shader);

		ref<void> r(shader);
		return r;
	}

	void ContentFactory<Shader>::unload(ref<void>& ref) {
		Shader* shad = ref.as<Shader>().get();
		glDeleteProgram(shad->mId);
		delete shad;
	}

	void ContentFactory<Shader>::dispose() {

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
	void ShaderRenderView::init()
	{
		mWorld.mLoc = -1;
		mView.mLoc = -1;
		mProjection.mLoc = -1;
		mWorldInverseTranspose.mLoc = -1;
		mEyePosition.mLoc = -1;
	}
}