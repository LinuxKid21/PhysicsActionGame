#include "Client.h"

#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>




int main(int argc, char *argv[])
{
    std::string serverIP = "127.0.0.1";
    if(argc >= 2) {
        serverIP = argv[1];
    } 
    Client client(serverIP);

    return 0;
}

