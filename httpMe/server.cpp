#include "globals.h"
#include "server.h"

Server::Server(asio::io_context& t_io_context, int t_port, Route& route, std::string& t_directory) 
	: acceptor(t_io_context, { tcp::v4(), static_cast<asio::ip::port_type>(t_port)}), route_ref(route), public_directory(t_directory), io(t_io_context), recv_timer(t_io_context), send_timer(t_io_context)
{
	std::cout << "Server created..." << std::endl;
	
	listen();
}

void Server::listen()
{
	acceptor.async_accept(
		[this](std::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				std::cout << "Creating new connection..." << std::endl;

				try
				{
					std::make_shared<HTTP_Connection>(std::move(socket), route_ref, public_directory, recv_timer, send_timer)->start();
				}
				catch (asio::error_code e)
				{
					std::cout << e.message() << std::endl;
				}
				std::cout << "Closing connection..." << std::endl;
			}	

			listen();
		});

	

	

}