#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <map>
#define LAZY_LOG(thing) Console::Log(thing, text::white, -1);

bool to_bool(std::string str) {
	try {
		if (str == "true") { return true; }
		if (str == "false") { return false; }
		else { throw std::invalid_argument("Error: to_bool requires the string input to be either 'true' or 'false'."); }
	}
	catch (std::invalid_argument& e) {
#ifdef antibox_console
		Console::Log(e.what(), ERROR, __LINE__);
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
struct SaveData {
	match<int> ints;
	match<float> floats;
	match<std::string> strings;
	match<int> items;
};


static class Tokenizer {
public:
	static std::vector<std::string> getTokens(std::string input)
	{
		std::vector<std::string> tokens;
		std::string current_token;

		bool in_quotes = false;
		std::string quote_chars = "\"";
		char close_bracket = ']';

		for (int i = 0; i < input.length(); i++) {
			if (in_quotes) { //if we are in quotes
				if (input[i] == quote_chars[0] || input[i] == close_bracket) { //check for closing quotes
					in_quotes = false;
					tokens.push_back(current_token);
					current_token = "";
				}
				else {
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
					if (quote_chars.find(input[i]) != std::string::npos || input[i] == '[') {
						in_quotes = true;
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
		return NULL;
	}
};

static class ItemReader{
public:

	static void GetDataFromFile(std::string filepath, std::string section, OpenedData* data) {
		
		std::ifstream file("dat/eid/" + filepath);
		
		std::string contents;
		std::string line; //read from file

		while (std::getline(file, line)) {
			contents += line;
		} //put file data into std::string

		std::string properSection = Tokenizer::getSection(contents, section);

		data->section_name = section;
		std::vector<std::string> temp_tokens = Tokenizer::getTokens(properSection);//split it up / tokenize it
		for (int i = 0; i < temp_tokens.size() - 1; i++)
		{
			data->tokens[temp_tokens[i]] = temp_tokens[i + 1];
			i++;
		}
		 
	}
	static void SaveDataToFile(std::string filename, std::string sectionName, SaveData data) {
		std::string file = sectionName + " {\n";
		for (int i = 0; i < data.ints.size(); i++)
		{
			file.append(data.ints.keys[i] + " : " + std::to_string(data.ints.values[i]) + ";\n");
		}
		for (int i = 0; i < data.strings.size(); i++)
		{
			file.append(data.strings.keys[i] + " : \"" + data.strings.values[i] + "\";\n");
		}
		for (int i = 0; i < data.floats.size(); i++)
		{
			file.append(data.floats.keys[i] + " : " + std::to_string(data.floats.values[i]) + ";\n");
		}
		if (data.items.size() > 0) {
			file.append("items : [");
			for (int i = 0; i < data.items.size(); i++)
			{
				file.append(data.items.keys[i] + "," + std::to_string(data.items.values[i]) + ",");
			}
			file.append("];\n");
		}
		file.append("}");


		std::ofstream myfile;
		myfile.open(filename);
		myfile << file;
		myfile.close();

		Console::Log("Wrote to file " + filename, text::green, __LINE__);
	}
};
