#include "texture.h"
#include "helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "glad/glad.h"

namespace antibox
{
	Texture::Texture(const std::string& path)
		: mPath(path)
		, mWidth(0)
		, mHeight(0)
		, mNumChannels(0)
		, mPixels(nullptr)
		, mFilter(TextureFilter::Nearest)
	{
		int width, height, numChannels;
		mPixels = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
		if (mPixels)
		{
			mWidth = (uint32_t)width;
			mHeight = (uint32_t)height;
			mNumChannels = (uint32_t)numChannels;
		}

		LoadTexture();
	}

	Texture::~Texture()
	{
		stbi_image_free(mPixels);
		mPixels = nullptr;
	}

	void Texture::Bind()
	{
		//Bind a texture with the ID
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mId);
	}

	void Texture::Unbind()
	{
		//Unbind the texture, free up the memory
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	

	void Texture::LoadTexture()
	{
		//Generate a texture and load it into OpenGL
		glGenTextures(1, &mId);
		glBindTexture(GL_TEXTURE_2D, mId);

		//We need to get the amount of channels and decide whether it has alpha or not
		GLenum dataFormat = 0;
		if (mNumChannels == 4)
		{
			dataFormat = GL_RGBA;
		}
		else if (mNumChannels == 3)
		{
			dataFormat = GL_RGB;
		}
		if (mPixels && dataFormat == 0)
		{
			Console::Log("Texture was loaded but used an invalid texture format (wrong number of channels).", text::red, __LINE__);
		}

		//if we have loaded an image correctly and we have the right channels
		if (mPixels && dataFormat != 0)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, mWidth, mHeight, 0, dataFormat, GL_UNSIGNED_BYTE, mPixels);

			SetTextureFilter(mFilter);
			Console::Log(("Loaded texture from " + mPath + "."), text::green, __LINE__);
		}
		else 
		{
			float pixels[] = {
				1.f, 0.f, 1.f,		0.f, 0.f, 0.f,
				0.f, 0.f, 0.f,		1.f, 0.f, 1.f,
			};

			mWidth = 2;
			mHeight = 2;
			mNumChannels = 3;

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, pixels);
			SetTextureFilter(TextureFilter::Nearest);
			Console::Log("Unable to load " + mPath + ". Loading default checkerboard texture. ", text::red, __LINE__);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::SetTextureFilter(TextureFilter filter)
	{
		mFilter = filter;

		glBindTexture(GL_TEXTURE_2D, mId);
		switch (mFilter)
		{
		case TextureFilter::Linear:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); ANTIBOX_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); ANTIBOX_CHECK_GL_ERROR;
			break;
		case TextureFilter::Nearest:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); ANTIBOX_CHECK_GL_ERROR;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); ANTIBOX_CHECK_GL_ERROR;
			break;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

}