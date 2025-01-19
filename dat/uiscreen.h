#pragma once

struct GameUI {
	bool statsOpen,
		debugOpen,
		itemMenu,
		navInv,
		useBool,
		craftingMenu,
		showDialogue,
		createChar,
		fancyGraphics,
		containerOpen,
		helpMenu,
		console_showing,
		equipmentScreenOpen,
		settingsOpen;

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