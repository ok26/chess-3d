#ifndef NAME_STORAGE_H
#define NAME_STORAGE_H

#include <array>
#include <string>
#include <iostream>

class ChessNameStore {

public:
	std::array<std::string, 64> squareNames;

	ChessNameStore() {

		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				squareNames[i * 8 + j] = { (char)('a' + j), (char)('1' + 7 - i) };
	}
};



#endif 
