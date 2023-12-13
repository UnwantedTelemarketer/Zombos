#pragma once
#ifndef SHADER_CLASS_H//if not defined as to not make two of the same
#define SHADER_CLASS_H

#include <unordered_map>
#include <string>
#include "glm/glm.hpp"

namespace antibox {
	class Shader
	{
	public:
		Shader(const std::string& vertex, const std::string& fragment);
		~Shader();

		bool IsValid() { return mProgramID != 0; }

		void Bind();
		void Unbind();

		void SetUniformInt(const std::string& name, int val);
		void SetUniformFloat(const std::string& name, float val);
		void SetUniformFloat2(const std::string& name, float val, float val2);
		void SetUniformFloat2(const std::string& name, glm::vec2& val);
		void SetUniformFloat3(const std::string& name, float val, float val2, float val3);
		void SetUniformFloat3(const std::string& name, glm::vec3& val);
		void SetUniformFloat4(const std::string& name, float val, float val2, float val3, float val4);
		void SetUniformFloat4(const std::string& name, glm::vec4& val);
		void SetUniformMat3(const std::string& name, const glm::mat3& mat);
		void SetUniformMat4(const std::string& name, const glm::mat4& mat);



	private:
		int GetUniformLocation(const std::string& name);

		uint32_t mProgramID;
		std::unordered_map<std::string, int> mUniformLocations;
	};
}

#endif