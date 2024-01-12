#include "common_utils.h"

#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Utils {
	std::filesystem::path getCurrentProcessDirectory() {
#ifdef _WIN32
		wchar_t buffer[1024];
		if (GetModuleFileNameW(NULL, buffer, sizeof(buffer))) {
			return std::filesystem::path(buffer).parent_path();
		}
		else {
			assert(0);
			return std::filesystem::path();
		}
#else
		assert(0);
		return std::filesystem::path();
#endif
	}
}