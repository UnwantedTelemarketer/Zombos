#pragma once

#include <memory>

namespace Factory
{
	static float Vertices2D[]
	{
		 0.5f,  0.5f, 0.f,
		 0.5f, -0.5f, 0.f,
		-0.5f, -0.5f, 0.f,
		-0.5f,  0.5f, 0.f
	};

	static uint32_t Elements2D[]
	{
		0, 3, 1,
		1, 3, 2
	};

	static float Vertices3D[]
	{
		-0.5f, 0.0f,  0.5f,
		-0.5f, 0.0f, -0.5f,
		 0.5f, 0.0f, -0.5f,
		 0.5f, 0.0f,  0.5f,
		 0.0f, 0.8f,  0.0f,
	};

	static uint32_t Elements3D[]
	{
		0, 1, 2,
		0, 2, 3,
		0, 1, 4,
		1, 2, 4,
		2, 3, 4,
		3, 0, 4
	};

	static float texcoords[]
	{
		1.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
		0.f, 1.f,
	};

	static const char* Default2DVert = R"(
			#version 410 core
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec2 texcoords;
			out vec2 uvs;
			uniform vec2 offset = vec2(0.5);
			uniform mat4 model = mat4(1.0);


			void main() {
				uvs = texcoords;
				vec4 transformedPosition = model * vec4(position + vec3(offset, 0.0), 1.0);
				gl_Position = transformedPosition;
			}
		)";

	static const char* Default3DVert = R"(
			#version 410 core
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec2 texcoords;
			out vec2 uvs;
			uniform vec2 offset = vec2(0.5);

			uniform mat4 camMatrix;

			void main() {
				uvs = texcoords;
				vec4 transformedPosition = camMatrix * vec4(position, 1.0);
				gl_Position = transformedPosition;
			}
		)";

	static const char* DefaultFrag2D = R"(
			#version 410 core
			out vec4 outColor;
			in vec3 vpos;
			in vec2 uvs;

			uniform vec3 color = vec3(0.0);
			uniform sampler2D tex;

			void main()
			{
				outColor = texture(tex, vec2(uvs.x, 1.0-uvs.y));
			}
		)";

	static const char* DefaultFrag3D = R"(
			#version 410 core
			out vec4 outColor;
			in vec3 vpos;
			in vec2 uvs;

			uniform sampler2D tex;

			uniform vec4 color;
			uniform float intensity;
			void main()
			{
				outColor = (texture(tex, uvs) + (color * intensity)) / 2;
				outColor[3] = 1.0;
			}
		)";
}