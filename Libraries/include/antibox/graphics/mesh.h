#pragma once
#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "texture.h"

namespace antibox {
	class Mesh
	{
	public:
		Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions);
		Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, uint32_t* elementArray, uint32_t elementCount);
		Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, float* texCoords, uint32_t* elementArray, uint32_t elementCount);
		~Mesh();

		void Bind();
		void Unbind();

		bool IsValid() { return mVao != 0; }

		inline uint32_t GetVertexCount() const { return mVertexCount; }
		inline uint32_t GetElementCount() const { return mElementCount; }

	private:
		uint32_t mVertexCount;
		uint32_t mElementCount;
		uint32_t mVao, mEbo;
		uint32_t mPositionVbo;
		uint32_t mTextCoordsVbo;
	};
}
#endif