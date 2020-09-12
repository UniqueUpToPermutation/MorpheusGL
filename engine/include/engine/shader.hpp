#pragma once

#include <glad/glad.h>
#include <iostream>

#include <engine/content.hpp>
#include <engine/sampler.hpp>
#include <engine/texture.hpp>

#define SET_WORLD(loc_str) mWorld.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_VIEW(loc_str) mView.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_PROJECTION(loc_str) mProjection.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_WORLDINVTRANSPOSE(loc_str) mWorldInverseTranspose.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_CASE(type, mLoc, ptr) case type: \
	GL_TYPE_<type>::setUniform(mLoc, *(GL_TYPE_<type>::C_TYPE_*)(ptr))

namespace Morpheus {

	template <GLenum>
	struct GL_TYPE_;
	
	template <> struct GL_TYPE_<GL_FLOAT> {
		typedef float C_TYPE_;
		inline static void setUniform(GLint loc, float v) {
			glUniform1f(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, float* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE> {
		typedef double C_TYPE_;
		inline static void setUniform(GLint loc, double v) {
			glUniform1d(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, double* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_BOOL> {
		typedef bool C_TYPE_;
		inline static void setUniform(GLint loc, bool v) {
			glUniform1i(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, bool* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_INT> {
		typedef int32_t C_TYPE_;
		inline static void setUniform(GLint loc, int32_t v) {
			glUniform1i(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, int32_t* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_INT> {
		typedef uint32_t C_TYPE_;
		inline static void setUniform(GLint loc, uint32_t v) {
			glUniform1ui(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, uint32_t* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC2> {
		typedef glm::fvec2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fvec2& v) {
			glUniform2f(loc, v.x, v.y);
		}
		inline static void readJson(const nlohmann::json& j, glm::fvec2* v) {
			std::vector<float> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC3> {
		typedef glm::fvec3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fvec3& v) {
			glUniform3f(loc, v.x, v.y, v.z);
		}
		inline static void readJson(const nlohmann::json& j, glm::fvec3* v) {
			std::vector<float> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC4> {
		typedef glm::fvec4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fvec4& v) {
			glUniform4f(loc, v.x, v.y, v.z, v.w);
		}
		inline static void readJson(const nlohmann::json& j, glm::fvec4* v) {
			std::vector<float> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
			v->w = e[3];
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_VEC2> {
		typedef glm::dvec2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dvec2& v) {
			glUniform2d(loc, v.x, v.y);
		}
		inline static void readJson(const nlohmann::json& j, glm::dvec2* v) {
			std::vector<double> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_VEC3> {
		typedef glm::dvec3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dvec3& v) {
			glUniform3d(loc, v.x, v.y, v.z);
		}
		inline static void readJson(const nlohmann::json& j, glm::dvec3* v) {
			std::vector<double> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_VEC4> {
		typedef glm::dvec4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dvec4& v) {
			glUniform4d(loc, v.x, v.y, v.z, v.w);
		}
		inline static void readJson(const nlohmann::json& j, glm::dvec4* v) {
			std::vector<double> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
			v->w = e[3];
		}
	};
	template <> struct GL_TYPE_<GL_INT_VEC2> {
		typedef glm::ivec2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::ivec2& v) {
			glUniform2i(loc, v.x, v.y);
		}
		inline static void readJson(const nlohmann::json& j, glm::ivec2* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_INT_VEC3> {
		typedef glm::ivec3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::ivec3& v) {
			glUniform3i(loc, v.x, v.y, v.z);
		}
		inline static void readJson(const nlohmann::json& j, glm::ivec3* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_INT_VEC4> {
		typedef glm::ivec4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::ivec4& v) {
			glUniform4i(loc, v.x, v.y, v.z, v.w);
		}
		inline static void readJson(const nlohmann::json& j, glm::ivec4* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
			v->w = e[3];
		}
	};
	template <> struct GL_TYPE_<GL_BYTE> {
		typedef int8_t C_TYPE_;
		inline static void setUniform(GLint loc, int8_t v) {
			glUniform1i(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, int8_t* v) {
			int32_t e;
			j.get_to(e);
			*v = static_cast<int8_t>(e);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_BYTE> {
		typedef uint8_t C_TYPE_;
		inline static void setUniform(GLint loc, uint8_t v) {
			glUniform1ui(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, uint8_t* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<uint8_t>(e);
		}
	};
	template <> struct GL_TYPE_<GL_SHORT> {
		typedef int16_t C_TYPE_;
		inline static void setUniform(GLint loc, int16_t v) {
			glUniform1i(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, int16_t* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<int16_t>(e);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_SHORT> {
		typedef uint16_t C_TYPE_;
		inline static void setUniform(GLint loc, uint16_t v) {
			glUniform1ui(loc, v);
		}
		inline static void readJson(const nlohmann::json& j, uint16_t* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<uint16_t>(e);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2> {
		typedef glm::fmat2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat2& v) {
			glUniformMatrix2fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3> {
		typedef glm::fmat3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat3& v) {
			glUniformMatrix3fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4> {
		typedef glm::fmat4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat4& v) {
			glUniformMatrix4fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2x3> {
		typedef glm::fmat2x3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat2x3& v) {
			glUniformMatrix2x3fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2x4> {
		typedef glm::fmat2x4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat2x4& v) {
			glUniformMatrix2x4fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3x2> {
		typedef glm::fmat3x2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat3x2& v) {
			glUniformMatrix3x2fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4x2> {
		typedef glm::fmat4x2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat4x2& v) {
			glUniformMatrix4x2fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4x3> {
		typedef glm::fmat4x3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat4x3& v) {
			glUniformMatrix4x3fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3x4> {
		typedef glm::fmat3x4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::fmat3x4& v) {
			glUniformMatrix3x4fv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2> {
		typedef glm::dmat2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat2& v) {
			glUniformMatrix2dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3> {
		typedef glm::dmat3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat3& v) {
			glUniformMatrix3dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4> {
		typedef glm::dmat4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat4& v) {
			glUniformMatrix4dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2x3> {
		typedef glm::dmat2x3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat2x3& v) {
			glUniformMatrix2x3dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2x4> {
		typedef glm::dmat2x4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat2x4& v) {
			glUniformMatrix2x4dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3x2> {
		typedef glm::dmat3x2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat3x2& v) {
			glUniformMatrix3x2dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4x2> {
		typedef glm::dmat4x2 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat4x2& v) {
			glUniformMatrix4x2dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4x3> {
		typedef glm::dmat4x3 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat4x3& v) {
			glUniformMatrix4x3dv(loc, 1, false, &v[0][0]);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3x4> {
		typedef glm::dmat3x4 C_TYPE_;
		inline static void setUniform(GLint loc, const glm::dmat3x4& v) {
			glUniformMatrix3x4dv(loc, 1, false, &v[0][0]);
		}
	};

	class GLTypeMetadata {
	public:
		static uint32_t sizeOf(GLenum type);
		static void fromJson(const nlohmann::json& j, GLenum type, void* out_ptr);
	};

	template <typename T> struct C_TO_GL_TYPE_;
	template <> struct C_TO_GL_TYPE_<int32_t> {
		static const GLenum RESULT = GL_INT;
	};
	template <> struct C_TO_GL_TYPE_<float> {
		static const GLenum RESULT = GL_FLOAT;
	};
	template <> struct C_TO_GL_TYPE_<double> {
		static const GLenum RESULT = GL_DOUBLE;
	};
	template <> struct C_TO_GL_TYPE_<bool> {
		static const GLenum RESULT = GL_BOOL;
	};
	template <> struct C_TO_GL_TYPE_<uint32_t> {
		static const GLenum RESULT = GL_UNSIGNED_INT;
	};
	template <> struct C_TO_GL_TYPE_<int16_t> {
		static const GLenum RESULT = GL_SHORT;
	};
	template <> struct C_TO_GL_TYPE_<uint16_t> {
		static const GLenum RESULT = GL_UNSIGNED_SHORT;
	};
	template <> struct C_TO_GL_TYPE_<int8_t> {
		static const GLenum RESULT = GL_BYTE;
	};
	template <> struct C_TO_GL_TYPE_<uint8_t> {
		static const GLenum RESULT = GL_UNSIGNED_BYTE;
	};
	template <> struct C_TO_GL_TYPE_<glm::fvec2> {
		static const GLenum RESULT = GL_FLOAT_VEC2;
	};
	template <> struct C_TO_GL_TYPE_<glm::fvec3> {
		static const GLenum RESULT = GL_FLOAT_VEC3;
	};
	template <> struct C_TO_GL_TYPE_<glm::fvec4> {
		static const GLenum RESULT = GL_FLOAT_VEC4;
	};
	template <> struct C_TO_GL_TYPE_<glm::dvec2> {
		static const GLenum RESULT = GL_DOUBLE_VEC2;
	};
	template <> struct C_TO_GL_TYPE_<glm::dvec3> {
		static const GLenum RESULT = GL_DOUBLE_VEC3;
	};
	template <> struct C_TO_GL_TYPE_<glm::dvec4> {
		static const GLenum RESULT = GL_DOUBLE_VEC4;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat2> {
		static const GLenum RESULT = GL_FLOAT_MAT2;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat3> {
		static const GLenum RESULT = GL_FLOAT_MAT3;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat4> {
		static const GLenum RESULT = GL_FLOAT_MAT4;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat2x3> {
		static const GLenum RESULT = GL_FLOAT_MAT2x3;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat2x4> {
		static const GLenum RESULT = GL_FLOAT_MAT2x4;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat3x2> {
		static const GLenum RESULT = GL_FLOAT_MAT3x2;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat4x2> {
		static const GLenum RESULT = GL_FLOAT_MAT4x2;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat4x3> {
		static const GLenum RESULT = GL_FLOAT_MAT4x3;
	};
	template <> struct C_TO_GL_TYPE_<glm::fmat3x4> {
		static const GLenum RESULT = GL_FLOAT_MAT3x4;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat2> {
		static const GLenum RESULT = GL_DOUBLE_MAT2;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat3> {
		static const GLenum RESULT = GL_DOUBLE_MAT3;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat4> {
		static const GLenum RESULT = GL_DOUBLE_MAT4;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat2x3> {
		static const GLenum RESULT = GL_DOUBLE_MAT2x3;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat2x4> {
		static const GLenum RESULT = GL_DOUBLE_MAT2x4;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat3x2> {
		static const GLenum RESULT = GL_DOUBLE_MAT3x2;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat4x2> {
		static const GLenum RESULT = GL_DOUBLE_MAT4x2;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat4x3> {
		static const GLenum RESULT = GL_DOUBLE_MAT4x3;
	};
	template <> struct C_TO_GL_TYPE_<glm::dmat3x4> {
		static const GLenum RESULT = GL_DOUBLE_MAT3x4;
	};

	template <typename T>
	struct ShaderUniform {
		static const GLenum UNIF_TYPE = C_TO_GL_TYPE_<T>::RESULT;
		GLint mLoc;
		inline void set(const T& v) const {
			GL_TYPE_<UNIF_TYPE>::setUniform(mLoc, v);
		}
	};

	template <>
	struct ShaderUniform<Sampler> {
		GLint mLoc;
		inline void set(const ref<Texture>& texture, const ref<Sampler>& sampler) const {
			sampler->bind(mLoc);
			texture->bind(GL_TEXTURE0 + mLoc);
		}

		inline void set(const Texture* texture, const Sampler* sampler) const {
			sampler->bind(mLoc);
			texture->bind(GL_TEXTURE0 + mLoc);
		}

		inline void set(const GLenum texture_target, const GLint texture, const GLint sampler) const {
			glActiveTexture(GL_TEXTURE0 + mLoc);
			glBindTexture(texture_target, texture);
			glBindSampler(mLoc, sampler);
		}
	};

	enum class ShaderParameterType {
		UNIFORM,
		SAMPLER
	};

	enum class ShaderType {
		VERTEX,
		FRAGMENT,
		COMPUTE
	};

	struct RendererShaderView {
		ShaderUniform<glm::mat4> mWorld;
		ShaderUniform<glm::mat4> mView;
		ShaderUniform<glm::mat4> mProjection;
		ShaderUniform<glm::mat4> mWorldInverseTranspose;
		ShaderUniform<glm::vec3> mEyePosition;

		void init();
	};

	struct ShaderEditorView {
		std::vector<GLint> mEditorUniforms;
	};

	struct ShaderUniformAssignment {
		GLint mUniformLocation;
		GLenum mUniformType;
		uint32_t mOffset;
	};

	class ShaderUniformAssignments {
	public:
		std::vector<ShaderUniformAssignment> mBindings;
		std::shared_ptr<uint8_t[]> mData;
		uint32_t mTotalSize;

		uint32_t computeSize() const;

		void assign() const;
		ShaderUniformAssignments overwrite(const ShaderUniformAssignments& toOverwrite);	
	};

	struct ShaderSamplerAssignment {
		GLint mUniformLocation;
		ref<Texture> mTexture;
		ref<Sampler> mSampler;
		GLint mBindTarget;
	};

	class ShaderSamplerAssignments {
	public:
		std::vector<ShaderSamplerAssignment> mBindings;

		void assign() const;
		ShaderSamplerAssignments overwrite(const ShaderSamplerAssignments& toOverwrite);
	};

	class Shader {
	private:
		GLuint mId;
		RendererShaderView mRenderView;
		ShaderEditorView mEditorView;
		ShaderUniformAssignments mDefaultUniformAssignments;
		ShaderSamplerAssignments mDefaultSamplerAssignments;

	public:
		inline const ShaderUniformAssignments& defaultUniformAssignments() const {
			return mDefaultUniformAssignments;
		}
		inline const ShaderSamplerAssignments& defaultSamplerAssignments() const {
			return mDefaultSamplerAssignments;
		}
		inline const RendererShaderView& renderView() const { return mRenderView; }
		inline const ShaderEditorView& editorView() const { return mEditorView; }
		inline GLuint id() const { return mId; }

		friend class ContentFactory<Shader>;
		friend class ShaderView;
	};
	SET_NODE_ENUM(Shader, SHADER);

	class ShaderView {
	private:
		ref<Shader> shader_;

	public:
		inline ShaderView(ref<Shader>& shader_) : shader_(shader_) { }

		inline ref<Shader> shader() const {
			return shader_;
		}

		template <typename T>
		inline void link(ShaderUniform<T>& u, const std::string& name) {
			u.mLoc = glGetUniformLocation(shader_->mId, name.c_str());
			if (u.mLoc < 0) 
				std::cout << "Warning: Uniform " << name << " not found!" << std::endl;
		}
	};

	template <>
	class ContentFactory<Shader> : public IContentFactory {
	private:
		void readJsonMetadata(const nlohmann::json& j, Shader* shad, Node& loadInto,
			const std::string& parentSrc = "");

	public:
		ContentFactory();
		ref<void> load(const std::string& source, Node& loadInto) override;
		ref<Shader> makeFromGL(GLint shaderProgram);
		void unload(ref<void>& ref) override;
		void dispose() override;
	};

	void readEditorUniforms(const nlohmann::json& j, const Shader* shad, 
		ShaderEditorView* out);
	void readRenderUniforms(const nlohmann::json& j, const Shader* shad, 
		RendererShaderView* out);
	void readUniformDefaults(const nlohmann::json& j, const Shader* shad,
		ShaderUniformAssignments* out);
	void loadSamplerDefaults(const nlohmann::json& j, const Shader* shad, 
		ShaderSamplerAssignments* out,
		ContentManager* content, Node parent,
		const std::string& parentSrc = "");

	GLuint compileShader(const std::string& code, const ShaderType type);
	void printProgramCompilerOutput(GLint program);
	void printShaderCompilerOutput(GLint shader);
}