#pragma once

#include <picosha2.h>

#include <string>
#include <iostream>

namespace Thorium {
    class Token {
    public:
    	std::string name;
    	std::string data;
    	uint32_t access_level;

    	bool test_against_salt(std::string proof, std::string salt);

        Token(std::string name, std::string data, uint32_t access_level);
        ~Token() {};
    };
}
