#include "cosmetic.h"
#include "imgui/imgui.h"
#include "antibox/core/mathlib.h"

using namespace antibox;

ImVec4 Cosmetic::FireColor() {
	switch (Math::RandInt(1, 5)) {
	case 1:
		return { 1, 0.65, 0, 1 };
		break;
	case 2:
		return { 1, 0.15, 0, 1 };
		break;
	case 3:
		return { 1, 1, 0, 1 };
		break;
	case 4:
	default:
		return { 1, 0.35, 0, 1 };
		break;
	}
}
ImVec4 Cosmetic::SmokeColor() {
	switch (Math::RandInt(1, 4)) {
	case 1:
		return { 0.5, 0.5, 0.5, 1 };
		break;
	case 2:
		return { 0.55, 0.55, 0.55, 1 };
		break;
	case 3:
		return { 0.6, 0.6, 0.6, 1 };
		break;
	case 4:
	default:
		return { 0.35, 0.35, 0.35, 1 };
		break;
	}
}

char Cosmetic::FireChar(std::string chars) {
	return chars[Math::RandInt(1, chars.length())];
}

//{ nothing, water, blood, fire };
const char* Cosmetic::CoveredName(int liquid)
{
	switch (liquid) {
	case 1:
		return "Wet";
		break;
	case 2:
		return "Bloody";
		break;
	case 3:
		return "Burning";
		break;
	default:
		return "";
		break;
	}
}


ImVec4 Cosmetic::CoveredColor(int liquid)
{
	switch (liquid) {
	case 1:
		return { 0, 0.6, 1, 1 }; //blue
		break;
	case 2:
		return { 1, 0.15, 0, 1 }; // red
		break;
	case 3:
		return FireColor();
		break;
	default:
		return { 1, 1, 1, 1 }; //white
		break;
	}
}
