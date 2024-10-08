#include <iostream>
#include <vector>
#define antibox_console


typedef const char textColor[8];


namespace Console {

	//class Vector2;
	//class Vector2_I;


	//Logs a message to the command prompt. ERROR is red, WARNING is yellow, SUCCESS is green, LOG is white.
	void Log(std::string message, textColor type, int lineNum);
	//void Log(unsigned char* message, textColor type, int lineNum);

	void Log(const char* message, textColor type, int lineNum);

	void Log(uint32_t message, textColor type, int lineNum);
	void Log(int number, textColor type, int lineNum);
	void Log(double number, textColor type, int lineNum);
	void Log(bool tf, textColor type, int lineNum);

	void Log(void* pointer, textColor type, int lineNum);

	void Log(std::vector<std::string> list, textColor type, int lineNum);
	//void Log(Vector2 vec, textColor type, int lineNum);
	//void Log(Vector2_I vec, textColor type, int lineNum);

	//template <typename T, typename T2>
	//void Log(std::pair<T, T2> pair, textColor type, int lineNum);

	void Log(std::pair<std::string, std::string> pair, textColor type, int lineNum);
}

#ifndef GEN_CONS_OUT
#define GEN_CONS_OUT

template <typename T>
void GenericConsoleOut(T obj, textColor col)
{
	std::cout << "[ Line " << __LINE__ << " ]: " << col << obj << "\033[0m\n" << std::endl;
}

#endif

#define ANTIBOX_QLOG(string) std::cout << "Error at line " << __LINE__ << " in file " << __FILE__ << "> \n"; std::cout << "\033[1;31m" << string << "\033[0m\n" << std::endl;
#define ANTIBOX_ERRLOG(string, log) std::cout << "Error at line " << __LINE__ << " in file " << __FILE__ << "> \n"; std::cout << "\033[1;31m" << string << "\033[0m\n" << std::endl;std::cout << "\033[1;31m" << log << "\033[0m\n" << std::endl;