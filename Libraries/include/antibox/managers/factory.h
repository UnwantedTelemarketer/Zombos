#pragma once

#include <memory>

namespace Factory
{
	static float Vertices[]
	{
		 0.5f,  0.5f, 0.f,
		 0.5f, -0.5f, 0.f,
		-0.5f, -0.5f, 0.f,
		-0.5f,  0.5f, 0.f
	};

	static uint32_t Elements[]
	{
		0, 3, 1,
		1, 3, 2
	};

	static float texcoords[]
	{
		1.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
		0.f, 1.f,
	};

	static const char* DefaultVert = R"(
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

	/*
	#version 410 core
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec2 texcoords;
			out vec3 vpos;
			out vec2 uvs;
			uniform vec2 offset = vec2(0.5);
			uniform mat4 model = mat4(1.0);

			void main()
			{
				uvs = texcoords;
				vpos = position + vec3(offset, 0);
				gl_Position = vec4(position, 1.0);
			}*/

	static const char* DefaultFrag = R"(
			#version 410 core
			out vec4 outColor;
			in vec3 vpos;
			in vec2 uvs;

			uniform vec3 color = vec3(0.0);
			uniform float blue = 0.5f;
			uniform sampler2D tex;
			void main()
			{
				//outColor = vec4(color, 1.0);
				outColor = texture(tex, uvs);
			}
		)";
}