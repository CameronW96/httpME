#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include "httpme.h"

constexpr int PORT = 4000;

using namespace HPM;

int main(int argc, char* argv[])
{
	HTTP http;
	std::string D = http.cwd();
	http.make_public(D + "\\tindog\\public");

	auto lambda = [&](HTTP_Connection& con)
	{
		con.response.send_file(D + "\\tindog\\index.html");
	};
	
	std::cout << http.cwd() << std::endl;
	std::cout << D + "\\tindog\\public" << std::endl;

	http.make_route("GET", "/", lambda);

	http.start(PORT);
}