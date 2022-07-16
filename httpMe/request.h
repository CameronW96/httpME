#pragma once

#include <map>
#include <string>
#include <vector>

class Request
{
	friend class HTTP_Connection;
public:
	Request();
	Request(std::string t_http_version, std::string t_request_type, std::string t_request_path, std::map<std::string, std::vector<std::string>> t_request_header_fields, std::vector<char> t_request_body);

private:
	bool is_file();

private:
	std::string http_version;
	std::string request_type;
	std::string request_path;
	std::map<std::string, std::vector<std::string>> request_header_fields;
	std::vector<char> request_body;
};

