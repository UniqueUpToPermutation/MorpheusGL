#pragma once

#include <glad/glad.h>

#include "content.hpp"

#define DEFINE_SHADER(name) class name; REGISTER_CONTENT_BASE_TYPE(name, IShader); class name : public IShader 

#define SHADER_BODY protected: \
void init() override; \
public:

#define SHADER_UNIFORM(type, name) \
ShaderUniform<type> name;

#define BEGIN_UNIFORM_LINK(name) void name::init() {
#define LINK_UNIFORM(unif, loc_str) unif.mLoc = glGetUniformLocation(id(), loc_str)
#define END_UNIFORM_LINK }

#define REGISTER_SHADER(name, str_class_name) std::function<IShader*(void)> name_f_ptr = []() { return new name(); }; \
	shaderRegistry()[str_class_name] = name_f_ptr;

namespace Morpheus {

	template <typename T>
	struct ShaderUniform;

	template <> 
	struct ShaderUniform<float> {
		GLint mLoc;
		inline void set(const GLfloat f) { glUniform1f(mLoc, f); }
	};

	template <>
	struct ShaderUniform<double> {
		GLint mLoc;
		inline void set(const GLdouble d) { glUniform1d(mLoc, d); }
	};

	template <>
	struct ShaderUniform<glm::vec1> {
		GLint mLoc;
		inline void set(const glm::vec1& v) { glUniform1f(mLoc, v.x); }
		inline void set(const glm::dvec1& v) { glUniform1d(mLoc, v.x); }
	};

	template <>
	struct ShaderUniform<glm::vec2> {
		GLint mLoc;
		inline void set(const glm::vec2& v) { glUniform2f(mLoc, v.x, v.y); }
		inline void set(const glm::dvec2& v) { glUniform2d(mLoc, v.x, v.y); }
	};

	template <>
	struct ShaderUniform<glm::vec3> {
		GLint mLoc;
		inline void set(const glm::vec3& v) { glUniform3f(mLoc, v.x, v.y, v.z); }
		inline void set(const glm::dvec3& v) { glUniform3d(mLoc, v.x, v.y, v.z); }
	};

	template <>
	struct ShaderUniform<glm::vec4> {
		GLint mLoc;
		inline void set(const glm::vec4& v) { glUniform4f(mLoc, v.x, v.y, v.z, v.w); }
		inline void set(const glm::dvec4& v) { glUniform4d(mLoc, v.x, v.y, v.z, v.w); }
	};

	template <>
	struct ShaderUniform<glm::mat2> {
		GLint mLoc;
		inline void set(const glm::mat2& m) { glUniformMatrix2fv(mLoc, 1, false, &m[0][0]); }
		inline void set(const glm::dmat2& m) { glUniformMatrix2dv(mLoc, 1, false, &m[0][0]); }
	};

	template <>
	struct ShaderUniform<glm::mat3> {
		GLint mLoc;
		inline void set(const glm::dmat3& m) { glUniformMatrix3dv(mLoc, 1, false, &m[0][0]); }
		inline void set(const glm::mat3& m) { glUniformMatrix3fv(mLoc, 1, false, &m[0][0]); }
	};

	template <>
	struct ShaderUniform<glm::mat4> {
		GLint mLoc;
		inline void set(const glm::mat4& m) { glUniformMatrix4fv(mLoc, 1, false, &m[0][0]); }
		inline void set(const glm::dmat4& m) { glUniformMatrix4dv(mLoc, 1, false, &m[0][0]); }
	};

	enum class ShaderType {
		VERTEX,
		FRAGMENT
	};

	class IShader {
	private:
		GLuint mId;

	protected:
		virtual void init() = 0;

	public:
		inline GLuint id() const { return mId; }

		friend class ContentFactory<IShader>;
	};
	REGISTER_NODE_TYPE(IShader, NodeType::SHADER);
	REGISTER_CONTENT_BASE_TYPE(IShader, IShader);

	template <>
	class ContentFactory<IShader> : public IContentFactory {
	public:
		ContentFactory();
		ref<void> load(const std::string& source) override;
		void unload(ref<void>& ref) override;
		void dispose() override;
	};

	std::unordered_map<std::string, std::function<IShader*(void)>>& shaderRegistry();
}