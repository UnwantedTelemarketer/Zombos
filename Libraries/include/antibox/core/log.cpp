#include "../core/log.h"
#include "string"
#include "mathlib.h"

using namespace antibox;

std::vector<std::string> Console::allLogs;

void Console::Log(std::string message, textColor type, int lineNum)
{ 
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + message;
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);

}

void Console::Log(
	std::string message,
	textColor type,
	const char* file,
	int lineNum
)
{
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + message;

	allLogs.push_back(msg);
}

/*void Console::Log(unsigned char* message, textColor type, int lineNum)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}*/
void Console::Log(const char* message, textColor type, const char* file, int lineNum)
{
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + message;
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(uint32_t message, textColor type, const char* file, int lineNum)
{
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + std::to_string(message);
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);
}
void Console::Log(int number, textColor type, const char* file, int lineNum)
{
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + std::to_string(number);
	allLogs.push_back(msg);
}
void Console::Log(double number, textColor type, const char* file, int lineNum)
{
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + std::to_string(number);
	allLogs.push_back(msg);
}
void Console::Log(bool tf, textColor type, const char* file, int lineNum)
{
	std::string message = tf ? "True" : "False";
	std::string msg =
		"[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + message;
	allLogs.push_back(msg);
}
void Console::Log(void* pointer, textColor type, const char* file, int lineNum)
{
	std::string msg;
	if (pointer == nullptr) {
		msg = "[ " + std::string(file) +
			":" + std::to_string(lineNum) +
			" ] " + "nullptr";
	}
	else {
		msg = "[ " + std::string(file) +
			":" + std::to_string(lineNum) +
			" ] " + "pointer";
	}
	allLogs.push_back(msg);
}

void Console::Log(std::vector<std::string> list, textColor type, const char* file, int lineNum)
{
	std::string msg;
	std::string message = "List : {";
	for (size_t i = 0; i < list.size(); i++)
	{
		message += list[i];
		message += ", ";
	}
	message += "}";

	msg = "[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + message;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector2 vec, textColor type, const char* file, int lineNum)
{
	std::string msg = "[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + "Vector2 {" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "}";
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector3 vec3, textColor type, const char* file, int lineNum)
{
	std::string msg = "[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + "Vector3 {" + std::to_string(vec3.x) + ", " + std::to_string(vec3.y) + ", " + std::to_string(vec3.z) + "}";
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector2_I vec, textColor type, const char* file, int lineNum)
{
	std::string msg = "[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + "Vector2_I {" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "}";
	//std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

//template <typename T, typename T2>
//void Console::Log(std::pair<T, T2>, textColor type, int lineNum)
//{
//	std::cout << "[ Line " << lineNum << " ]: " << type << "Key : "<< std::to_string(vec.x) + ", " + std::to_string(vec.y) << "}" << "\033[0m\n" << std::endl;
//}

void Console::Log(std::pair<std::string, std::string> pair, textColor type, const char* file, int lineNum)
{
	std::string msg = "[ " + std::string(file) +
		":" + std::to_string(lineNum) +
		" ] " + "\nKey : " + pair.first + "\nValue : " + pair.second;
	allLogs.push_back(msg);
}