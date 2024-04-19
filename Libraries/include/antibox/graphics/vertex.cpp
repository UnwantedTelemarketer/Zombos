#include "glad/glad.h"

#include "antibox/graphics/vertex.h"
#include "antibox/graphics/helpers.h"

namespace antibox {

	RawVertexBuffer::RawVertexBuffer()
	{
		glGenBuffers(1, &mVbo); ANTIBOX_CHECK_GL_ERROR;
	}

	RawVertexBuffer::~RawVertexBuffer()
	{
		glDeleteBuffers(1, &mVbo); ANTIBOX_CHECK_GL_ERROR;
	}

	void RawVertexBuffer::SetLayout(const std::vector<uint32_t>& layout)
	{
		mLayout = layout;
		mStride = 0;
		for (auto& count : layout) {
			mStride += count;
		}
	}

	void RawVertexBuffer::Upload(bool dynamic)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mVbo); ANTIBOX_CHECK_GL_ERROR;
		glBufferData(GL_ARRAY_BUFFER, mSize, mData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW); ANTIBOX_CHECK_GL_ERROR;
		glBindBuffer(GL_ARRAY_BUFFER, 0); ANTIBOX_CHECK_GL_ERROR;
	}

	void RawVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, mVbo); ANTIBOX_CHECK_GL_ERROR;
	}

	void RawVertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0); ANTIBOX_CHECK_GL_ERROR;
	}
}