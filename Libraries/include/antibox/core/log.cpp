#include "../core/log.h"
#include "string"
#include "mathlib.h"

void Console::Log(std::string message, textColor type = "\033[0;37m", int lineNum = -1)
{ 
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}
/*void Console::Log(unsigned char* message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}*/
void Console::Log(const char* message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}

void Console::Log(uint32_t message, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << std::to_string(message) << "\033[0m\n" << std::endl;
}
void Console::Log(int number, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << std::to_string(number) << "\033[0m\n" << std::endl;
}
void Console::Log(double number, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << std::to_string(number) << "\033[0m\n" << std::endl;
}
void Console::Log(bool tf, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string message = tf ? "True" : "False";
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}
void Console::Log(void* pointer, textColor type = "\033[0;37m", int lineNum = -1)
{
	if (pointer == nullptr) {
		std::cout << "[ Line " << lineNum << " ]: " << type << "nullptr" << "\033[0m\n" << std::endl;
	}
	else {
		std::cout << "[ Line " << lineNum << " ]: " << type << pointer << "\033[0m\n" << std::endl;
	}
}

void Console::Log(std::vector<std::string> list, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::string message = "List : {";
	for (size_t i = 0; i < list.size(); i++)
	{
		message += list[i];
		message += ", ";
	}
	message += "}";
	std::cout << "[ Line " << lineNum << " ]: " << type << message << "\033[0m\n" << std::endl;
}

void Console::Log(Vector2 vec, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << "Vector2 {" << std::to_string(vec.x) + ", " + std::to_string(vec.y) << "}" << "\033[0m\n" << std::endl;
}

void Console::Log(Vector2_I vec, textColor type = "\033[0;37m", int lineNum = -1)
{
	std::cout << "[ Line " << lineNum << " ]: " << type << "Vector2 {" << std::to_string(vec.x) + ", " + std::to_string(vec.y) << "}" << "\033[0m\n" << std::endl;
}

