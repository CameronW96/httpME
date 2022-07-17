#include "httpme.h"
#include <filesystem>


void HPM::HTTP::make_public(std::string directory_path)
{
	public_directory = directory_path;
	
}

void HPM::HTTP::make_route(std::string route_type, std::string request_path, std::function<void(HTTP_Connection&)> callback)
{
	if (route_type == "GET")
	{
		route.route_GET(request_path, callback);
	}
	else if (route_type == "POST")
	{
		route.route_POST(request_path, callback);
	}
	else if (route_type == "PATCH")
	{
		route.route_PATCH(request_path, callback);
	}
	else if (route_type == "DELETE")
	{
		route.route_DELETE(request_path, callback);
	}
	else
	{
		std::cerr << "Unknown Request Type Specified" << std::endl;
	}
}

void HPM::HTTP::start(short port)
{
	asio::io_context io;
	Server server(io, port, route, public_directory);
	io.run();
}

std::string HPM::HTTP::cwd()
{
	return std::filesystem::current_path().string();
}