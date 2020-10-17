#include <engine/shader.hpp>
#include <engine/json.hpp>
#include <engine/shader_rc.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

using namespace nlohmann;
using namespace std;

#define SIZE_CASE(type) case type: \
	return sizeof(GL_TYPE_<type>::C_TYPE_)
#define JSON_CASE(type, j, ptr) case type: \
{ \
	GL_TYPE_<type>::C_TYPE_* ptr_cast = static_cast<GL_TYPE_<type>::C_TYPE_*>(ptr); \
	GL_TYPE_<type>::readJson(j, ptr_cast); \
	break; \
}
#define ASSIGN_CASE(type, loc, ptr, len) case type: \
{ \
	if constexpr (GL_TYPE_<type>::IS_ARRAY_ENABLED) { \
		const GL_TYPE_<type>::C_ARRAY_TYPE_* ptr_cast = static_cast<const GL_TYPE_<type>::C_ARRAY_TYPE_*>(ptr); \
		GL_TYPE_<type>::setUniformArray(loc, ptr_cast, len); \
	} else { \
		const GL_TYPE_<type>::C_TYPE_* ptr_cast = static_cast<const GL_TYPE_<type>::C_TYPE_*>(ptr); \
		GL_TYPE_<type>::setUniform(loc, *ptr_cast); \
	} \
	break; \
}

namespace Morpheus {

	uint32_t GLTypeMetadata::sizeOf(GLenum type) {
		switch (type) {
			SIZE_CASE(GL_INT);
			SIZE_CASE(GL_UNSIGNED_INT);
			SIZE_CASE(GL_SHORT);
			SIZE_CASE(GL_UNSIGNED_SHORT);
			SIZE_CASE(GL_BYTE);
			SIZE_CASE(GL_UNSIGNED_BYTE);
			SIZE_CASE(GL_BOOL);
			SIZE_CASE(GL_FLOAT);
			SIZE_CASE(GL_DOUBLE);
			SIZE_CASE(GL_FLOAT_VEC2);
			SIZE_CASE(GL_FLOAT_VEC3);
			SIZE_CASE(GL_FLOAT_VEC4);
			SIZE_CASE(GL_FLOAT_MAT2);
			SIZE_CASE(GL_FLOAT_MAT3);
			SIZE_CASE(GL_FLOAT_MAT4);
			SIZE_CASE(GL_FLOAT_MAT2x3);
			SIZE_CASE(GL_FLOAT_MAT3x2);
			SIZE_CASE(GL_FLOAT_MAT2x4);
			SIZE_CASE(GL_FLOAT_MAT4x2);
			SIZE_CASE(GL_FLOAT_MAT3x4);
			SIZE_CASE(GL_FLOAT_MAT4x3);
			SIZE_CASE(GL_DOUBLE_VEC2);
			SIZE_CASE(GL_DOUBLE_VEC3);
			SIZE_CASE(GL_DOUBLE_VEC4);
			SIZE_CASE(GL_DOUBLE_MAT2);
			SIZE_CASE(GL_DOUBLE_MAT3);
			SIZE_CASE(GL_DOUBLE_MAT4);
			SIZE_CASE(GL_DOUBLE_MAT2x3);
			SIZE_CASE(GL_DOUBLE_MAT3x2);
			SIZE_CASE(GL_DOUBLE_MAT2x4);
			SIZE_CASE(GL_DOUBLE_MAT4x2);
			SIZE_CASE(GL_DOUBLE_MAT3x4);
			SIZE_CASE(GL_DOUBLE_MAT4x3);
			SIZE_CASE(GL_INT_VEC2);
			SIZE_CASE(GL_INT_VEC3);
			SIZE_CASE(GL_INT_VEC4);
		default:
			assert(0);
			return 0;
		}
	}

