#include "http_connection.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "globals.h"
#include "route.h"

std::string _DIRECTORY = std::filesystem::current_path().string();

// Error page locations
#define NOT_FOUND    _DIRECTORY + "\\Error Pages\\404.htm"
#define SERVER_ERROR _DIRECTORY + "\\Error Pages\\500.htm"

HTTP_Connection::HTTP_Connection(tcp::socket t_sock, Route& route, std::string& t_directory, asio::steady_timer& t_recv_timer, asio::steady_timer& t_send_timer) 
	: sock(std::move(t_sock)), route_table(route), req_buf(MAX_REQ_SIZE), public_directory(t_directory), recv_timer_ref(t_recv_timer), send_timer_ref(t_send_timer)
{
	std::cout << "New connection established" << std::endl;
}

void HTTP_Connection::start()
{
	read_request();

}

void HTTP_Connection::read_request()
{
	std::cout << "Reading request..." << std::endl;

	//Timeout timer
	recv_timer_ref.expires_from_now(std::chrono::seconds(RCV_TIMEOUT_SECONDS));
	recv_timer_ref.async_wait(
		[this](std::error_code ec)
		{
			if (ec)
			{
				return;
			}
			else if (recv_timer_ref.expires_at() <= std::chrono::steady_clock::now())
			{
				if (sock.is_open())
				{
					std::cout << "Receive timeout" << std::endl;
					sock.close();
				}
			}
		}
	);

	if (sock.available() > MAX_REQ_SIZE)
	{
		//413 Payload Too Large
	}
	else if (sock.is_open())
	{
		auto self = shared_from_this();

		sock.async_read_some(asio::buffer(req_buf), [this, self] (std::error_code ignored, std::size_t bytes_transferred)
			{
				if (bytes_transferred > 0)
				{
					recv_timer_ref.expires_at(std::chrono::steady_clock::time_point::max());
				}

				if (sock.is_open())
				{
					process_request();
				}
			});
					
	}

}

void HTTP_Connection::process_request()
{
	{ // scope limiting for memory cleanup
		//std::cout << "Processing request..." << std::endl;

		std::string req_string(req_buf.begin(), req_buf.end());
		std::string::iterator req_string_it = req_string.begin();

		std::string req_type;
		std::string req_path;
		std::string http_version;

		std::map<std::string, std::vector<std::string>> req_fields;
		std::vector<char> req_body;

		// get first line of header
		req_type = get_with_delim(req_string_it, req_string);
		req_path = get_with_delim(req_string_it, req_string);
		http_version = get_with_delim(req_string_it, req_string);

		while (req_string_it != req_string.end()) // get the rest of the fields
		{
			std::string line = get_line_with_delim(req_string_it, req_string);
			if (line.empty())
			{
				req_string_it + 2;
				req_body = parse_request_body(req_string, req_string_it);
				break;
			}
			std::pair<std::string, std::vector<std::string>> kvp = parse_header_field(line);
			req_fields.emplace(kvp.first, kvp.second);
		}

		Request new_request(http_version, req_type, req_path, req_fields, req_body);
		request = new_request;
	}

	process_response();

}

void HTTP_Connection::process_response()
{
	//std::cout << "Processing response..." << std::endl;

	// request file or directory?
	if (request.is_file())
	{

		std::string file_path = backward_to_forward_slash(public_directory);
		std::string file_name = request.request_path;

		file_path += file_name;

		if (std::filesystem::exists(file_path))
		{
			int sent_bytes = response.send_file(file_path);
			if (sent_bytes <= 0)
			{
				// 500 internal server error
				response.send_file(SERVER_ERROR, "500");
			}
		}
		else
		{
			// 404 not found
			response.send_file(NOT_FOUND, "404");
		}
	}
	else // is directory
	{
		if (request.request_type == "GET")
		{
			route_table.unroute_GET(request.request_path, *this);
		}
		else if (request.request_type == "POST")
		{
			route_table.unroute_POST(request.request_path, *this);
		}
		else if (request.request_type == "PATCH")
		{
			route_table.unroute_PATCH(request.request_path, *this);
		}
		else if (request.request_type == "DELETE")
		{
			route_table.unroute_DELETE(request.request_path, *this);
		}
		else
		{
			// 500 internal server error
			response.send_file(SERVER_ERROR, "500");
		}
	}
	send_response();
}

void HTTP_Connection::send_response()
{
	std::cout << "Sending response" << std::endl;

	//Timeout timer
	send_timer_ref.expires_from_now(std::chrono::seconds(SEND_TIMEOUT_SECONDS));
	send_timer_ref.async_wait(
		[this](std::error_code ec)
		{
			if (ec)
			{
				return;
			}
			else if (send_timer_ref.expires_at() <= std::chrono::steady_clock::now())
			{
				if (sock.is_open())
				{
					std::cout << "Send Timeout" << std::endl;
					sock.close();

				}
			}
		}
	);
	
	if (sock.is_open())
	{
		auto self = shared_from_this();
		asio::async_write(sock, asio::buffer(response.response_buf, response.response_buf.size()),
			[this, self] (std::error_code ignored, std::size_t bytes_sent)
			{
				std::cout << "Bytes sent: " << bytes_sent << std::endl;

				send_timer_ref.expires_at(std::chrono::steady_clock::time_point::max());
			}
		);		
	}
}

