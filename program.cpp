#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <map>
#include <algorithm>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "443"

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

void parseResponse(std::string& allData)
{
    std::string _method;
    std::string _uri;
    std::map<std::string, std::string> _headers;
    std::string _payload;

    auto pos = allData.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        std::stringstream ss;
        ss.str(allData.substr(0, pos));
        std::string line;

        // Determine methode, uri and HTTP version
        if (std::getline(ss, line))
        {
            trim(line);
            std::cout << line << std::endl;
            auto first = line.find_first_of(' ');
            auto second = line.find_first_of(' ', first + 1);
            if (first != std::string::npos && second != std::string::npos)
            {
                auto version = line.substr(second);
//                if (trim(version) != "HTTP/1.1") throw new std::runtime_error("invalid HTTP version");

                auto method = line.substr(0, first);
                _method = trim(method);

                auto uri = line.substr(first, second-first);
                _uri = trim(uri);
            }
        }

        // Determine headers
        while (std::getline(ss, line))
        {
            auto pos = line.find_first_of(':');
            if (pos != std::string::npos)
            {
                auto key = line.substr(0, pos);
                auto value = line.substr(pos + 1);
                _headers.insert(std::make_pair(trim(key), trim(value)));
            }
        }

        // And finally determine payload(if any)
        _payload = allData.substr(pos + 4);
    }

    for (auto pair : _headers)
    {
        std::cout << pair.first << ":" << pair.second << std::endl;
    }

    std::cout << _payload << std::endl;
}

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    std::string data = "entry.1595413962=Naam&entry.635739924=Data";

    // http://docs.google.com/forms/d/e/1FAIpQLSftLcKlBFTAodyDKI8kzZcDcc6uBUbsfp4cpQLiGbFuFT58_Q/formResponse
    std::stringstream ss;
    ss << "POST /forms/d/e/1FAIpQLSftLcKlBFTAodyDKI8kzZcDcc6uBUbsfp4cpQLiGbFuFT58_Q/formResponse HTTP/1.1\r\n"
       << "Host: docs.google.com\r\n"
       << "Content-type: application/x-www-form-urlencoded; charset=utf-8\r\n"
       << "Content-length: " << data.size() << "\r\n"
       << "Connection: close\r\n"
       << "\r\n"
       << data;
    char recvbuf[DEFAULT_BUFLEN + 1];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("docs.google.com", DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        setsockopt(ConnectSocket, SOL_SOCKET, SO_SECURE, (DWORD)SO_SEC_SSL, 0);

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send( ConnectSocket, ss.str().c_str(), (int)ss.str().size(), 0 );
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::string response;
    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
        {
            recvbuf[iResult] = '\0';
            printf("Bytes received: %d\n", iResult);
            response += recvbuf;
        }
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    parseResponse(response);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
