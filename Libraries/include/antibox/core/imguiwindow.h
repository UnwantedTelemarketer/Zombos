#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace antibox 
{
	struct FontData
	{
		ImFont* fontFile;
		std::string fontPath;
	};

	struct ImguiWindowProperties
	{
		bool MoveTitleBarOnly = true;
		bool DockingEnabled = true;
		bool IsViewportEnabled = false;
		std::vector<std::string> fontPaths;
		std::vector<std::string> fontNames;
		float fontSize = 16.f;
	};

	class ImguiWindow 
	{
	public:
		ImguiWindow() {}
		~ImguiWindow() {}
		std::unordered_map<std::string, FontData> fonts;

		void Create(const ImguiWindowProperties& props);
		void Shutdown();

		void AddFont(const std::string& filepath, const std::string& fontname);

		bool WantCaptureMouse();
		bool WantCaptureKeyboard();

		void UpdateScale();

		void BeginRender();
		void EndRender();
	};
}