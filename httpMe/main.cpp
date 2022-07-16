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
	http.make_public("C:\\Users\\mynam\\OneDrive\\Documents\\Projects\\FS Tutorials\\bootstrap-tutorial\\tindog\\public");

	auto lambda = [](HTTP_Connection& con)
	{
		con.response.send_file("C:\\Users\\mynam\\OneDrive\\Documents\\Projects\\FS Tutorials\\bootstrap-tutorial\\tindog\\index.html");
	};

	http.make_route("GET", "/", lambda);

	http.start(PORT);
}