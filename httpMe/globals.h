#ifndef GLOBALS
#define GLOBALS

// Error page locations
#define NOT_FOUND "C:\\Users\\mynam\\source\\repos\\httpMe\\httpMe\\404.htm"
#define SERVER_ERROR "C:\\Users\\mynam\\source\\repos\\httpMe\\httpMe\\500.htm"

// Config settings
const int MAX_REQ_SIZE = 8192;
const bool KEEP_ALIVE_ENABLE = true;
const int KEEP_ALIVE_MAX_CONNECTIONS = 1000;
const int RCV_TIMEOUT_SECONDS = 5;
const int SEND_TIMEOUT_SECONDS = 5;


#endif // !GLOBALS