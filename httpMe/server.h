#pragma once

#include <iostream>

#include "asio.hpp"
#include "httpme.h"
#include "http_connection.h"
//#include "route.h"

using asio::ip::tcp;

class Server
{
public:
	Server(asio::io_context& t_io_context, int t_port, Route &route, std::string& t_directory);
private:
	void listen();

private:
	tcp::acceptor acceptor;	
	Route& route_ref;
	std::string& public_directory;
	asio::io_context& io;
	asio::steady_timer recv_timer;
	asio::steady_timer send_timer;

};

