// WebClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <iostream>
#include <Windows.h>
#include "WebClient.h"

std::wstring GetUTF16(const std::string& str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

bool EndsWithCRLF(std::string data)
{
	DWORD szData = data.size();
	if (data[szData - 2] != '\r' && data[szData - 1] != '\n')
	{
		return false;
	}
	return true;
}

bool EndsWithCRLF(std::string* data)
{
	return EndsWithCRLF(*data);
}

void NormalizeCRLF(std::string* data)
{
	if (!EndsWithCRLF(data))
	{
		*data += "\r\n";
	}
}


/*
Perform a GET request to the requested resource.

@param domain            The hostname to send the request to.
@param url               Path to the resource.
@param outputBufferPtr   The buffer to hold the response of the server.
@param userAgent         User agent to send to the server.
@param additionalHeaders Other headers to send to the server besides user agent.
@param ssl               Use secure socket layer for https.
@param port              Port of webserver.
@returns int             Status of operation. 0 indicates success.
*/
int WebGETRequest(
	string domain,
	string url,
	string* outputBufferPtr,
	string userAgent = "WinHTTP Example/1.0",
	string additionalHeaders = "",
	bool ssl = true,
	int port = INTERNET_DEFAULT_HTTPS_PORT
)
{
	// Ensure data is passed
	assert(domain.size() > 0);
	assert(url.size() > 0);

	// Return value
	int status = 0;

	// Conversions to make the functions work
	wstring wcsDomain = GetUTF16(domain, CP_UTF8);
	wstring wcsUrl = GetUTF16(url, CP_UTF8);
	wstring wcsUserAgent = GetUTF16(userAgent, CP_UTF8);
	string * additional_headers_ptr = &additionalHeaders;
	wstring wcsAdditionalHeaders;
	if (additionalHeaders != "")
	{
		// All headers need to end with CRLF
		NormalizeCRLF(&additionalHeaders);
	}

	wcsAdditionalHeaders = GetUTF16(additionalHeaders, CP_UTF8);

	// Extra vars
	LPCWSTR final_content_type;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;


	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(
		wcsUserAgent.c_str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);


	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(
			hSession,
			wcsDomain.c_str(),
			port,
			0);

	// Create an HTTP request handle.
	if (hConnect)
	{
		if (ssl)
		{
			hRequest = WinHttpOpenRequest(
				hConnect,
				L"GET",
				wcsUrl.c_str(),
				NULL,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				WINHTTP_FLAG_SECURE);
		}
		else
		{
			hRequest = WinHttpOpenRequest(
				hConnect,
				L"GET",
				wcsUrl.c_str(),
				NULL,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				0);
		}
	}

	// Send a request.
	if (hRequest)
	{
		if (wcsAdditionalHeaders.size() > 0)
		{
			bResults = WinHttpSendRequest(
				hRequest,
				wcsAdditionalHeaders.c_str(),
				-1L,
				WINHTTP_NO_REQUEST_DATA,
				0,
				0,
				0);
		}
		else
		{
			bResults = WinHttpSendRequest(
				hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS,
				0,
				WINHTTP_NO_REQUEST_DATA,
				0,
				0,
				0);
		}
	}


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				status = GetLastError();
#ifdef DEBUG
				cout << "Error in WinHttpQueryDataAvailable: " << status << endl;
#endif
			}

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{
					status = GetLastError();
#ifdef DEBUG
					cout << "Error in WinHttpReadData: " << status << endl;
#endif
				}
				else
				{
					*outputBufferPtr += string(pszOutBuffer);
				}

				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}


	// Report any errors.
	if (!bResults)
	{
		status = GetLastError();
#ifdef DEBUG
		cout << "Error has occurred: " << status << endl;
#endif
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return status;
}

