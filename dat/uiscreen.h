#pragma once
struct UIPopup {
	std::string title;
	std::string message;
};
struct GameUI {
	bool statsOpen = false;
	bool debugOpen = false;
	bool itemMenu = false;
	bool navInv = false;
	bool useBool = false;
	bool craftingMenu = false;
	bool showDialogue = false;
	bool createChar = false;
	bool fancyGraphics = false;
	bool containerOpen = false;
	bool helpMenu = false;
	bool console_showing = false;
	bool equipmentScreenOpen = true;
	bool settingsOpen = false;
	bool tradeDialogue = false;
	bool minimapOpen = false;
	bool largeMapOpen = false;
	bool glyphViewerOpen = false;
	bool itemViewerOpen = false;
	std::map<std::string, UIPopup> popups;

	void FlipScreens() {
		if (Input::KeyDown(KEY_GRAVE_ACCENT)) {
			console_showing = !console_showing;
		}
		if (console_showing || craftingMenu) { return; }
		if (Input::KeyDown(KEY_P))
		{
			debugOpen = !debugOpen;
		}
		if (Input::KeyDown(KEY_C))
		{
			craftingMenu = !craftingMenu;
		}
		if (Input::KeyDown(KEY_H))
		{
			helpMenu = !helpMenu;
		}
		if (Input::KeyDown(KEY_I))
		{
			equipmentScreenOpen = !equipmentScreenOpen;
		}
		if (Input::KeyDown(KEY_ESCAPE))
		{
			settingsOpen = !settingsOpen;
		}
		if (Input::KeyDown(KEY_M)) {
			largeMapOpen = !largeMapOpen;
		}
	}

	void ShowPopups() {
		for (const auto& element : popups) {
			bool deleteme = false;
			ImGui::Begin(element.second.title.c_str());
			ImGui::Text(element.second.message.c_str());
			if (ImGui::Button("Close")) {
				deleteme = true;
			}
			ImGui::End();
			if(deleteme) DeletePopup(element.second.title);
		}
	}

	void MainMenu() {
		settingsOpen = false;
		console_showing = false;
	}

	void CreatePopup(std::string title, std::string message) {
		popups.insert({ title, {title, message} });
	}

	void DeletePopup(std::string title) {
		popups.erase(title);
	}
};