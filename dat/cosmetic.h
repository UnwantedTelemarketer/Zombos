#pragma once
#include <string>

struct ImVec4;

class Cosmetic {
public:
	static ImVec4 CoveredColor(int liquid);
	static ImVec4 FireColor();
	static ImVec4 RandomColor();
	static ImVec4 SmokeColor();
	static char FireChar(std::string chars);
	static const char* CoveredName(int liquid);
	static const char* CoveredAdjName(int liquid);
	static const char* WeatherName(int weather);
	static const char* EquipTypeName(int type);
};

