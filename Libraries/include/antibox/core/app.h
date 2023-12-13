#pragma once

namespace antibox 
{
	class App 
	{
	public:

		App() {}
		~App() {}
		
		//Default non overwritten GetWindowProperties should just return the default window settings.
		virtual WindowProperties GetWindowProperties() { return WindowProperties(); } 

		virtual void Init() {} //This will be called before the first frame.
		virtual void Shutdown() {} //This is called when the user closes the window.

		virtual void Update() {} //This is called before each frame is rendered.
		virtual void Render() {} //This is the calls to render each frame.
		virtual void ImguiRender() {} //This is called to render ImGui.

	private:
	};
}