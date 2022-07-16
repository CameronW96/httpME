#pragma once

#include <memory>
#include <iostream>
#include <sstream>

#include "asio.hpp"
#include "http_connection.h"
#include "server.h"
//#include "route.h"

using asio::ip::tcp;

//constexpr std::string VERSION = const std::string("0.0.1");

namespace HPM
{
	class HTTP
	{
		friend class Server;
	public:
		void make_public(std::string directory_path);
		void make_route(std::string route_type, std::string request_path, std::function<void(HTTP_Connection&)> callback);
		void start(short port = 80);

	private:
		Route route;
		std::string public_directory;

	};
}