	void GLTypeMetadata::fromJson(const nlohmann::json& j, GLenum type, void* out_ptr) {
		switch (type) {
			JSON_CASE(GL_INT, j, out_ptr);
			JSON_CASE(GL_UNSIGNED_INT, j, out_ptr);
			JSON_CASE(GL_SHORT, j, out_ptr);
			JSON_CASE(GL_UNSIGNED_SHORT, j, out_ptr);
			JSON_CASE(GL_BYTE, j, out_ptr);
			JSON_CASE(GL_UNSIGNED_BYTE, j, out_ptr);
			JSON_CASE(GL_BOOL, j, out_ptr);
			JSON_CASE(GL_FLOAT, j, out_ptr);
			JSON_CASE(GL_DOUBLE, j, out_ptr);
			JSON_CASE(GL_FLOAT_VEC2, j, out_ptr);
			JSON_CASE(GL_FLOAT_VEC3, j, out_ptr);
			JSON_CASE(GL_FLOAT_VEC4, j, out_ptr);
			JSON_CASE(GL_DOUBLE_VEC2, j, out_ptr);
			JSON_CASE(GL_DOUBLE_VEC3, j, out_ptr);
			JSON_CASE(GL_DOUBLE_VEC4, j, out_ptr);
			JSON_CASE(GL_INT_VEC2, j, out_ptr);
			JSON_CASE(GL_INT_VEC3, j, out_ptr);
			JSON_CASE(GL_INT_VEC4, j, out_ptr);
		default:
			assert(0);
			break;
		}
	}

	Shader* Shader::toShader() {
		return this;
	}

	ContentFactory<Shader>::ContentFactory() : mPreprocessor(this) {
		makeSourceMap(&mInternalShaders);
	}

	bool ContentFactory<Shader>::tryFind(const std::string& source, std::string* contents) {
		// Search internal shaders first
		auto it = mInternalShaders.find(source);
		if (it != mInternalShaders.end()) {
			*contents = it->second;
			return true;
		} else {
			fstream f(source);

			if (f.is_open()) {
				f.seekg(0, std::ios::end);
				contents->reserve((size_t)f.tellg());
				f.seekg(0, std::ios::beg);

				contents->assign((std::istreambuf_iterator<char>(f)),
					std::istreambuf_iterator<char>());
				f.close();

				return true;
			}
			return false;
		}
	}

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

	void substitute(const ShaderUniformAssignment& assign, std::vector<ShaderUniformAssignment>* vec) {
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
		ShaderUniformAssignments* out) {
		vector<string> names;

		out->mBindings.clear();
		
		uint32_t offset = 0u;
		for (auto& unif : j.items()) {
			std::string name = unif.key();
			const GLchar* name_ptr = name.c_str();
			GLuint index;
			glGetUniformIndices(shad->id(), 1, &name_ptr, &index);

			if (index != GL_INVALID_INDEX) {
				GLenum type;
				GLint size;
				glGetActiveUniform(shad->id(), index, 0, nullptr, &size, &type, nullptr);

				ShaderUniformAssignment assign;
				assign.mUniformLocation = glGetUniformLocation(shad->id(), name_ptr);
				assign.mUniformType = type;
				assign.mOffset = offset;
				assign.mArrayLength = 1;
				offset += GLTypeMetadata::sizeOf(type) * assign.mArrayLength;

				out->mBindings.push_back(assign);
				names.push_back(name);
			}
			else
				cout << "Warning: could not find uniform " << unif.key().c_str() << endl;
		}

		// Allocate memory block
		out->mTotalSize = out->computeSize();
		out->mData.resize(out->mTotalSize);

		for (uint32_t i = 0; i < names.size(); ++i) {
			auto& name = names[i];
			auto& binding = out->mBindings[i];

			// Read actual data from json
			GLTypeMetadata::fromJson(j[name], binding.mUniformType,
				&out->mData[binding.mOffset]);
		}
	}

