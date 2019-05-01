#pragma once

#include <iostream>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")
using namespace std;

class WebClient
{
public:
	WebClient(
		string domainName,
		int portNum = INTERNET_DEFAULT_HTTPS_PORT,
		bool useSSL = true,
		string userAgent = "WinHTTP Example/1.0",
		string additionalHeaders = "")
	{
		domain = domainName;
		port = portNum;
		ssl = useSSL;
		user_agent = userAgent;
		additional_headers = additionalHeaders;
	}

	int GET(string url, string* outputBufferPtr, string userAgent="", string additionalHeaders="");
	int POST(string url, string postData, string contentType, string* outputBufferPtr, string userAgent = "", string additionalHeaders = "");
	// Implement PUT as well later
	string domain;
	int port;
	bool ssl;
	string user_agent;
	string additional_headers;
};