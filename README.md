# WebClient

Simple C++ library to wrap web requests using standardized C++ types.

## Instantiation

The only required arguments for instantiation are the domain name and port number it operates over. The class assumes you'll be speaking https. If not, set SSL to false.

If you wish to use a different user agent, ensure you specify it. Additional headers, such as Host headers, can be specified using the `additionalHeaders` argument; however, this field should **not** contain User-agent or Content-type strings.

## Methods

Only methods on the class currently are GET and POST. They return the status of the operation, where 0 indicates success. They both require the output buffer for the response of the server.

By default, when issuing GET and POST, it'll default to using the headers passed in the constructor. If you wish to override that functionality, you can explicitly set them in the function call or by setting them on the class object.

```C++
// Function definitions for WebClient
int GET(string url, string* outputBufferPtr, string userAgent="", string additionalHeaders="");
int POST(string url, string postData, string contentType, string* outputBufferPtr, string userAgent = "", string additionalHeaders = "");
  ```

```C++
WebClient(
		string domainName,
		int portNum = INTERNET_DEFAULT_HTTPS_PORT,
		bool useSSL = true,
		string userAgent = "WinHTTP Example/1.0",
		string additionalHeaders = "")
```


## Usage
```C++
#include <iostream>
#include "WebClient.h"

int main()
{
  // Initiates over https with default UA
  WebClient msft("www.microsoft.com");
  std::string* html_ptr = new string();
  int status = msft.GET("/", html_ptr);
  if (status == 0)
  {
    // Request completed successfully
    std::cout << *html_ptr << std::endl;
  }
  else
  {
    std::cout << "Error: " << status << std::endl;
  }
}
```
