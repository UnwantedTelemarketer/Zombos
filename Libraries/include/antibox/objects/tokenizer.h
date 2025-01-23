#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <filesystem>
//#include <boost/algorithm/string.hpp>

#define LAZY_LOG(thing) Console::Log(thing, text::white, -1);

using namespace antibox;

static bool fileExists(const char* filePath) {
	struct stat buffer;
	return (stat(filePath, &buffer) == 0);
}

bool to_bool(std::string str) {
	try {
		//boost::algorithm::to_lower(str);
		if (str == "true") { return true; }
		if (str == "false") { return false; }
		else { throw std::invalid_argument("Error: to_bool requires the string input to be either 'true' or 'false'."); }
	}
	catch (std::invalid_argument& e) {
#ifdef antibox_console
		//Console::Log(e.what(), ERROR, __LINE__);
#endif

#ifndef antibox_console
		std::cout << "error on line " << __LINE__ << std::endl;
#endif
	}
}

template <typename T>
struct match {
	std::vector<std::string> keys;
	std::vector<T> values;

	int size() {
		if (keys.size() == values.size()) {
			return keys.size();
		}
		else {
			return std::min(keys.size(), values.size());
		}
	}

	void append(std::string key, T value) {
		keys.push_back(key);
		values.push_back(value);
	}
};
struct InternalData {
	std::map<std::string, std::string> tokens;
	//Returns a string using the key (key : value)
	std::string getString(std::string key) {
		if (tokens.count(key) > 0)
		{
			return tokens[key];
		}
		return "";
	}
};

//OpenedData is where data is separated and saved from an EID file being read. Use the different "get" functions to pull relevant data.
struct OpenedData {
	std::string section_name;
	//'Tokens' is the file split into individual words so we can check each keyword for keys and values
	std::map<std::string, std::string> tokens;

	//Returns a string using the key (key : value)
	std::string getString(std::string key) {
		if (tokens.count(key) > 0)
		{
			return tokens[key];
		}
		return "";
	}

	//Returns an int using the key (key : value)
	int getInt(std::string key) {
		try {
			return stoi(getString(key));
		}

		catch (std::exception e) {
			Console::Log("Error: getInt requires the value to be an int.", text::red, __LINE__);
			return -1;
		}
	}

	//Returns a float using the key (key : value)
	float getFloat(std::string key) {
		try {
			return stof(getString(key));
		}

		catch (std::exception e) {
			Console::Log("Error: getFloat requires the value to be a float.", text::red, __LINE__);
			return -1;
		}
	}

	//Returns a boolean using the key (key : value)
	bool getBool(std::string key)
	{
		return to_bool(getString(key));
	}

	std::vector<std::string> getArray(std::string key) {
		std::string array = getString(key);
		std::string current_array;
		std::vector<std::string> actual_array;

		for (size_t i = 0; i < array.length(); i++)
		{
			if (array[i] == ',' && current_array != "") {
				actual_array.push_back(current_array);
				current_array = "";
				continue;
			}
			current_array += array[i];
		}

		if (current_array != "") { actual_array.push_back(current_array); }

		return actual_array;
	}
};

struct SaveSection {
	match<int> ints;
	match<float> floats;
	match<std::string> strings;
	match<int> items;
	std::map<std::string, std::vector<std::string>> lists;
	match<Vector2_I> vec2;
};
struct SaveData {
	std::string filename;
	std::map<std::string, SaveSection> sections;

	void addInt(std::string sect, std::string key, int val) {
		sections[sect].ints.append(key, val);
	}
	void addFloat(std::string sect, std::string key, float val) {
		sections[sect].floats.append(key, val);
	}
	void addString(std::string sect, std::string key, std::string val) {
		sections[sect].strings.append(key, val);
	}
	void addVec2(std::string sect, std::string key, vec2_i val) {
		sections[sect].vec2.append(key, val);
	}
};


class Tokenizer {
public:

	static std::vector<std::string> getTokens(std::string input)
	{
		std::vector<std::string> tokens;
		std::string current_token;

		bool in_quotes = false;
		bool in_brackets = false;
		std::string quote_chars = "\"";
		char close_bracket = ']';

		for (int i = 0; i < input.length(); i++) {
			if (in_quotes || in_brackets) { //if we are in quotes
				if (input[i] == quote_chars[0]) { //check for closing quotes
					in_quotes = false;
					tokens.push_back(current_token);
					current_token = "";
				}
				else if (input[i] == close_bracket) {
					in_brackets = false;
					tokens.push_back(current_token);
					current_token = "";
				}
				else {
					if (in_brackets) {
						if (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
							continue;
						}
					}
					current_token += input[i];
				}
			}
			else {
				if (input[i] == ' ' || input[i] == ':' || input[i] == ';') {
					if (current_token != "") {
						tokens.push_back(current_token);
						current_token = "";
					}
				}
				else {
					if (quote_chars.find(input[i]) != std::string::npos) {
						in_quotes = true;
					}
					else if (input[i] == '[') {
						in_brackets = true;
					}
					else {
						current_token += input[i];
					}
				}
			}
		}

		tokens.push_back(current_token);

		return tokens;
	}

