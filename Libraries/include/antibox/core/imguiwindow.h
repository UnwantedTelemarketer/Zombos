#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace antibox 
{
	struct ImguiWindowProperties
	{
		bool MoveTitleBarOnly = true;
		bool DockingEnabled = true;
		bool IsViewportEnabled = false;
		const char* fontPath = "c:\\Windows\\Fonts\\Arial.ttf";
		float fontSize = 16.f;
	};

	class ImguiWindow 
	{
	public:
		ImguiWindow() {}
		~ImguiWindow() {}
		ImFont* mainFont;

		void Create(const ImguiWindowProperties& props);
		void Shutdown();

		bool WantCaptureMouse();
		bool WantCaptureKeyboard();

		void BeginRender();
		void EndRender();
	};
}