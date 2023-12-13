#include "framebuffer.h"

#include "glad/glad.h"
#include "../core/log.h"
#include "helpers.h"

namespace antibox 
{
	Framebuffer::Framebuffer(uint32_t width, uint32_t height)
		: mFbo(0)
		, mTextureId(0)
		, mRenderbufferId(0)
		, mSize({ width, height })
		, mClearColour(1.f)
	{
		glGenFramebuffers(1, &mFbo); ANTIBOX_CHECK_GL_ERROR;
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo); ANTIBOX_CHECK_GL_ERROR;

		// Create colour texture
		glGenTextures(1, &mTextureId); ANTIBOX_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, mTextureId); ANTIBOX_CHECK_GL_ERROR;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.x, mSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); ANTIBOX_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); ANTIBOX_CHECK_GL_ERROR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); ANTIBOX_CHECK_GL_ERROR;
		glBindTexture(GL_TEXTURE_2D, 0); ANTIBOX_CHECK_GL_ERROR;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0); ANTIBOX_CHECK_GL_ERROR;

		// Create depth/stencil renderbuffer
		glGenRenderbuffers(1, &mRenderbufferId); ANTIBOX_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferId); ANTIBOX_CHECK_GL_ERROR;
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mSize.x, mSize.y); ANTIBOX_CHECK_GL_ERROR;
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRenderbufferId); ANTIBOX_CHECK_GL_ERROR;

		// Check for completeness
		int32_t completeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); ANTIBOX_CHECK_GL_ERROR;
		if (completeStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			ANTIBOX_ERRLOG("Failure to create framebuffer. Complete status: ", completeStatus);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); ANTIBOX_CHECK_GL_ERROR;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &mFbo); ANTIBOX_CHECK_GL_ERROR
		mFbo = 0;
		mTextureId = 0;
		mRenderbufferId = 0;
	}
}