	void loadSamplerDefaults(const nlohmann::json& j, const Shader* shad, ShaderSamplerAssignments* out,
		ContentManager* content, Node parent, const std::string& parentSrc) {
		out->mBindings.clear();

		string prefix_include_path = "";

		auto extract_ptr = parentSrc.find_last_of("\\/");
		if (extract_ptr != string::npos)
			prefix_include_path = parentSrc.substr(0, extract_ptr + 1);

		for (auto& unif : j.items()) {
			std::string name = unif.key();
			const GLchar* name_ptr = name.c_str();
			GLuint index;
			glGetUniformIndices(shad->id(), 1, &name_ptr, &index);
			GL_ASSERT;

			if (index != GL_INVALID_INDEX) {
				GLenum type;
				GLint size;
				GLchar name;

				glGetActiveUniform(shad->id(), index, 0, nullptr, &size, &type, &name);
				GL_ASSERT;

				if (type == GL_SAMPLER_2D || type == GL_SAMPLER_1D ||
					type == GL_SAMPLER_1D_ARRAY || type == GL_SAMPLER_2D_ARRAY ||
					type == GL_SAMPLER_CUBE || type == GL_SAMPLER_CUBE_MAP_ARRAY || 
					type == GL_SAMPLER_3D) {
					std::string samplerSrc;
					if (type == GL_SAMPLER_2D)
						samplerSrc = MATERIAL_TEXTURE_2D_DEFAULT_SAMPLER_SRC;
					else if (type == GL_SAMPLER_CUBE)
						samplerSrc = MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC;
					else
						samplerSrc = MATERIAL_TEXTURE_2D_DEFAULT_SAMPLER_SRC;
					std::string textureSrc;

					// User just provided a texture name
					if (unif.value().is_string()) {
						unif.value().get_to(textureSrc);

						if (type == GL_SAMPLER_2D || type == GL_SAMPLER_1D ||
							type == GL_SAMPLER_1D_ARRAY || type == GL_SAMPLER_2D_ARRAY || 
							type == GL_SAMPLER_3D) {
							samplerSrc = MATERIAL_TEXTURE_2D_DEFAULT_SAMPLER_SRC;
						}
						else if (type == GL_SAMPLER_CUBE || type == GL_SAMPLER_CUBE_MAP_ARRAY) {
							samplerSrc = MATERIAL_CUBEMAP_DEFAULT_SAMPLER_SRC;
						}
					}
					else if (unif.value().is_object()) {
						// User provided a texture/sampler pair
						if (unif.value().contains("sampler"))
							unif.value()["sampler"].get_to(samplerSrc);
						unif.value()["texture"].get_to(textureSrc);
					}
					ShaderSamplerAssignment assignment;

					textureSrc = prefix_include_path + textureSrc;

					assignment.mTexture = load<Texture>(textureSrc);
					assignment.mSampler = load<Sampler>(samplerSrc);

					parent.addChild(assignment.mTexture->node());
					parent.addChild(assignment.mSampler->node());

					assignment.mUniformLocation = glGetUniformLocation(shad->id(), name_ptr);
					glGetUniformiv(shad->id(), assignment.mUniformLocation, 
						&assignment.mTextureUnit); // Read the texture unit we should bind to
					GL_ASSERT;
					out->mBindings.emplace_back(assignment);
				}
				else {
					cout << "Warning: uniform " << unif.key().c_str() << " does not have acceptable sampler type!" << std::endl;
				}
			}
			else {
				cout << "Warning: could not find uniform " << unif.key().c_str() << endl;
			}
		}
	}

