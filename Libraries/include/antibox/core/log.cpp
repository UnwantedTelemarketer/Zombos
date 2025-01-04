#include "../core/log.h"
#include "string"
#include "mathlib.h"

using namespace antibox;


	std::vector<std::string> Console::allLogs;


void Console::Log(std::string message, textColor type = "\033[0;37m", int lineNum = -1)
{ 
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + message + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);

}
/*void Console::Log(unsigned char* message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}*/
void Console::Log(const char* message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + message + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(uint32_t message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + std::to_string(message) + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}
void Console::Log(int number, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + std::to_string(number) + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}
void Console::Log(double number, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + std::to_string(number) + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}
void Console::Log(bool tf, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string message = tf ? "True" : "False";
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + message + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}
void Console::Log(void* pointer, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg;
	if (pointer == nullptr) {
		msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + "nullptr" + "\033[0m\n";
	}
	else {
		msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + "pointer" + "\033[0m\n";
	}
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(std::vector<std::string> list, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg;
	std::string message = "List : {";
	for (size_t i = 0; i < list.size(); i++)
	{
		message += list[i];
		message += ", ";
	}
	message += "}";
	msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + message + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector2 vec, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + "Vector2 {" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "}" + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector3 vec3, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + "Vector3 {" + std::to_string(vec3.x) + ", " + std::to_string(vec3.y) + ", " + std::to_string(vec3.z) + "}" + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

void Console::Log(antibox::Vector2_I vec, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string msg = "[ Line " + std::to_string(lineNum) + " ]: " + type + "Vector2 {" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "}" + "\033[0m\n";
	std::cout << msg << std::endl;
	allLogs.push_back(msg);
}

//template <typename T, typename T2>
//void Console::Log(std::pair<T, T2>, textColor type, int lineNum)
//{
//	std::cout << "[ Line " << lineNum << " ]: " << type << "Key : "<< std::to_string(vec.x) + ", " + std::to_string(vec.y) << "}" << "\033[0m\n" << std::endl;
//}

void Console::Log(std::pair<std::string, std::string> pair, textColor type, int lineNum)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << "\nKey : " << pair.first + "\nValue : " + pair.second << "" << "\033[0m\n" << std::endl;
}