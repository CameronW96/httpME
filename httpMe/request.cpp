#include "request.h"

Request::Request()
{
}

Request::Request(std::string t_http_version, std::string t_request_type, std::string t_request_path, std::map<std::string, std::vector<std::string>> t_request_header_fields, std::vector<char> t_request_body)
	: http_version(t_http_version), request_type(t_request_type), request_path(t_request_path), request_header_fields(t_request_header_fields), request_body(t_request_body)
{
}

bool Request::is_file()
{
	std::string::reverse_iterator it = request_path.rbegin();

	while (it != request_path.rend())
	{
		if (*it == '.')
		{
			return true;
		}
		it++;
	}
	return false;
}