/*
Perform a POST request to the requested resource and passed data.

@param domain             The hostname name to send the request to.
@param url                Path to the resource.
@param post_data          The data to send to the resource.
@param content_type       The content-type of the post_data
@param outbuffer_ptr      The buffer to hold the response of the server.
@param user_agent         User agent to send to the server. Default is WinHTTP Example/1.0
@param additional_headers Other headers BESIDES content-type to send to the server.
@param ssl                Indicates protocol uses SSL
@return int               Status of the operation. 0 indicates success.
*/
int WebPOSTRequest(
	string domain,
	string url,
	string post_data,
	string content_type,
	string* outbuffer_ptr,
	string user_agent = "WinHTTP Example/1.0",
	string additional_headers = "",
	bool ssl = true,
	int port = INTERNET_DEFAULT_HTTPS_PORT)
{
	int szContentType = content_type.size();
	// Ensure data is passed
	assert(domain.size() > 0);
	assert(url.size() > 0);
	assert(post_data.size() > 0);
	assert(outbuffer_ptr != nullptr);

	// Return value
	int status = 0;

	// Conversions to make the functions work
	LPSTR data = const_cast<char*>(post_data.c_str());;
	DWORD szData = strlen(data);
	wstring wcsDomain = GetUTF16(domain, CP_UTF8);
	wstring wcsUrl = GetUTF16(url, CP_UTF8);
	wstring wcsUserAgent = GetUTF16(user_agent, CP_UTF8);
	string * additional_headers_ptr = &additional_headers;
	wstring wcsAdditionalHeaders;
	if (additional_headers != "")
	{
		NormalizeCRLF(&additional_headers);
	}
	additional_headers += content_type;
	// Headers need to end with CRLF
	NormalizeCRLF(&additional_headers);

	wcsAdditionalHeaders = GetUTF16(additional_headers, CP_UTF8);

	// Extra vars
	LPCWSTR final_content_type;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;


	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(
		wcsUserAgent.c_str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(
			hSession,
			wcsDomain.c_str(),
			port,
			0);

	// Create an HTTP request handle.
	if (hConnect)
	{
		if (ssl)
		{
			hRequest = WinHttpOpenRequest(
				hConnect,
				L"POST",
				wcsUrl.c_str(),
				NULL,                          // HTTP/1.1
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				WINHTTP_FLAG_SECURE);
		}
		else
		{
			hRequest = WinHttpOpenRequest(
				hConnect,
				L"POST",
				wcsUrl.c_str(),
				NULL,                          // HTTP/1.1
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				0);
		}
	}

	// Send a request.
	if (hRequest)
	{
		if (wcsAdditionalHeaders.size() > 0)
		{
			bResults = WinHttpSendRequest(hRequest,
				wcsAdditionalHeaders.c_str(),
				-1L,
				(LPVOID)data,
				szData,
				szData,
				0);
		}
		else
		{
			bResults = WinHttpSendRequest(hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS,
				0,
				(LPVOID)data,
				szData,
				szData,
				0);
		}
	}


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				status = GetLastError();
#ifdef DEBUG
				cout << "Error in WinHttpQueryDataAvailable: " << status << endl;
#endif
			}

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{
					status = GetLastError();
#ifdef DEBUG
					cout << "Error in WinHttpReadData: " << status << endl;
#endif
				}
				else
					*outbuffer_ptr += string(pszOutBuffer);
				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

	// Report any errors.
	if (!bResults)
	{
		status = GetLastError();
#ifdef DEBUG
		cout << "Error has occurred: " << status << endl;
#endif
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return status;

}

int WebClient::GET(
	string url,
	string* outputBufferPtr,
	string userAgent,
	string additionalHeaders)
{
	int status;
	string ua = user_agent;
	string other_headers = additional_headers;
	if (userAgent != "")
	{
		ua = userAgent;
	}

	if (additionalHeaders != "")
	{
		other_headers = additionalHeaders;
	}
	status = WebGETRequest(domain, url, outputBufferPtr, ua, other_headers, ssl, port);
	return status;
}

int WebClient::POST(
	string url,
	string postData,
	string contentType,
	string* outputBufferPtr,
	string userAgent,
	string additionalHeaders)
{
	int status;
	std::string ua = user_agent;
	std::string other_headers = additional_headers;
	if (userAgent != "")
	{
		ua = userAgent;
	}

	if (additionalHeaders != "")
	{
		other_headers = additionalHeaders;
	}
	status = WebPOSTRequest(domain, url, postData, contentType, outputBufferPtr, ua, other_headers, ssl, port);
	return status;
}

int main()
{
    WebClient msft("www.microsoft.com");
	WebClient postman("postman-echo.com");
	int status;
	string* outbuf_ptr = new string();
	string* outbuf_ptr2 = new string();
	status = msft.GET("/", outbuf_ptr);
	if (status == 0)
	{
		std::cout << "Successful GET request for microsoft.com! " << (*outbuf_ptr).size() << " strlen" << std::endl;
	}
	status = postman.POST("/post", "Hello world!", "Content-type: application/x-www-form-urlencoded", outbuf_ptr2);
	if (status == 0)
	{
		std::cout << "Successful POST to postman-echo.com!" << std::endl << *outbuf_ptr2 << std::endl;
	}
	delete outbuf_ptr;
	outbuf_ptr = nullptr;
	delete outbuf_ptr2;
	outbuf_ptr2 = nullptr;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
