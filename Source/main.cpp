#include "Server.h"
#include "Client.h"

#include "Rectangle.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>




int main(int argc, char *argv[])
{
    std::vector<std::string> args;
    for(int i = 1;i < argc; i++) {
        args.emplace_back(argv[i]);
    }
    
    
    bool isServer = false;
    for(int i = 0;i < args.size(); i++) {
        if(args[i] == "--server") {
            isServer = true;
        } else {
            std::cerr << "unkown option: " << args[i] << "! Aborting...\n";
            return -1;
        }
    }
    
    if(isServer)
        Server server;
    else
        Client client;


    return 0;
}

