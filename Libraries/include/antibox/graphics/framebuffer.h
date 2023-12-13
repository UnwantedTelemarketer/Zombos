#pragma once
#include <stdint.h>
#include "glm/glm.hpp"

namespace antibox 
{
	class Framebuffer
	{
	public:
		Framebuffer(uint32_t width, uint32_t height);
		~Framebuffer();

		inline uint32_t GetFBO() const { return mFbo; }
		inline uint32_t GetTextureID() const { return mTextureId; }
		inline uint32_t GetRenderbufferID() const { return mRenderbufferId; }
		inline const glm::ivec2& GetSize() { return mSize; }
		inline void SetClearColor(const glm::vec4 cc) { mClearColour = cc; }
		inline const glm::vec4& GetClearColor() { return mClearColour; }

	private:
		uint32_t mFbo;
		uint32_t mTextureId;
		uint32_t mRenderbufferId;

		glm::ivec2 mSize;
		glm::vec4 mClearColour;
	};
}