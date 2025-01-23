#pragma once
#include <string>
#include <stdint.h>

namespace antibox
{
	enum class TextureFilter
	{
		Nearest,
		Linear
	};

	class Texture
	{
	public:
		Texture(const std::string& path);
		~Texture();
		
		inline uint32_t GetId() const { return mId; }
		inline uint32_t GetWidth() const { return mWidth; }
		inline uint32_t GetHeight() const { return mHeight; }
		inline uint32_t GetNumChannels() const { return mNumChannels; }
		inline const std::string& GetPath() const { return mPath; }
		inline TextureFilter GetTextureFilter() const { return mFilter; }

		bool IsValid() { return mId != 0; }

		void Bind();
		void Unbind();

		void SetTextureFilter(TextureFilter filter);

	private:
		void LoadTexture();

	private:
		TextureFilter mFilter;

		std::string mPath;
		uint32_t mId;
		uint32_t mWidth, mHeight;
		uint32_t mNumChannels;

		unsigned char* mPixels;
	};
}