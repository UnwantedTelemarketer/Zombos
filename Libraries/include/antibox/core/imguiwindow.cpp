#include "imguiwindow.h"

#include "engine.h"

#define ICO_SMILE "\xe2\x98\xba"
#define ICON_MIN_FA 0xf000
#define ICON_MAX_16_FA 0xf2e0
#define ICON_MAX_FA 0xf2e0

namespace antibox
{
	void ImguiWindow::Create(const ImguiWindowProperties& props)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigWindowsMoveFromTitleBarOnly = props.MoveTitleBarOnly;
		if(props.DockingEnabled) {
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		}
		if (props.IsViewportEnabled) {
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}
		(void)io; 
		io.Fonts->AddFontDefault();

		ImVector<ImWchar> ranges;
		ImFontGlyphRangesBuilder builder;
		builder.AddText(ICO_SMILE);                        // Add a string (here "Hello world" contains 7 unique characters)
		builder.AddChar(0xe298ba);                               // Add a specific character
		builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());// Add one of the default ranges
		builder.AddRanges(io.Fonts->GetGlyphRangesDefault());// Add one of the default ranges
		builder.BuildRanges(&ranges);
			

		for (size_t i = 0; i < props.fontPaths.size(); i++)
		{
			ImFont* newFont = io.Fonts->AddFontFromFileTTF(props.fontPaths[i].c_str(), props.fontSize, NULL, ranges.Data);
			IM_ASSERT(newFont != NULL);
			fonts.insert({ props.fontNames[i], newFont});
		}

		io.Fonts->Build();

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(Engine::Instance().GetWindow()->glfwin(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	void ImguiWindow::Shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	bool ImguiWindow::WantCaptureMouse()
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool ImguiWindow::WantCaptureKeyboard()
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void ImguiWindow::BeginRender() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImguiWindow::EndRender() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			auto& window = *Engine::Instance().GetWindow();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(window.glfwin());
		}
	}
}