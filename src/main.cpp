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

            std::stringstream readData;

            auto bytesRead = read(connfd, buffer, BUFFER_SIZE);

            while (bytesRead == BUFFER_SIZE)
            {
                readData.write(buffer, bytesRead);
                bytesRead = read(connfd, buffer, BUFFER_SIZE);
            }

            readData.write(buffer, bytesRead);

            std::cout << "Got request: " << readData.str() << std::endl;

            std::stringstream out;

            out << "Hello from brunto!\n";

            const auto res = out.str();

            write(connfd, res.c_str(), res.size());
            close(connfd);
        };

        connections.emplace_back(std::thread(handleConnection));

        // ticks = time(NULL);
        // snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
    }
}