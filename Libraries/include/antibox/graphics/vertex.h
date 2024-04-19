#pragma once

#include <vector>

namespace antibox {
	class RawVertexBuffer {
	public:
		RawVertexBuffer();
		virtual ~RawVertexBuffer();

		inline uint32_t GetId() const { return mVbo; }
		inline uint32_t GetVertexCount() const { return mVertexCount; }
		inline uint32_t GetStride() const { return mStride; }
		inline const std::vector<uint32_t>& GetLayout() const { return mLayout; }

		void SetLayout(const std::vector<uint32_t>& layout);

		void Upload(bool dynamic = false);

		void Bind();
		void Unbind();

	protected:
		uint32_t mVbo = 0;
		uint32_t mVertexCount = 0;
		//how many bytes total
		uint32_t mStride = 0;

		//How the elements are spaced (how many of each category when separating sections in our mData)
		std::vector<uint32_t> mLayout;

		//the beginning of our arbitrary data stored without a type
		void* mData = nullptr;

		//size of the data in bytes
		uint32_t mSize = 0;
	};
}