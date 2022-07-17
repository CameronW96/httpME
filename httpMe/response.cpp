//
//
#include <chrono>
#include <filesystem>
#include <iostream>
#include <ios>
#include <sstream>

#include "http_connection.h"
#include "response.h"


// Header
Header::Header(std::string t_content_type, int t_content_length, std::string t_status, std::string t_last_modified, std::string t_server_version)
	: content_type(t_content_type), content_length(t_content_length), status(t_status), last_modified(t_last_modified), server_version(t_server_version)
{
}

std::string Header::get_header()
{
	std::string header = "";

		header += "HTTP/1.1 ";
		header += status;
		header += "\r\n";
		header += "Date: ";
		header += get_UTC_date();
		header += "\r\n";
		header += "Server: httpMe/" + server_version;
		header += "\r\n";
		header += "Last-Modified: " + last_modified;
		header += "\r\n";
		header += "Content-Length: " + std::to_string(content_length);
		header += "\r\n";
		header += "Content-Type: " + content_type;
		header += "\r\n";
		header += "Connection: Closed";
		header += "\r\n\r\n";

		return header;

}

std::string Header::get_UTC_date()
{
	std::time_t now;
	time(&now);
	char buf[100];
	tm* gmtm = gmtime(&now);

	strftime(buf, 100, "%a, %d %b %Y %T GMT", gmtm);

	return std::string(buf);
}

// Response

int Response::send_file(std::string t_file_path, std::string t_code)
{
	response_buf.clear(); // Prevent stacking multiple files in the buffer

	std::ifstream file(t_file_path, std::ios::binary);
	if (file.is_open())
	{
		Status_Codes status_codes;

		std::stringstream file_data;
		file_data << file.rdbuf();
		file_data.seekg(0, std::ios::end);
		int data_size = file_data.tellg();

		auto last_modified = std::filesystem::last_write_time(t_file_path);

		Header header(get_MIME_type(t_file_path), data_size, status_codes.get_status(t_code), file_time_to_string(last_modified), std::string("0.1"));

		std::stringstream resbuf;
		resbuf << header.get_header();
		resbuf << file_data.str();

		file_data.clear(); // reduce memory usage when sending large files

		std::string output = resbuf.str();
		response_buf = std::vector<char>(output.begin(), output.end());

		return response_buf.size();
	}
	else
	{
		// 500 internal server error
		std::cout << "500 Internal Server Error" << std::endl;
	}
}

std::string Response::get_MIME_type(std::string t_path)
{
	MIME_Types mime;
	std::string::iterator it = t_path.end();
	std::string extension;

	if (t_path.size() < 1) { return "NULL"; }
	it--;

	while (it != t_path.begin() && *it != '.')
	{
		if (*it == '\\' || *it == '/')
		{
			it--;
		}
		else
		{
			extension += *it;
			it--;
		}
	}
	std::reverse(extension.begin(), extension.end());

	auto m_it = mime.MIME_map.find(extension);
	if (m_it != mime.MIME_map.end())
	{
		return m_it->second;
	}
	else
	{
		return "NULL";
	}
}

std::string Response::file_time_to_string(std::filesystem::file_time_type t_time)
{
	std::time_t last_write_time_t = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(t_time));

	struct tm* time_info;
	char buf[100];
	time_info = gmtime(&last_write_time_t);
	strftime(buf, 100, "%a, %d %b %Y %T GMT", time_info);

	return std::string(buf);
}



