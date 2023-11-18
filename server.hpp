#pragma once

#include <token.hpp>

#include <picosha2.h>
#include <json.hpp>

#include <string>
#include <iostream>
#include <vector>

namespace Thorium {
    class Server {
    public:
    	std::vector<Thorium::Token> tokens;
    	std::string approved_file;
    	int server_socket;

    	void load_tokens_from_file(std::string path);
    	void allow_address(Thorium::Token token, std::string address);
    	void remove_address(std::string address);
    	void handle_incoming_connections();

    	Server(uint16_t port, std::string approved_file_path);
    	~Server();
    };
}
