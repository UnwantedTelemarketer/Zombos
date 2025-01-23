#pragma once

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
	bool equipmentScreenOpen = false;
	bool settingsOpen = false;

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
	}
};