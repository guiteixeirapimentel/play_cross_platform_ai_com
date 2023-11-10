#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <sstream>
#include <iostream>
#include <string>
#include <future>
#include <vector>
#include <algorithm>

#include "SharedStructs.hpp"

using namespace pimentel;

void sendAkn(int connfd)
{
    const auto msg = Aknowledge{ .m_valid = true, .m_message = "Hello from the server!" };

	const auto msgPacketBytes = serializeMessage(generateMessagePacket(msg));

    write(connfd, msgPacketBytes.data(), msgPacketBytes.size());
}

struct ServerVisitor : public pimentel::MessageVisitor
{
    void visit(const Aknowledge& akn) override
    {
        std::cout << "Got akn msg with content " << akn.m_message << " valid? " << akn.m_valid << std::endl;


    }

    void handleInvalidData(std::error_code ec) override
    {
		std::cout << "Handling error code " << ec << std::endl;
    }

};

std::vector<std::thread> connections;

int main(int, char **)
{
    static constexpr auto port = 8090;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        std::cout << "Failed to bind on port " << port << std::endl;
        return 1;
    }

    listen(listenfd, 10);

    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);

        struct timeval tv;
        tv.tv_sec = 15;
        tv.tv_usec = 0;
        setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        const auto handleConnection = [connfd]()
        {
            static constexpr auto BUFFER_SIZE = 4096;
            char buffer[BUFFER_SIZE];

            std::vector<uint8_t> readData;

            auto bytesRead = read(connfd, buffer, BUFFER_SIZE);

            while (bytesRead == BUFFER_SIZE)
            {
                std::copy(buffer, buffer + bytesRead, std::back_inserter(readData));
                bytesRead = read(connfd, buffer, BUFFER_SIZE);
            }

            std::copy(buffer, buffer + bytesRead, std::back_inserter(readData));

            std::cout << "Got request size: " << readData.size() << std::endl;
            
            ServerVisitor sv;

            handleData(readData, sv);

            sendAkn(connfd);

            close(connfd);
        };

        connections.emplace_back(std::thread(handleConnection));

        // ticks = time(NULL);
        // snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
    }
}