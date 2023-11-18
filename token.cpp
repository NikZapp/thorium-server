#include <token.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

using Thorium::Token;

std::string decode_data(std::string hex_data) {
	std::string output = "";

	for (size_t i = 0; i < hex_data.size(); i += 2) {
		// Convert two hex characters to an integer
		int value = std::stoi(hex_data.substr(i, 2), 0, 16);
		output += static_cast<char>(static_cast<unsigned char>(value));
	}

	return output;
}

Token::Token(std::string name, std::string data, uint32_t access_level) {
	this->name = name;
	this->data = decode_data(data);
	this->access_level = access_level;
}

bool Token::test_against_salt(std::string proof, std::string salt) {
	std::string test = this->data + salt;
	std::string correct_proof = picosha2::hash256_hex_string(test);
	return (proof == correct_proof);
}