	static std::string getSection(std::string file, std::string sectionName) {
		 
		std::string current_section;
		bool badsection = false;
		bool goodsection = false;

		for (int i = 0; i < file.size(); i++)
		{
			if (goodsection) {
				if (file[i] == '}') {
					return current_section;
				}
				current_section += file[i];
				continue;
			}

			if (badsection && file[i] != '}') { current_section = ""; continue; } //skip the block if we have the wrong header
			else if (badsection && file[i] == '}') { badsection = false; continue; } //if we reach the end of the section, start reading again
			else if (file[i] == ';' || file[i] == ' ') { continue; }


			if (file[i] == '{') {
				if (sectionName != current_section) { badsection = true; continue; }
				else {
					goodsection = true;
					current_section = "";
					continue;
				}
			}
			current_section += file[i];

		}
		return "err_none";
	}
};

class ItemReader{
public:

	static bool GetDataFromFile(std::string filepath, std::string section, OpenedData* data, bool defaultPath = true) {

		std::string new_filepath;
		if (defaultPath) { new_filepath = "dat/eid/" + filepath; }
		else { new_filepath = filepath; }

		if (!std::filesystem::exists(new_filepath)) {
			Console::Log("Attempting to read from file '" + new_filepath + "', but can't find file.", ERROR, __LINE__);
			return false;
		}

		std::ifstream file(new_filepath);

		std::string contents;
		std::string line; //read from file
		bool shouldWrite = false;

		//write only the section we want to a string
		while (std::getline(file, line)) {
			//when we find the section, start writing
			std::string lineNoSpace = line;
			lineNoSpace.erase(remove_if(lineNoSpace.begin(), lineNoSpace.end(), isspace), lineNoSpace.end());
			if (lineNoSpace == section + "{") { shouldWrite = true; continue; }
			else if (!shouldWrite) { continue; }

			//when we find the closing bracket, stop reading the file
			if (lineNoSpace == "}" && shouldWrite) { shouldWrite = false; break; }

			//write if we should
			if (shouldWrite) { contents += line; }
		}
		//std::string properSection = Tokenizer::getSection(contents, section);

		data->section_name = section;
		std::vector<std::string> temp_tokens = Tokenizer::getTokens(contents);//split it up / tokenize it


		for (int i = 0; i < temp_tokens.size() - 1; i++)
		{
			data->tokens[temp_tokens[i]] = temp_tokens[i + 1];
			i++;
		}
		return true;
	}

	static std::string ReturnDataToSave(SaveData data) {
		std::string file = "";
		for (auto& sect : data.sections) {
			file.append(sect.first + " {\n");
			for (int i = 0; i < sect.second.ints.size(); i++)
			{
				file.append(sect.second.ints.keys[i] + " : " + std::to_string(sect.second.ints.values[i]) + ";\n");
			}
			for (int i = 0; i < sect.second.strings.size(); i++)
			{
				file.append(sect.second.strings.keys[i] + " : \"" + sect.second.strings.values[i] + "\";\n");
			}
			for (int i = 0; i < sect.second.floats.size(); i++)
			{
				file.append(sect.second.floats.keys[i] + " : " + std::to_string(sect.second.floats.values[i]) + ";\n");
			}
			for (int i = 0; i < sect.second.vec2.size(); i++)
			{
				file.append(sect.second.vec2.keys[i] + " : [" + std::to_string(sect.second.vec2.values[i].x) + "," + std::to_string(sect.second.vec2.values[i].y) + "]; \n");
			}
			if (sect.second.lists.size() > 0) {
				//read each list
				for (auto const& list : sect.second.lists)
				{
					//write the list name
					file.append(list.first + " : [");
					//put their data into the save file
					for (size_t i = 0; i < list.second.size(); i++)
					{
						file.append(list.second[i]);
						if (i < list.second.size() - 1) {
							file.append(",");
						}
					}
					file.append("];\n");

				}
			}
			file.append("}\n");
		}
		file.append("\n");

		return file;
	}

	static void SaveDataToFile(std::string filename, SaveData data, bool logSuccess) {
		std::string dat = ReturnDataToSave(data);

		std::ofstream myfile;
		myfile.open(filename);
		myfile << dat;
		myfile.close();

		if(logSuccess) Console::Log("Wrote to file " + filename, text::green, __LINE__);
	}
};

#endif