#pragma once

#include <algorithm>
#include <filesystem>
#include <functional>

#include "asio.hpp"
#include "request.h"
#include "response.h"

using asio::ip::tcp;

class Route;

class HTTP_Connection : public std::enable_shared_from_this<HTTP_Connection>
{
	friend class Server;
public:
	HTTP_Connection(tcp::socket t_sock, Route& route, std::string& t_directory, asio::steady_timer& t_recv_timer, asio::steady_timer& t_send_timer);
	void start();
	void set_directory(const std::string& dirname);
	   
private:
	void run();
	void read_request();
	void process_request();
	void process_response();
	void send_response();
	void parse_request();
	std::pair<std::string, std::vector<std::string>> parse_header_field(std::string field);
	std::vector<char> parse_request_body(const std::string& body, std::string::iterator& it);
	std::string get_with_delim(std::string::iterator& it, const std::string& str);
	std::string get_line_with_delim(std::string::iterator& it, const std::string& str);
	std::string get_directory(std::string path);
	std::string forward_to_backward_slash(std::string t_string_input);
	std::string backward_to_forward_slash(std::string t_string_input);

public:
	Request request;
	Response response;

private:
	tcp::socket sock;
	std::vector<char> req_buf;
	std::vector<char> res_buf;
	std::string fpath;
	std::filesystem::path base_directory;
	std::string& public_directory;
	Route& route_table;
	asio::steady_timer& recv_timer_ref;
	asio::steady_timer& send_timer_ref;

};

class Route
{
	using route_vec = std::vector<std::pair<std::string, std::function<void(HTTP_Connection&)>>>;

public:
	void route_GET(std::string path, std::function<void(HTTP_Connection&)> functor);
	void route_POST(std::string path, std::function<void(HTTP_Connection&)> functor);
	void route_PATCH(std::string path, std::function<void(HTTP_Connection&)> functor);
	void route_DELETE(std::string path, std::function<void(HTTP_Connection&)> functor);
	void unroute_GET(std::string path, HTTP_Connection& http_instance);
	void unroute_POST(std::string path, HTTP_Connection& http_instance);
	void unroute_PATCH(std::string path, HTTP_Connection& http_instance);
	void unroute_DELETE(std::string path, HTTP_Connection& http_instance);
private:
	route_vec GET_list;
	route_vec POST_list;
	route_vec PATCH_list;
	route_vec DELETE_list;
};