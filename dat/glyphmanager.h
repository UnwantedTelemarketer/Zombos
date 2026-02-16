#include <antibox/graphics/helpers.h>
#include <antibox/core/log.h>
#include "antibox/objects/tokenizer.h"
#include <iomanip>

class GlyphManager {
private:
	std::map<std::string, std::string> glyphs;
	const int WIDTH = 44;
	const int HEIGHT = 12;
	std::unordered_set<std::string> invalid_glyphs;

public:
	void LoadGlyphs();
	const std::string& getGlyph(const std::string& id);
	std::string convert(char32_t codepoint);
	const std::map<std::string, std::string>& getAllGlyphs();
};

void GlyphManager::LoadGlyphs() {


	//reading glyph data file (for custom glyphs)
	OpenedData defaultFontName;
	ItemReader::GetDataFromFile("game_settings.eid", "FONTS", &defaultFontName);

	std::string dataFile = defaultFontName.getString("glyph_data_file");

	//using custom glyph data file if it exists, otherwise using the default one
	OpenedData glyphSectReader;
	ItemReader::GetDataFromFile(dataFile, "SECTIONS", &glyphSectReader);


	std::vector<std::string> glyphSections;

	//read in the section names
	for (auto const& x : glyphSectReader.getArray("sections")) {
		glyphSections.push_back(x);
	}

	//read each section
	for (auto const& sect : glyphSections)
	{
		ConsoleLog(sect, LOG);
		OpenedData spriteData;
		ItemReader::GetDataFromFile("glyphs.eid", sect, &spriteData);

		for (auto const& arrPair : spriteData.tokens) {

			//we are basically reading in paired coordinates, converting those to
			//the proper unicode private use area codepoint, then converting that to the right stuff
			//to be converted to a utf-8 string so we can render it :/

			std::stringstream sstream;
			uint32_t coord = 0xE000;
			int x = stoi(spriteData.getArray(arrPair.first)[0]); //grab the column
			int y = stoi(spriteData.getArray(arrPair.first)[1]); //grab the row
			coord += x + (WIDTH * (y - 1));						 //convert it to 1d

			// Convert integer -> char32_t
			char32_t ch = static_cast<char32_t>(coord);

			glyphs.insert({ arrPair.first , convert(ch) });
		}
	}

	ConsoleLog("Finished loading glyphs!", SUCCESS);
}

const std::string& GlyphManager::getGlyph(const std::string& id) {
	auto it = glyphs.find(id);
	if (it != glyphs.end()) {
		return it->second;   // no copy
	}

	//only spit out the error once
	if (!invalid_glyphs.contains(id)) {
		Console::Log("Glyph ID '" + id + "' not found!", text::red, __LINE__);
		invalid_glyphs.emplace(id);
	}

	return glyphs.at("error");  // also no copy
}

std::string GlyphManager::convert(char32_t codepoint) {
	std::string out;

	if (codepoint <= 0x7F) {
		// 1-byte
		out.push_back(static_cast<char>(codepoint));
	}
	else if (codepoint <= 0x7FF) {
		// 2-byte
		out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
		out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
	}
	else {
		// 3-byte (correct for all PUA BMP characters)
		out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
		out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
	}

	return out;
}

const std::map<std::string, std::string>& GlyphManager::getAllGlyphs() {
	return glyphs;
}