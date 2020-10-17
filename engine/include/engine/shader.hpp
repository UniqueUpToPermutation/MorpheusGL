#pragma once

#include <glad/glad.h>
#include <iostream>

#include <engine/content.hpp>
#include <engine/sampler.hpp>
#include <engine/texture.hpp>

#include <engine/glslpreprocessor.hpp>

#include <glm/gtc/type_ptr.hpp>

#define SET_WORLD(loc_str) mWorld.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_VIEW(loc_str) mView.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_PROJECTION(loc_str) mProjection.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_WORLDINVTRANSPOSE(loc_str) mWorldInverseTranspose.mLoc = glGetUniformLocation(id(), loc_str)
#define SET_CASE(type, mLoc, ptr) case type: \
	GL_TYPE_<type>::setUniform(mLoc, *(GL_TYPE_<type>::C_TYPE_*)(ptr))

namespace Morpheus {

	template <GLenum uniformType>
	struct GL_TYPE_;
	template <> struct GL_TYPE_<GL_FLOAT> {
		typedef GLfloat C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1f(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform1fv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE> {
		typedef GLdouble C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1d(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform1dv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_BOOL> {
		typedef GLboolean C_TYPE_;
		typedef GLboolean C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = false;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1i(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			throw std::runtime_error("setUniformArray not supported for boolean!");
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_INT> {
		typedef GLint C_TYPE_;
		typedef GLint C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1i(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform1iv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_INT> {
		typedef GLuint C_TYPE_;
		typedef GLuint C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1ui(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform1uiv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			j.get_to(*v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC2> {
		typedef glm::fvec2 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform2f(loc, v.x, v.y);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform2fv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<float> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC3> {
		typedef glm::fvec3 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform3f(loc, v.x, v.y, v.z);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform3fv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<float> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_VEC4> {
		typedef glm::fvec4 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform4f(loc, v.x, v.y, v.z, v.w);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform4fv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
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
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform2d(loc, v.x, v.y);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform2dv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<double> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_VEC3> {
		typedef glm::dvec3 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform3d(loc, v.x, v.y, v.z);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform3dv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<double> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_VEC4> {
		typedef glm::dvec4 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform4d(loc, v.x, v.y, v.z, v.w);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform4dv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
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
		typedef GLint C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform2i(loc, v.x, v.y);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform2iv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
		}
	};
	template <> struct GL_TYPE_<GL_INT_VEC3> {
		typedef glm::ivec3 C_TYPE_;
		typedef GLint C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform3i(loc, v.x, v.y, v.z);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform3iv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
		}
	};
	template <> struct GL_TYPE_<GL_INT_VEC4> {
		typedef glm::ivec4 C_TYPE_;
		typedef GLint C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniform4i(loc, v.x, v.y, v.z, v.w);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniform4iv(loc, len, v);
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			std::vector<int32_t> e;
			j.get_to(e);
			v->x = e[0];
			v->y = e[1];
			v->z = e[2];
			v->w = e[3];
		}
	};
	template <> struct GL_TYPE_<GL_BYTE> {
		typedef GLbyte C_TYPE_;
		typedef GLbyte C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = false;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1i(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			throw std::runtime_error("GLbyte does not support arrays!");
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			int32_t e;
			j.get_to(e);
			*v = static_cast<C_TYPE_>(e);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_BYTE> {
		typedef GLubyte C_TYPE_;
		typedef GLubyte C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = false;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1ui(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			throw std::runtime_error("GLubyte does not support arrays!");
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<C_TYPE_>(e);
		}
	};
	template <> struct GL_TYPE_<GL_SHORT> {
		typedef GLshort C_TYPE_;
		typedef GLshort C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = false;
		inline static void setUniform(GLint loc, int16_t v) {
			glUniform1i(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			throw std::runtime_error("GLshort does not support arrays!");
		}
		inline static void readJson(const nlohmann::json& j, int16_t* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<C_TYPE_>(e);
		}
	};
	template <> struct GL_TYPE_<GL_UNSIGNED_SHORT> {
		typedef GLushort C_TYPE_;
		typedef GLushort C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = false;
		inline static void setUniform(GLint loc, C_TYPE_ v) {
			glUniform1ui(loc, v);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			throw std::runtime_error("GLushort does not support arrays!");
		}
		inline static void readJson(const nlohmann::json& j, C_TYPE_* v) {
			uint32_t e;
			j.get_to(e);
			*v = static_cast<uint16_t>(e);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2> {
		typedef glm::fmat2 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3> {
		typedef glm::fmat3 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix3fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4> {
		typedef glm::fmat4 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const glm::fmat4& v) {
			glUniformMatrix4fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2x3> {
		typedef glm::fmat2x3 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2x3fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2x3fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT2x4> {
		typedef glm::fmat2x4 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2x4fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2x4fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3x2> {
		typedef glm::fmat3x2 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix3x2fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3x2fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4x2> {
		typedef glm::fmat4x2 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix4x2fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4x2fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT4x3> {
		typedef glm::fmat4x3 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix4x3fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4x3fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_FLOAT_MAT3x4> {
		typedef glm::fmat3x4 C_TYPE_;
		typedef GLfloat C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix3x4fv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3x4fv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2> {
		typedef glm::dmat2 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3> {
		typedef glm::dmat3 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix3dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4> {
		typedef glm::dmat4 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix4dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2x3> {
		typedef glm::dmat2x3 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2x3dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2x3dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT2x4> {
		typedef glm::dmat2x4 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix2x4dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix2x4dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3x2> {
		typedef glm::dmat3x2 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix3x2dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3x2dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4x2> {
		typedef glm::dmat4x2 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix4x2dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4x2dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT4x3> {
		typedef glm::dmat4x3 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const C_TYPE_& v) {
			glUniformMatrix4x3dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix4x3dv(loc, len, false, v);
		}
	};
	template <> struct GL_TYPE_<GL_DOUBLE_MAT3x4> {
		typedef glm::dmat3x4 C_TYPE_;
		typedef GLdouble C_ARRAY_TYPE_;
		static constexpr bool IS_ARRAY_ENABLED = true;
		inline static void setUniform(GLint loc, const glm::dmat3x4& v) {
			glUniformMatrix3x4dv(loc, 1, false, &v[0][0]);
		}
		inline static void setUniformArray(GLint loc, const C_ARRAY_TYPE_* v, GLsizei len) {
			glUniformMatrix3x4dv(loc, len, false, v);
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
	using UNIFORM_C_TYPE_ = 
		typename GL_TYPE_<C_TO_GL_TYPE_<typename std::remove_all_extents_t<T>>::RESULT>::C_TYPE_;

	template <typename T>
	using UNIFORM_C_ARRAY_TYPE_ = 
		typename GL_TYPE_<C_TO_GL_TYPE_<typename std::remove_all_extents_t<T>>::RESULT>::C_ARRAY_TYPE_;

	template <typename T>
	constexpr GLenum UNIFORM_TYPE_() { 
		return C_TO_GL_TYPE_<typename std::remove_all_extents_t<T>>::RESULT; 
	}

	struct RendererShaderView;

	template <typename T>
	struct ShaderUniform {

		static constexpr GLenum UNIF_TYPE = UNIFORM_TYPE_<T>();

		typedef UNIFORM_C_TYPE_<T> C_TYPE_;
		typedef UNIFORM_C_ARRAY_TYPE_<T> C_ARRAY_TYPE_;

	private:
		GLint mLoc;

	public:

		inline bool valid() const {
			return mLoc != -1;
		}

		inline GLint location() const { 
			return mLoc;
		}

		inline void set(const C_TYPE_& v) const {
			GL_TYPE_<UNIF_TYPE>::setUniform(mLoc, v);
		}

		inline void set(const C_ARRAY_TYPE_ v[], GLsizei len) const {
			GL_TYPE_<UNIF_TYPE>::setUniformArray(mLoc, v, len);
		}

		inline ShaderUniform() : mLoc(-1) {
		}

		inline void find(const Shader* shader, const std::string& identifier);
		inline ShaderUniform(const Shader* shader, const std::string& identifier);

		friend void readRenderUniforms(const nlohmann::json& j, const Shader* shad, RendererShaderView* out);
	};

	template <>
	struct ShaderUniform<Texture> {
	private:
		GLint mLoc;
		GLint mUnit;

	public:
		inline bool valid() const {
			return mLoc != -1;
		}

		inline GLint location() const { 
			return mLoc;
		}

		inline GLint unit() const {
			return mUnit;
		}

		inline void set(const Texture* texture, const GLenum access) const {
			texture->bindImage(mUnit, access);
		}

		inline void setUnit(const GLint unit) {
			glUniform1i(mLoc, unit);
			mUnit = unit;
		}

		inline ShaderUniform() : mLoc(-1), mUnit(-1) {
		}

		inline void find(const Shader* shader, const std::string& identifier);
		inline ShaderUniform(const Shader* shader, const std::string& identifier);
	};

	template <>
	struct ShaderUniform<Sampler> {
	private:
		GLint mLoc;
		GLint mUnit;

	public:
		inline bool valid() const {
			return mLoc != -1;
		}

		inline GLint location() const { 
			return mLoc;
		}

		inline GLint unit() const {
			return mUnit;
		}
		
		inline void set(const Texture* texture, const Sampler* sampler) const {
			sampler->bind(mUnit);
			texture->bind(mUnit);
		}

		inline void set(const GLenum texture_target, const GLint texture, const GLint sampler) const {
			glActiveTexture(GL_TEXTURE0 + mUnit);
			glBindTexture(texture_target, texture);
			glBindSampler(mUnit, sampler);
		}

		inline void setUnit(const GLint unit) {
			glUniform1i(mLoc, unit);
			mUnit = unit;
		}

		inline ShaderUniform() : mLoc(-1), mUnit(-1) {
		}

		inline void find(const Shader* shader, const std::string& identifier);
		inline ShaderUniform(const Shader* shader, const std::string& identifier);
	};

	enum class ShaderParameterType {
		UNIFORM,
		SAMPLER
	};

	enum class ShaderType {
		VERTEX,
		FRAGMENT,
		COMPUTE,
		GEOMETRY
	};

	struct RendererShaderView {
		// The world transformation matrix
		ShaderUniform<glm::mat4> mWorld;
		// The view transformation matrix
		ShaderUniform<glm::mat4> mView;
		// The projection matrix
		ShaderUniform<glm::mat4> mProjection;
		// The world inverse transpose matrix for correct normals
		ShaderUniform<glm::mat4> mWorldInverseTranspose;
		// The eye position
		ShaderUniform<glm::vec3> mEyePosition;
		// The SH coefficients of diffuse irradiance from light probes
		ShaderUniform<glm::vec3[]> mEnvironmentDiffuseSH;
		// The current time
		ShaderUniform<float> mTime;
	};

	struct ShaderEditorView {
		std::vector<GLint> mEditorUniforms;
	};

	struct ShaderUniformAssignment {
		GLint mUniformLocation;
		GLenum mUniformType;
		uint32_t mOffset;
		GLsizei mArrayLength;
	};

	class ShaderUniformAssignments {
	public:
		std::vector<ShaderUniformAssignment> mBindings;
		std::vector<uint8_t> mData;
		uint32_t mTotalSize;

		uint32_t computeSize() const;

		void assign() const;
		ShaderUniformAssignments overwrite(const ShaderUniformAssignments& toOverwrite);

		template <typename T>
		void add(const ShaderUniform<T>& uniform, const UNIFORM_C_TYPE_<T>& value) {
			ShaderUniformAssignment binding;
			uint oldSize = mData.size();

			binding.mUniformLocation = uniform.location();
			binding.mArrayLength = 1;
			binding.mUniformType = UNIFORM_TYPE_<T>();
			binding.mOffset = oldSize;

			mBindings.emplace_back(binding);
			mData.resize(oldSize + sizeof(UNIFORM_C_TYPE_<T>));
			memcpy(&mData[oldSize], &value, sizeof(UNIFORM_C_TYPE_<T>));
		}

		template <typename T>
		void add(const ShaderUniform<T[]>& uniform, const std::vector<UNIFORM_C_TYPE_<T>>& value) {
			ShaderUniformAssignment binding;
			uint oldSize = mData.size();

			binding.mUniformLocation = uniform.mLoc;
			binding.mArrayLength = value.size();
			binding.mUniformType = UNIFORM_TYPE_<T>();
			binding.mOffset = oldSize;

			mBindings.emplace_back(binding);
			mData.resize(oldSize + value.size() * sizeof(UNIFORM_C_TYPE_<T>));
			memcpy(&mData[oldSize], &value.front(), value.size() * sizeof(UNIFORM_C_TYPE_<T>));
		}

		template <typename T>
		void add(const ShaderUniform<T[]>& uniform, const std::vector<UNIFORM_C_ARRAY_TYPE_<T>>& value) {
			add(uniform, &value[0], value.size() / (sizeof(UNIFORM_C_TYPE_<T>) / sizeof(UNIFORM_C_ARRAY_TYPE_<T>)));
		}

		template <typename T>
		void add(const ShaderUniform<T[]>& uniform, const UNIFORM_C_ARRAY_TYPE_<T> arr[], GLsizei len) {
			ShaderUniformAssignment binding;
			uint oldSize = mData.size();

			binding.mUniformLocation = uniform.location();
			binding.mArrayLength = len;
			binding.mUniformType = UNIFORM_TYPE_<T>();
			binding.mOffset = oldSize;

			mBindings.emplace_back(binding);
			mData.resize(oldSize + len * sizeof(UNIFORM_C_TYPE_<T>));
			memcpy(&mData[oldSize], arr, len * sizeof(UNIFORM_C_TYPE_<T>));
		}

		template <typename T>
		int findOffset(const ShaderUniform<T>& uniform) const {
			for (auto& binding : mBindings) {
				if (binding.mUniformLocation == uniform.location())
					return binding.mOffset;
			}
			return -1;
		}

		template <typename T>
		T& at(int offset) {
			return *(reinterpret_cast<T*>(&mData[offset]));
		}

		template <typename T>
		T* arrayAt(int offset) {
			return reinterpret_cast<T*>(&mData[offset]);
		}

		template <typename T>
		const T& at(int offset) const {
			return *(reinterpret_cast<const T*>(&mData[offset]));
		}

		template <typename T>
		const T* arrayAt(int offset) const {
			return reinterpret_cast<const T*>(&mData[offset]);
		}

		template <typename T>
		T* arrayAt(const ShaderUniform<T>& uniform) {
			int offset = findOffset(uniform);
			if (offset >= 0) 
				return arrayAt<T>(offset);
			else 
				return nullptr;
		}

		template <typename T>
		const T* arrayAt(const ShaderUniform<T>& uniform) const {
			int offset = findOffset(uniform);
			if (offset >= 0) 
				return arrayAt<T>(offset);
			else 
				return nullptr;
		}

		// This is quite slow, do not use in performance critical code
		// The better way to do this is to find the offset once and then
		// use at<T>(offset)
		template <typename T>
		void set(const ShaderUniform<T>& uniform, const T& value) {
			int offset = findOffset(uniform);
			if (offset >= 0) {
				at<T>(offset) = value;
			}
		}

		// This is quite slow, do not use in performance critical code
		// The better way to do this is to find the offset once and then
		// use at<T>(offset)
		template <typename T>
		T get(const ShaderUniform<T>& uniform) const {
			int offset = findOffset(uniform);
			if (offset >= 0) {
				return at<T>(offset);
			}
			return T();
		}

		// This is quite slow, do not use in performance critical code
		// The better way to do this is to find the offset once and then
		// use atArray<T>(offset)
		template <typename T>
		void setArray(const ShaderUniform<T>& uniform, const T values[], const uint len) {
			int offset = findOffset(uniform);
			if (offset >= 0) {
				std::memcpy(&mData[offset], values, len * sizeof(T));
			}
		}

		// This is quite slow, do not use in performance critical code
		// The better way to do this is to find the offset once and then
		// use atArray<T>(offset)
		template <typename T>
		void setArray(const ShaderUniform<T>& uniform, std::vector<T>& values) {
			setArray(uniform, &values[0], values.size());
		}
	};

	struct ShaderSamplerAssignment {
		GLint mUniformLocation;
		Texture* mTexture;
		Sampler* mSampler;
		GLint mTextureUnit;
	};

	class ShaderSamplerAssignments {
	public:
		std::vector<ShaderSamplerAssignment> mBindings;

		void assign() const;
		ShaderSamplerAssignments overwrite(const ShaderSamplerAssignments& toOverwrite);

		void add(const ShaderUniform<Sampler>& uniform, Sampler* sampler, Texture* texture);
	};

	class Shader : public INodeOwner {
	private:
		GLuint mId;
		RendererShaderView mRenderView;
		ShaderEditorView mEditorView;
		ShaderUniformAssignments mDefaultUniformAssignments;
		ShaderSamplerAssignments mDefaultSamplerAssignments;

	public:
		inline Shader() : INodeOwner(NodeType::SHADER) {
		}

		Shader* toShader() override;

		inline const ShaderUniformAssignments& defaultUniformAssignments() const {
			return mDefaultUniformAssignments;
		}
		inline const ShaderSamplerAssignments& defaultSamplerAssignments() const {
			return mDefaultSamplerAssignments;
		}
		inline const RendererShaderView& renderView() const { return mRenderView; }
		inline const ShaderEditorView& editorView() const { return mEditorView; }
		inline GLuint id() const { return mId; }

		inline void bind() const {
			glUseProgram(mId);
		}

		template <typename T>
		ShaderUniform<T> getUniform(const std::string& name) const {
			return ShaderUniform<T>(this, name);
		}

		friend class ContentFactory<Shader>;
		friend class ShaderView;
	};
	SET_NODE_ENUM(Shader, SHADER);

	class ShaderView {
	private:
		Shader* shader_;

	public:
		inline ShaderView(Shader* shader_) : shader_(shader_) { }

		inline Shader* shader() const {
			return shader_;
		}

		template <typename T>
		inline void bind(ShaderUniform<T>& u, const std::string& name) {
			u.mLoc = glGetUniformLocation(shader_->mId, name.c_str());
			if (u.mLoc < 0) 
				std::cout << "Warning: Uniform " << name << " not found!" << std::endl;
		}
	};

	template <>
	struct ContentExtParams<Shader> {
		GLSLPreprocessorConfig mConfigOverride;
	};

	struct ShaderStageSource {
		ShaderType mStage;
		std::string mSource;
	};

	template <>
	class ContentFactory<Shader> : public IContentFactory, public IGLSLSourceLoader {
	private:
		GLSLPreprocessor mPreprocessor;
		std::unordered_map<std::string, const char*> mInternalShaders;

		void readJsonMetadata(const nlohmann::json& j, Shader* shad, Node loadInto,
			const std::string& parentSrc = "");

		Shader* loadJson(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides);
		Shader* loadComp(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides);

	public:
		ContentFactory();
		~ContentFactory();

		bool tryFind(const std::string& source, std::string* contents) override;

		INodeOwner* load(const std::string& source, Node loadInto) override;
		INodeOwner* load(const std::string& source, Node loadInto, const GLSLPreprocessorConfig* overrides);
		INodeOwner* loadEx(const std::string& source, Node loadInto, const void* extParam) override;

		Shader* makeUnmanaged(const std::vector<ShaderStageSource>& sources);
		Shader* make(INodeOwner* parent, const std::vector<ShaderStageSource>& sources);
		Shader* makeUnmanaged(const std::vector<ShaderStageSource>& sources, const GLSLPreprocessorConfig* overrides);
		Shader* make(INodeOwner* parent, const std::vector<ShaderStageSource>& sources, const GLSLPreprocessorConfig* overrides);

		Shader* makeUnmanagedFromGL(GLint shaderProgram);
		void unload(INodeOwner* ref) override;

		std::string getContentTypeString() const override;

		inline GLSLPreprocessor* preprocessor() { return &mPreprocessor; }
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
	GLuint compileShader(const GLSLPreprocessorOutput& code, const ShaderType type);
	GLuint compileComputeKernel(const std::string& code);
	GLuint compileComputeKernel(const GLSLPreprocessorOutput& code);
	void printProgramLinkerOutput(GLint program);
	void printShaderCompilerOutput(GLint shader);
	void printShaderCompilerOutput(GLint shader, const GLSLPreprocessorOutput& outputs);

	inline void ShaderUniform<Sampler>::find(const Shader* shader, const std::string& identifier) {
		mLoc = glGetUniformLocation(shader->id(), identifier.c_str());
		glGetUniformiv(shader->id(), mLoc, &mUnit);
	}

	inline ShaderUniform<Sampler>::ShaderUniform(const Shader* shader, const std::string& identifier) {
		find(shader, identifier);
	}

	template <typename T>
	inline void ShaderUniform<T>::find(const Shader* shader, const std::string& identifier) {
		mLoc = glGetUniformLocation(shader->id(), identifier.c_str());
	}

	template <typename T>
	inline ShaderUniform<T>::ShaderUniform(const Shader* shader, const std::string& identifier) {
		find(shader, identifier);
	}

	inline void ShaderUniform<Texture>::find(const Shader* shader, const std::string& identifier) {
		mLoc = glGetUniformLocation(shader->id(), identifier.c_str());
		glGetUniformiv(shader->id(), mLoc, &mUnit);
	}

	inline ShaderUniform<Texture>::ShaderUniform(const Shader* shader, const std::string& identifier) {
		find(shader, identifier);
	}
}