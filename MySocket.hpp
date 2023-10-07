#pragma once

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

namespace myUtility
{
    class MySocket
    {
    public:
        static int Socket()
        {
            int socketId = socket(AF_INET, SOCK_STREAM, 0);
            if(socketId == -1)
            {
                std::cerr << "Socket Error! " << errno << std::endl;
                exit(2);
            }
            return socketId;
        }

        static void Bind(int socketId, uint16_t port)
        {
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(port);
            local.sin_addr.s_addr = INADDR_ANY; 
            
            if(bind(socketId, (struct sockaddr*)&local, sizeof(local)) == -1)
            {
                std::cerr << "Bind Error! " << errno << std::endl;
                exit(3);
            }
        }

        static void Listen(int socketId)
        {
            if(listen(socketId, 5) == -1)
            {
                std::cerr << "Listen Error! " << errno << std::endl;
                exit(4);
            }
        }

        static int Accept(int socketId)
        {
            struct sockaddr_in peer;
            memset(&peer, 0, sizeof(peer));
            socklen_t len = sizeof(peer);
            int fd = accept(socketId, (struct sockaddr*)&peer, &len);
            if(fd == -1)
            {
                std::cerr << "Accept Error! " << errno << std::endl;
                exit(5);
            }
            else
            {   
                return fd;
            }
        }

        static void Close(int socket)
        {
            close(socket);
        }
    };
}