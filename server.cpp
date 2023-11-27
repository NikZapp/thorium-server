#include <server.hpp>

#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>

using Thorium::Server;
using nlohmann::json;

Server::Server(uint16_t port, std::string approved_file_path) {
	this->tokens = {};
	load_tokens_from_file("tokens.json");
	this->approved_file = approved_file_path;

	this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_socket == -1) {
		std::cerr << "Error creating socket" << std::endl;
		return;
	}
	// Set up the server address structure
	sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	// Bind the socket
	if (bind(this->server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
		std::cerr << "Error binding socket" << std::endl;
		return;
	}

	// Listen for incoming connections
	if (listen(this->server_socket, 10) == -1) {
		std::cerr << "Error listening for connections" << std::endl;
		return;
	}

	std::cout << "Server listening on port " << port << std::endl;
}

void generate_salt(char* buffer) {
	// Randomize the seed sometimes
	if (rand() & 0x3 == 0) srand(static_cast<unsigned>(time(nullptr)));

	// Obfuscate more
	int throwaway = 0;
	int r = rand() % 21;
	for (int i = 0; i < r; i++) {
		throwaway += rand();
	}
	std::cout << "Generating random seed. " << throwaway << std::endl;

	// Generate the buffer
	buffer[0] = 'K';
	for (int i = 0; i < 16; i++) {
		buffer[i + 1] = rand() % 256;
	}
}

void Server::handle_incoming_connections() {
	// Accept a connection
	int client_socket = accept(this->server_socket, NULL, NULL);
	if (client_socket == -1) return;

	// Get the address of the connected client
	sockaddr_in address;
	socklen_t address_size = sizeof(address);
	getpeername(client_socket, (struct sockaddr*)&address, &address_size);

	// Convert the client's IP address to a string and print it
	char client_ip[16];
	inet_ntop(AF_INET, &(address.sin_addr), client_ip, 16);
	std::cout << std::endl;
	std::cout << "Client connected from IP address: " << client_ip << std::endl;

	// Set a timeout for the socket
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	// Send salt
	char salt[17];
	generate_salt(salt);

	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (int i = 0; i < 16; i++) {
		ss << std::setw(2) << (int)(uint8_t)(salt[i + 1]);
	}

	send(client_socket, salt, 17, 0); // 17 Bytes
	std::cout << "Sent salt: " << ss.str() << std::endl;

	// Read data from the client
	char buffer[1024];
	ssize_t bytesRead = recv(client_socket, buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		// Data received, process it as needed
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		for (int i = 0; i < bytesRead; i++) {
			ss << std::setw(2) << (int)(uint8_t)(buffer[i]);
		}
		std::cout << "Received proof from client: " << ss.str() << std::endl;

		for (auto& test_token : this->tokens) {
			std::string salt_str(reinterpret_cast<char *>(salt), sizeof(salt));
			if (test_token.test_against_salt(ss.str(), salt_str.substr(1))) {
				std::cout << "Token " << test_token.name << " has matched" << std::endl;
				send(client_socket, "P", 1, 0); // Pass

				// Wait for IP (optional)
				uint8_t compact_ip[4];
				ssize_t bytesRead = recv(client_socket, compact_ip, 4, 0);

				if (bytesRead > 0) {
					// I know this is an abomination, but it
					// works and its 2 AM so it stays here.
					strcpy(client_ip, (
							std::to_string(compact_ip[0]) + "." +
							std::to_string(compact_ip[1]) + "." +
							std::to_string(compact_ip[2]) + "." +
							std::to_string(compact_ip[3])
							).c_str());
				}

				std::cout << "IP:" << client_ip << std::endl;
				allow_address(test_token, client_ip);
				close(client_socket);
				return;
			}
		}
		std::cout << "No match found." << std::endl;
		send(client_socket, "F", 1, 0); // Fail
		close(client_socket);
	} else if (bytesRead == 0) {
		std::cout << "Connection closed by client" << std::endl;
		close(client_socket);
	} else {
		// Timeout or error
		std::cerr << "Error or timeout receiving data from client" << std::endl;
		send(client_socket, "T", 1, 0); // Timeout
		close(client_socket);
	}
}

void Server::allow_address(Thorium::Token token, std::string address) {
	std::ifstream f("authorized.json");
	json authorized = json::parse(f);

	authorized[address] = {
			{"name", token.name},
			{"level", token.access_level}
	};

	std::ofstream o("authorized.json");
	o << std::setw(4) << authorized << std::endl;
}

void Server::load_tokens_from_file(std::string path) {
	std::ifstream f(path);
	json tokens = json::parse(f);
	std::cout << "Loading tokens from " << path << ":" << std::endl;
	for(auto& token : tokens.items()) {
		this->tokens.push_back(Thorium::Token(token.key(), token.value()["token"], token.value()["level"]));

		// Set color based on level
		int level_colors[4] = {31, 33, 32, 36};
		std::cout << "\033[1;" << level_colors[(int)token.value()["level"]] << "m";

		std::cout << token.key() << " " << (std::string)token.value()["token"] << std::endl;
	}
	// Reset color
	std::cout << "\033[0m";
	std::cout << "Loaded " << this->tokens.size() << " tokens" << std::endl;
}

// Token access levels:
// 0 - not allowed (red)
// 1 - untrusted (yellow)
// 2 - trusted (green)
// 3 - admin (cyan)
