#pragma once

#include <iostream>
#include <vector>

// print std container

template <class T>
inline std::ostream& operator<<(std::ostream& str, const std::vector<T>& vec) {
	str << "std::vector(";
	bool first = true;
	for (const auto& elem : vec) {
		if (first) {
			first = false;
		} else {
			str << ", ";
		}
		str << elem;
	}
	str << ")";
	return str;
}
