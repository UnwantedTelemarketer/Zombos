#include "mesh.h"
#include "helpers.h"

namespace antibox{

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions)
		: mVertexCount(vertexCount)
		, mEbo(0)
		, mElementCount(0)
	{
		glGenVertexArrays(1, &mVao); ANTIBOX_CHECK_GL_ERROR
		glBindVertexArray(mVao); ANTIBOX_CHECK_GL_ERROR

		glGenBuffers(1, &mPositionVbo); ANTIBOX_CHECK_GL_ERROR
		glBindBuffer(GL_ARRAY_BUFFER, mPositionVbo); ANTIBOX_CHECK_GL_ERROR
		glBufferData(GL_ARRAY_BUFFER, vertexCount * dimensions * sizeof(float), vertexArray, GL_STATIC_DRAW); ANTIBOX_CHECK_GL_ERROR

		glEnableVertexAttribArray(0); ANTIBOX_CHECK_GL_ERROR
		glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, 0, 0); ANTIBOX_CHECK_GL_ERROR
		glDisableVertexAttribArray(0); ANTIBOX_CHECK_GL_ERROR
		glBindBuffer(GL_ARRAY_BUFFER, 0); ANTIBOX_CHECK_GL_ERROR

		glBindVertexArray(0); ANTIBOX_CHECK_GL_ERROR
	}

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, uint32_t* elementArray, uint32_t elementCount)
		: Mesh(vertexArray, vertexCount, dimensions)
	{
		mElementCount = elementCount;
		glBindVertexArray(mVao); ANTIBOX_CHECK_GL_ERROR

		glGenBuffers(1, &mEbo); ANTIBOX_CHECK_GL_ERROR
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo); ANTIBOX_CHECK_GL_ERROR
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementCount * sizeof(uint32_t), elementArray, GL_STATIC_DRAW); ANTIBOX_CHECK_GL_ERROR

		glBindVertexArray(0); ANTIBOX_CHECK_GL_ERROR
	}

	Mesh::Mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions, float* texCoords, uint32_t* elementArray, uint32_t elementCount)
		: Mesh(vertexArray, vertexCount, dimensions, elementArray, elementCount)
	{
		glBindVertexArray(mVao); ANTIBOX_CHECK_GL_ERROR

		glGenBuffers(1, &mTextCoordsVbo); ANTIBOX_CHECK_GL_ERROR
		glBindBuffer(GL_ARRAY_BUFFER, mTextCoordsVbo); ANTIBOX_CHECK_GL_ERROR
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), texCoords, GL_STATIC_DRAW); ANTIBOX_CHECK_GL_ERROR

		glEnableVertexAttribArray(1); ANTIBOX_CHECK_GL_ERROR
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); ANTIBOX_CHECK_GL_ERROR
		//glDisableVertexAttribArray(0); ANTIBOX_CHECK_GL_ERROR
		glBindBuffer(GL_ARRAY_BUFFER, 0); ANTIBOX_CHECK_GL_ERROR

		glBindVertexArray(0); ANTIBOX_CHECK_GL_ERROR
	}

	Mesh::~Mesh() {
		glDeleteBuffers(1, &mPositionVbo); ANTIBOX_CHECK_GL_ERROR
			if (mEbo != 0) {
				glDeleteBuffers(1, &mEbo); ANTIBOX_CHECK_GL_ERROR
			}
		glDeleteVertexArrays(1, &mVao); ANTIBOX_CHECK_GL_ERROR
	}

	void Mesh::Bind() {
		glBindVertexArray(mVao); ANTIBOX_CHECK_GL_ERROR
		glEnableVertexAttribArray(0); ANTIBOX_CHECK_GL_ERROR
	}
	void Mesh::Unbind() {
		glDisableVertexAttribArray(0); ANTIBOX_CHECK_GL_ERROR
		glBindVertexArray(0); ANTIBOX_CHECK_GL_ERROR
	}
}