// ********************************************* HELPER METHODS ********************************************* \\

std::pair<std::string, std::vector<std::string>> HTTP_Connection::parse_header_field(std::string field)
{
	std::string key;
	std::string value;
	std::vector<std::string> output_vector;

	std::string::iterator it = field.begin();

	// get key
	while (it != field.end())
	{
		if (*it == ' ')
		{
			it++;
		}
		else if (*it == ':')
		{
			it++;
			break;
		}
		else
		{
			key += *it;
			it++;
		}
	}

	// get value
	while (it != field.end())
	{
		if (*it == ' ')
		{
			it++;
		}
		else if (*it == ',' || *it == ';')
		{
			it++;
			output_vector.push_back(value);
			value.clear();
		}
		else
		{
			value += *it;
			it++;
		}
	}

	output_vector.push_back(value);
	return std::make_pair(key, output_vector);
}

std::vector<char> HTTP_Connection::parse_request_body(const std::string& body, std::string::iterator& it)
{
	std::vector<char> output;

	while (it != body.end())
	{
		output.push_back(*it);
		it++;
	}

	return output;
}

// Iterator must be derived from the string argument
std::string HTTP_Connection::get_with_delim(std::string::iterator& it, const std::string& str)
{
	std::string output;

	while (it != str.end() && *it != ' ' && *it != '\r' && *it != '\n')
	{
		output += *it;
		it++;
	}

	if (it != str.end() && *it == '\r')
	{
		it++;
		it++;
		return output;
	}
	else if (it != str.end())
	{
		it++;
		return output;
	}
	else
	{
		return output;
	}
}

std::string HTTP_Connection::get_line_with_delim(std::string::iterator& it, const std::string& str)
{
	std::string output;

	while (it != str.end() && *it != '\r' && *it != '\n')
	{
		output += *it;
		it++;
	}

	if (it != str.end() && *it == '\r')
	{
		it++;
		it++;
		return output;
	}
	else if (it != str.end())
	{
		it++;
		return output;
	}
	else
	{
		return output;
	}
}

std::string HTTP_Connection::get_directory(std::string path)
{
	std::string directory;
	std::string::iterator it = path.end();
	it--;

	if (path.size() > 1) {
		while (*it == ' ' || *it == '/' || *it == '\\') // skip whitespace or trailing slashes
		{
			it--;
		}
	}

	while (*it != '/' && *it != '\\')
	{
		directory += *it;
		it--;
	}

	std::reverse(directory.begin(), directory.end());
	return directory;
}


void HTTP_Connection::set_directory(const std::string& dirname)
{
	base_directory = dirname;
}

std::string HTTP_Connection::forward_to_backward_slash(std::string t_string_input)
{
	std::string output;
	std::string::iterator it = t_string_input.begin();

	while (it != t_string_input.end())
	{
		if (*it == '/')
		{
			output += '\\';
		}
		else
		{
			output += *it;
		}
		it++;
	}
	return output;
}

std::string HTTP_Connection::backward_to_forward_slash(std::string t_string_input)
{
	std::string output;
	std::string::iterator it = t_string_input.begin();

	while (it != t_string_input.end())
	{
		if (*it == '\\')
		{
			output += '/';
		}
		else
		{
			output += *it;
		}
		it++;
	}
	return output;
}


// Route
void Route::route_GET(std::string path, std::function<void(HTTP_Connection&)> functor)
{
	GET_list.push_back(std::make_pair(path, functor));
}

void Route::route_POST(std::string path, std::function<void(HTTP_Connection&)> functor)
{
	POST_list.push_back(std::make_pair(path, functor));
}

void Route::route_PATCH(std::string path, std::function<void(HTTP_Connection&)> functor)
{
	PATCH_list.push_back(std::make_pair(path, functor));
}

void Route::route_DELETE(std::string path, std::function<void(HTTP_Connection&)> functor)
{
	DELETE_list.push_back(std::make_pair(path, functor));
}

void Route::unroute_GET(std::string path, HTTP_Connection& http_instance)
{
	route_vec::iterator it = GET_list.begin();
	while (it != GET_list.end())
	{
		if (it->first == path)
		{
			it->second(http_instance);
			break;
		}
		it++;
	}
}

void Route::unroute_POST(std::string path, HTTP_Connection& http_instance)
{
	route_vec::iterator it = POST_list.begin();
	while (it != POST_list.end())
	{
		if (it->first == path)
		{
			it->second(http_instance);
			break;
		}
		it++;
	}
}

void Route::unroute_PATCH(std::string path, HTTP_Connection& http_instance)
{
	route_vec::iterator it = PATCH_list.begin();
	while (it != PATCH_list.end())
	{
		if (it->first == path)
		{
			it->second(http_instance);
			break;
		}
		it++;
	}
}

void Route::unroute_DELETE(std::string path, HTTP_Connection& http_instance)
{
	route_vec::iterator it = DELETE_list.begin();
	while (it != DELETE_list.end())
	{
		if (it->first == path)
		{
			it->second(http_instance);
			break;
		}
		it++;
	}
}