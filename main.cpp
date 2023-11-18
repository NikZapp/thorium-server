#include <token.hpp>
#include <server.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
	Thorium::Server *server = new Thorium::Server(22542, "allowed_ips.txt");
	while (true) {
		server->handle_incoming_connections();
	}
	return 0;
}