	void readRenderUniforms(const nlohmann::json& j, const Shader* shad, RendererShaderView* out) {
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
				else if (name == "diffuse_irradiance_sh")
					out->mEnvironmentDiffuseSH.mLoc = a;
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

	void ContentFactory<Shader>::readJsonMetadata(const nlohmann::json& j, Shader* shad, Node loadInto,
		const std::string& parentSrc) {
		if (j.contains("editor_uniforms")) {
			auto jsonEdUnif = j["editor_uniforms"];
			readEditorUniforms(jsonEdUnif, shad, &shad->mEditorView);
		}
		if (j.contains("uniform_defaults")) {
			auto jsonUnifDefaults = j["uniform_defaults"];
			readUniformDefaults(jsonUnifDefaults, shad, &shad->mDefaultUniformAssignments);
		}
		if (j.contains("renderer_uniforms")) {
			auto jsonRendererUnif = j["renderer_uniforms"];
			readRenderUniforms(jsonRendererUnif, shad, &shad->mRenderView);
		}
		if (j.contains("sampler_defaults")) {
			auto jsonUnifSamplers = j["sampler_defaults"];
			loadSamplerDefaults(jsonUnifSamplers, shad, &shad->mDefaultSamplerAssignments, 
			content(), loadInto, parentSrc);
		}
	}

	void printProgramLinkerOutput(GLint program) {
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			GLchar* compiler_log = new GLchar[len];
			glGetProgramInfoLog(program, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}
	}

	void printProgramLinkerOutput(GLint program, const GLSLPreprocessorOutput& code) {
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			std::cout << "Includes: " << std::endl;
			for (size_t i = 0; i < code.mSources.size(); ++i) {
				const auto& out = code.mSources[i];
				std::cout << "[" << i << "]: " << out << std::endl;
			}
			
			GLchar* compiler_log = new GLchar[len];
			glGetProgramInfoLog(program, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}
	}

	void printShaderCompilerOutput(GLint shader) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			GLchar* compiler_log = new GLchar[len];
			glGetShaderInfoLog(shader, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}
	}

