#include "cosmetic.h"
#include "imgui/imgui.h"
#include "antibox/core/mathlib.h"

using namespace antibox;

ImVec4 Cosmetic::FireColor() {
	switch (Math::RandInt(1, 5)) {
	case 1:
		return { 1, 0.65f, 0, 1 };
		break;
	case 2:
		return { 1, 0.15f, 0, 1 };
		break;
	case 3:
		return { 1, 1, 0, 1 };
		break;
	case 4:
	default:
		return { 1, 0.35f, 0, 1 };
		break;
	}
}

ImVec4 Cosmetic::RandomColor() {
	switch (Math::RandInt(1, 5)) {
	case 1:
		return { 1, 0, 0, 1 };
		break;
	case 2:
		return { 0, 1, 0, 1 };
		break;
	case 3:
		return { 0, 0, 1, 1 };
		break;
	case 4:
	default:
		return { 1, 1, 0, 1 };
		break;
	}
}

ImVec4 Cosmetic::SmokeColor() {
	switch (Math::RandInt(1, 4)) {
	case 1:
		return { 0.5f, 0.5f, 0.5f, 1 };
		break;
	case 2:
		return { 0.55f, 0.55f, 0.55f, 1 };
		break;
	case 3:
		return { 0.6f, 0.6f, 0.6f, 1 };
		break;
	case 4:
	default:
		return { 0.35f, 0.35f, 0.35f, 1 };
		break;
	}
}

char Cosmetic::FireChar(std::string chars) {
	return chars[Math::RandInt(1, chars.length())];
}

// Liquid IDs
// nothing = 0, water = 1, blood = 2, fire = 3, guts = 4, mud = 5, snow = 6
const char* Cosmetic::CoveredName(int liquid)
{
	switch (liquid) {
	case -1:
		return "Strange Liquid";
		break;
	case 1:
		return "Water";
		break;
	case 2:
		return "Blood";
		break;
	case 3:
		return "Fire";
		break;
	case 4:
		return "Guts";
		break;
	case 5:
		return "Mud";
		break;
	case 6:
		return "Snow";
		break;
	default:
		return "";
		break;
	}
}

const char* Cosmetic::CoveredAdjName(int liquid)
{
	switch (liquid) {
	case -1:
		return "Strange";
		break;
	case 1:
		return "Wet";
		break;
	case 2:
		return "Bloody";
		break;
	case 3:
		return "Burning";
		break;
	case 4:
		return "Bloody";
		break;
	case 5:
		return "Muddy";
		break;
	case 6:
		return "Snow-Covered";
		break;
	default:
		return "";
		break;
	}
}


ImVec4 Cosmetic::CoveredColor(int liquid)
{
	switch (liquid) {
	case -1:
		return RandomColor(); //rgby
		break;
	case 1:
		return { 0, 0.6f, 1, 1 }; //blue
		break;
	case 2:
		return { 1, 0.15f, 0, 1 }; // red
		break;
	case 3:
		return FireColor();
		break;
	case 4:
		return { 0.65f, 0.1f, 0, 1 }; // dark red (steve lacy reference??)
		break;
	case 5:
		return { 0.6f, 0.4f, 0.3f, 1 };
		break;
	case 6:
		return { 0.85f, 0.85f, 0.85f, 1 };
		break;
	default:
		return { 1, 1, 1, 1 }; //white
		break;
	}
}

const char* Cosmetic::WeatherName(int weather) {
	if (weather == 0) {
		return "Clear";
	}
	else if (weather == 1) {
		return "Rainy";
	}
	else if (weather == 2) {
		return "Thunder";
	}
	return "NULL";
}
const char* Cosmetic::EquipTypeName(int type) {
	switch (type)
	{
	case 0:
		return "None";
	case 1:
		return "Weapon";
	case 2:
		return "Hat";
	case 3:
		return "Shirt";
	case 4:
		return "Pants";
	case 5:
		return "Boots";
	case 6:
		return "Gloves";
	case 7:
		return "Neck";
	case 8:
		return "Back";
	default:
		return "None";
	}
}