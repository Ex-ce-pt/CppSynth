#ifndef CPPSYNTH_DEBUG_LOG_HPP
#define CPPSYNTH_DEBUG_LOG_HPP

#include <iostream>
#include <string_view>

namespace CppSynth::Debug {

	// TODO: add a proper logging system later, this will do for now
	inline void log(std::string_view s) {
		std::cout << s;
	}

	inline void log(int64_t n) {
		std::cout << n;
	}

	inline void log(double d) {
		std::cout << d;
	}

}

#endif /* CPPSYNTH_DEBUG_LOG_HPP */