	void printShaderCompilerOutput(GLint shader, const GLSLPreprocessorOutput& output) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
		{
			std::cout << "Includes: " << std::endl;
			for (size_t i = 0; i < output.mSources.size(); ++i) {
				const auto& out = output.mSources[i];
				std::cout << "[" << i << "]: " << out << std::endl;
			}

			GLchar* compiler_log = new GLchar[len];
			glGetShaderInfoLog(shader, len, &len, compiler_log);
			cout << compiler_log << endl;
			delete[] compiler_log;
		}
	}

	Shader* ContentFactory<Shader>::loadJson(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides) {
		json j;

		auto internalIt = mInternalShaders.find(source);
		if (internalIt != mInternalShaders.end()) {
			cout << "Loading internal shader " << source << "..." << endl;

			j = json::parse(internalIt->second);
		}
		else {
			ifstream f(source);

			cout << "Loading shader " << source << "..." << endl;

			if (!f.is_open()) {
				cout << "Failed to open " << source << "!" << endl;
				return nullptr;
			}

			f >> j;
			f.close();
		}

		// Instantiate the C++ code surrounding the shader
		Shader* shader = new Shader();

		string prefix_include_path = "";

		auto extract_ptr = source.find_last_of("\\/");
		if (extract_ptr != string::npos)
			prefix_include_path = source.substr(0, extract_ptr + 1);

		unordered_map<string, ShaderType> stringToShaderTypes;
		vector<GLuint> subShaders;
		stringToShaderTypes["compute_shader"] = ShaderType::COMPUTE;
		stringToShaderTypes["vertex_shader"] = ShaderType::VERTEX;
		stringToShaderTypes["fragment_shader"] = ShaderType::FRAGMENT;
		stringToShaderTypes["geometry_shader"] = ShaderType::GEOMETRY;

		for (auto& i : j.items()) {
			auto it = stringToShaderTypes.find(i.key());
			if (it != stringToShaderTypes.end()) {
				// Compile an Open GL shader
				string src;
				i.value().get_to(src);
				src = prefix_include_path + src;

				cout << "Compiling shader: " << src << endl;
				GLSLPreprocessorOutput preprocessor_out;
				mPreprocessor.load(src, &preprocessor_out, overrides);
				GLuint shaderId = compileShader(preprocessor_out, it->second);
				subShaders.emplace_back(shaderId);
			}
		}

		// Link the program and spit any errors to stdout
		GLuint id = glCreateProgram();
		for (auto subShader : subShaders) {
			glAttachShader(id, subShader);
		}

		glLinkProgram(id);
		printProgramLinkerOutput(id);

		// Individual shaders no longer needed
		for (auto subShader : subShaders) {
			glDetachShader(id, subShader);
			glDeleteShader(subShader);
		}

		// Set the shader ID!
		shader->mId = id;
		readJsonMetadata(j, shader, loadInto, source);

		return shader;
	}

	Shader* ContentFactory<Shader>::loadComp(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides) {
		Shader* shader = new Shader();

		vector<string> paths;
		stringstream comp_ss;

		GLSLPreprocessorOutput preprocessorOutput;
		mPreprocessor.load(source, &preprocessorOutput, overrides);

		std::cout << "Compiling compute shader: " << source << endl;
		GLint comp_id = compileShader(preprocessorOutput, ShaderType::COMPUTE);

		GLuint id = glCreateProgram();
		glAttachShader(id, comp_id);
		glLinkProgram(id);
		printProgramLinkerOutput(id, preprocessorOutput);

		glDetachShader(id, comp_id);
		glDeleteShader(comp_id);

		shader->mId = id;
		return shader;
	}

	Shader* ContentFactory<Shader>::makeUnmanaged(const std::vector<ShaderStageSource>& sources,
		const GLSLPreprocessorConfig* overrides) {
		// Instantiate the C++ code surrounding the shader
		Shader* shader = new Shader();

		vector<GLuint> subShaders;

		for (auto& src : sources) {
			// Compile an Open GL shader
			cout << "Compiling shader: " << src.mSource << endl;
			GLSLPreprocessorOutput preprocessor_out;
			mPreprocessor.load(src.mSource, &preprocessor_out, overrides);
			GLuint shaderId = compileShader(preprocessor_out, src.mStage);
			subShaders.emplace_back(shaderId);
		}

		// Link the program and spit any errors to stdout
		GLuint id = glCreateProgram();
		for (auto subShader : subShaders) {
			glAttachShader(id, subShader);
		}

		glLinkProgram(id);
		printProgramLinkerOutput(id);

		// Individual shaders no longer needed
		for (auto subShader : subShaders) {
			glDetachShader(id, subShader);
			glDeleteShader(subShader);
		}

		// Set the shader ID!
		shader->mId = id;

		// Read renderer stuff
		shader->mRenderView.mEnvironmentDiffuseSH.find(shader, "environmentDiffuseSH");
		shader->mRenderView.mEyePosition.find(shader, "eyePosition");
		shader->mRenderView.mProjection.find(shader, "projection");
		shader->mRenderView.mTime.find(shader, "time");
		shader->mRenderView.mView.find(shader, "view");
		shader->mRenderView.mWorld.find(shader, "world");
		shader->mRenderView.mWorldInverseTranspose.find(shader, "worldInverseTranspose");

		return shader;
	}

	Shader* ContentFactory<Shader>::makeUnmanaged(const std::vector<ShaderStageSource>& sources) {
		return makeUnmanaged(sources, nullptr);
	}

	Shader* ContentFactory<Shader>::make(INodeOwner* parent, const std::vector<ShaderStageSource>& sources, 
		const GLSLPreprocessorConfig* overrides) {
		Shader* shad = makeUnmanaged(sources, overrides);
		createContentNode(shad, parent);
		return shad;
	}

	Shader* ContentFactory<Shader>::make(INodeOwner* parent, const std::vector<ShaderStageSource>& sources) {
		Shader* shad = makeUnmanaged(sources);
		createContentNode(shad, parent);
		return shad;
	}

	INodeOwner* ContentFactory<Shader>::load(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides) {
		size_t loc = source.rfind('.');
		if (loc != std::string::npos) {
			auto ext = source.substr(loc);
			if (ext == ".json") {
				return loadJson(source, loadInto, overrides);
			}
			else if (ext == ".comp") {
				return loadComp(source, loadInto, overrides);
			}
			else {
				std::runtime_error("Shader format not supported!");
			}
		}
		std::runtime_error("Shader has no extension!");
		return nullptr;
	}

	INodeOwner* ContentFactory<Shader>::load(const std::string& source, Node loadInto) {
		return load(source, loadInto, nullptr);
	}

	INodeOwner* ContentFactory<Shader>::loadEx(const std::string& source, Node loadInto, const void* extParam) {
		const auto& config = static_cast<const ContentExtParams<Shader>*>(extParam)->mConfigOverride;
		return load(source, loadInto, &config);
	}

	Shader* ContentFactory<Shader>::makeUnmanagedFromGL(GLint shaderProgram) {
		Shader* shad = new Shader();
		shad->mId = shaderProgram;
		return shad;
	}

	void ContentFactory<Shader>::unload(INodeOwner* ref) {
		Shader* shad = ref->toShader();
		glDeleteProgram(shad->mId);
		delete shad;
	}

	ContentFactory<Shader>::~ContentFactory() {
	}

	GLuint compileComputeKernel(const std::string& code) {
		auto id = compileShader(code, ShaderType::COMPUTE);

		auto program = glCreateProgram();
		glAttachShader(program, id);
		glLinkProgram(program);
		printProgramLinkerOutput(program);

		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		glDeleteShader(id);
		if (len > 0) {
			glDeleteProgram(program);
			return 0;
		} else {
			return program;
		}
	}
	
	GLuint compileComputeKernel(const GLSLPreprocessorOutput& code) {
		auto id = compileShader(code, ShaderType::COMPUTE);

		auto program = glCreateProgram();
		glAttachShader(program, id);
		glLinkProgram(program);
		printProgramLinkerOutput(program);

		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		glDeleteShader(id);
		if (len > 0) {
			glDeleteProgram(program);
			return 0;
		} else {
			return program;
		}
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
		case ShaderType::COMPUTE:
			shader_type = GL_COMPUTE_SHADER;
			break;
		case ShaderType::GEOMETRY:
			shader_type = GL_GEOMETRY_SHADER;
			break;
		default:
			throw std::runtime_error("Shader type not recognized!");
			break;
		}

		id = glCreateShader(shader_type);

		const GLchar* ptr = code.c_str();
		glShaderSource(id, 1, &ptr, nullptr);
		glCompileShader(id);
		printShaderCompilerOutput(id);
		
		return id;
	}

	GLuint compileShader(const GLSLPreprocessorOutput& code, const ShaderType type) {
		GLuint id;
		GLenum shader_type;

		switch (type) {
		case ShaderType::VERTEX:
			shader_type = GL_VERTEX_SHADER;
			break;
		case ShaderType::FRAGMENT:
			shader_type = GL_FRAGMENT_SHADER;
			break;
		case ShaderType::COMPUTE:
			shader_type = GL_COMPUTE_SHADER;
			break;
		case ShaderType::GEOMETRY:
			shader_type = GL_GEOMETRY_SHADER;
			break;
		default:
			throw std::runtime_error("Shader type not recognized!");
			break;
		}

		id = glCreateShader(shader_type);

		const char* src = code.mContent.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);
		printShaderCompilerOutput(id, code);
		
		return id;
	}

	uint32_t ShaderUniformAssignments::computeSize() const
	{
		uint32_t size = 0u;
		for (auto& binding : mBindings) {
			size += static_cast<uint32_t>(GLTypeMetadata::sizeOf(binding.mUniformType)) * binding.mArrayLength;
		}
		return size;
	}

	void ShaderUniformAssignments::assign() const
	{
		for (auto& binding : mBindings) {
			const void* ptr = &mData[binding.mOffset];
			switch (binding.mUniformType) {
				ASSIGN_CASE(GL_INT, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_UNSIGNED_INT, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_SHORT, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_UNSIGNED_SHORT, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_BYTE, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_UNSIGNED_BYTE, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_BOOL, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_VEC2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_VEC3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_VEC4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT2x3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT3x2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT2x4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT4x2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT3x4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_FLOAT_MAT4x3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_VEC2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_VEC3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_VEC4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT2x3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT3x2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT2x4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT4x2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT3x4, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_DOUBLE_MAT4x3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_INT_VEC2, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_INT_VEC3, binding.mUniformLocation, ptr, binding.mArrayLength);
				ASSIGN_CASE(GL_INT_VEC4, binding.mUniformLocation, ptr, binding.mArrayLength);
			default:
				assert(0);
				break;
			}
		}
	}

	ShaderUniformAssignments ShaderUniformAssignments::overwrite(const ShaderUniformAssignments& toOverwrite)
	{
		if (toOverwrite.mBindings.size() == 0)
			return *this;

		ShaderUniformAssignments result;
		for (auto& binding : mBindings) {
			result.mBindings.push_back(binding);
		}

		vector<uint32_t> carryOverIndices;
		size_t initial_bindings = mBindings.size();
		uint32_t offset = result.computeSize();
		for (uint32_t i = 0; i < toOverwrite.mBindings.size(); ++i) {
			auto& copy_binding = toOverwrite.mBindings[i];
			ShaderUniformAssignment binding = copy_binding;
			binding.mOffset += offset;
			bool bAdd = true;
			for (uint32_t i = 0; i < mBindings.size(); ++i) {
				auto& cmp_binding = mBindings[i];
				if (cmp_binding.mUniformLocation == binding.mUniformLocation)
					bAdd = false;
			}
			if (bAdd) {
				result.mBindings.push_back(binding);
				carryOverIndices.push_back(i);
			}
		}

		result.mTotalSize = result.computeSize();
		result.mData.resize(result.mTotalSize);
		std::memcpy(&result.mData[0], &mData[0], offset);

		for (size_t i = mBindings.size(); i < result.mBindings.size(); ++i) {
			auto& binding = result.mBindings[i];
			auto& original_binding = toOverwrite.mBindings[carryOverIndices[i - mBindings.size()]];
			std::memcpy(&result.mData[offset], &toOverwrite.mData[original_binding.mOffset], 
				GLTypeMetadata::sizeOf(original_binding.mUniformType) * original_binding.mArrayLength);
		}
		return result;
	}

	void ShaderSamplerAssignments::add(const ShaderUniform<Sampler>& uniform, Sampler* sampler, Texture* texture) {
		ShaderSamplerAssignment assign;
		assign.mUniformLocation = uniform.location();
		assign.mSampler = sampler;
		assign.mTexture = texture;
		assign.mTextureUnit = uniform.unit();
		mBindings.emplace_back(assign);
	}

	ShaderSamplerAssignments ShaderSamplerAssignments::overwrite(const ShaderSamplerAssignments& toOverwrite) {
		if (toOverwrite.mBindings.size() == 0)
			return *this;

		ShaderSamplerAssignments result;
		for (auto& binding : mBindings) {
			result.mBindings.push_back(binding);
		}

		for (uint32_t i = 0; i < toOverwrite.mBindings.size(); ++i) {
			auto& copy_binding = toOverwrite.mBindings[i];
			bool bAdd = true;
			for (uint32_t i = 0; i < mBindings.size(); ++i) {
				auto& cmp_binding = mBindings[i];
				if (cmp_binding.mUniformLocation == copy_binding.mUniformLocation)
					bAdd = false;
			}
			if (bAdd) {
				result.mBindings.push_back(copy_binding);
			}
		}

		return result;
	}

	void ShaderSamplerAssignments::assign() const
	{
		for (uint i = 0, size = mBindings.size(); i < size; ++i) {
			auto& binding = mBindings[i];
			binding.mTexture->bind(i);
			binding.mSampler->bind(i);
			glUniform1i(binding.mUniformLocation, i);
		}
	}

	std::string ContentFactory<Shader>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(Shader);
	}
}