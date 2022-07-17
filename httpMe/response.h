#pragma once

#include <string>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <vector>

#include "globals.h"
#include "MIME_types.h"
#include "status_codes.h"

class Response
{
	friend class HTTP_Connection;
public:
	int send_file(std::string t_file_path, std::string t_code = "200");

private:
	bool is_file = false;
	std::vector<char> response_buf;
	std::string content;

private:
	std::string get_MIME_type(std::string t_path);
	std::string file_time_to_string(std::filesystem::file_time_type time);
};

class Header
{
	friend class Response;
public:
	Header(std::string t_content_type, int t_content_length, std::string t_status, std::string t_last_modified, std::string t_server_version);
	std::string get_header();

private:
	std::string get_UTC_date();
	std::string get_day_of_week(tm* t_tm);
	std::string get_two_digit_day(tm* t_tm);
	std::string get_month(tm* t_tm);
	std::string get_two_digit_hour(tm* t_tm);
	std::string get_two_digit_minute(tm* t_tm);
	std::string get_two_digit_second(tm* t_tm);

private:
	int content_length;
	std::string content_type;
	std::string status;
	std::string date;
	std::string last_modified;
	std::string server_version;
};