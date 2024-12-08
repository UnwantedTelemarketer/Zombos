#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;


namespace antibox {
	std::filesystem::path getCurrentWorkingDirectory() {
		fs::path cwd = fs::current_path();  // Get current working directory
		return cwd;
	}

	void deleteAllFilesInDirectory() {
		try {
			fs::path curPath = getCurrentWorkingDirectory();
			curPath /= "dat";
			curPath /= "map";
			for (const auto& entry : fs::directory_iterator(curPath)) {
				// Check if it's a regular file before deleting
				if (fs::is_regular_file(entry.status())) {
					fs::remove(entry.path());
					std::cout << "Deleted: " << entry.path() << std::endl;
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Error deleting files: " << e.what() << std::endl;
		}
	}
}