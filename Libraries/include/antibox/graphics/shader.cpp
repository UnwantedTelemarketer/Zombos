#include "shader.h"
#include "helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../core/log.h"

namespace antibox {
	Shader::Shader(const std::string& vertex, const std::string& fragment)
	{
		mProgramID = glCreateProgram(); 

		int compileStatus;
		int status = GL_FALSE;
		char errorLog[512];

		uint32_t vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* glSource = vertex.c_str();
		glShaderSource(vertexShaderID, 1, &glSource, NULL); ANTIBOX_CHECK_GL_ERROR
		glCompileShader(vertexShaderID); ANTIBOX_CHECK_GL_ERROR
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status); ANTIBOX_CHECK_GL_ERROR
		if (status != GL_TRUE) {
			glGetShaderInfoLog(vertexShaderID, sizeof(errorLog), NULL, errorLog);
			ANTIBOX_ERRLOG("Error compiling vertex shader. OpenGL details below:", errorLog);
		}
		else
		{
			glAttachShader(mProgramID, vertexShaderID); ANTIBOX_CHECK_GL_ERROR
		}

		uint32_t fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		if (status == GL_TRUE) {
			const GLchar* glSource = fragment.c_str();
			glShaderSource(fragmentShaderID, 1, &glSource, NULL); ANTIBOX_CHECK_GL_ERROR
			glCompileShader(fragmentShaderID); ANTIBOX_CHECK_GL_ERROR
			glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status); ANTIBOX_CHECK_GL_ERROR
			if (status != GL_TRUE) {
				glGetShaderInfoLog(fragmentShaderID, sizeof(errorLog), NULL, errorLog);
				ANTIBOX_ERRLOG("Error compiling fragment shader. OpenGL details below:", errorLog);
			}
			else
			{
				glAttachShader(mProgramID, fragmentShaderID); ANTIBOX_CHECK_GL_ERROR
			}
		}

		if (status == GL_TRUE) {
			glLinkProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
			glValidateProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
			glGetProgramiv(mProgramID, GL_LINK_STATUS, &status); ANTIBOX_CHECK_GL_ERROR
			if (status != GL_TRUE) {
				glGetProgramInfoLog(mProgramID, sizeof(errorLog), NULL, errorLog);
				ANTIBOX_ERRLOG("Error linking shaders. OpenGL details below:", errorLog);
				glDeleteProgram(mProgramID);
				mProgramID = -1;
			}
		}

		glDeleteShader(vertexShaderID); ANTIBOX_CHECK_GL_ERROR
		glDeleteShader(fragmentShaderID); ANTIBOX_CHECK_GL_ERROR
	}

	Shader::~Shader() 
	{
		glUseProgram(0); ANTIBOX_CHECK_GL_ERROR
		glDeleteProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::Bind()
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::Unbind()
	{
		glUseProgram(0); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformInt(const std::string& name, int val)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniform1i(GetUniformLocation(name), val); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformFloat(const std::string& name, float val)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniform1f(GetUniformLocation(name), val); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformFloat2(const std::string& name, float val, float val2)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniform2f(GetUniformLocation(name), val, val2); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformFloat2(const std::string& name, glm::vec2& val)
	{
		SetUniformFloat2(name, val.x, val.y);
	}

	void Shader::SetUniformFloat3(const std::string& name, float val, float val2, float val3)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniform3f(GetUniformLocation(name), val, val2, val3); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformFloat3(const std::string& name, glm::vec3& val)
	{
		SetUniformFloat3(name, val.x, val.y, val.z);
	}

	void Shader::SetUniformFloat4(const std::string& name, float val, float val2, float val3, float val4)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniform4f(GetUniformLocation(name), val, val2, val3, val4); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformFloat4(const std::string& name, glm::vec4& val)
	{
		SetUniformFloat4(name, val.x, val.y, val.z, val.w);
	}

	void Shader::SetUniformMat3(const std::string& name, const glm::mat3& mat)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); ANTIBOX_CHECK_GL_ERROR
	}

	void Shader::SetUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		glUseProgram(mProgramID); ANTIBOX_CHECK_GL_ERROR
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)); ANTIBOX_CHECK_GL_ERROR
	}

	int Shader::GetUniformLocation(const std::string& name)
	{
		auto it = mUniformLocations.find(name);
		if (it == mUniformLocations.end()) {
			mUniformLocations[name] = glGetUniformLocation(mProgramID, name.c_str()); ANTIBOX_CHECK_GL_ERROR
		}
		return mUniformLocations[name];
	}
}