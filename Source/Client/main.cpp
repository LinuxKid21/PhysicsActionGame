#include "Client.h"

#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>



void loadMainMenu(sf::TcpSocket &socket) {
    std::cerr << "enter command: ";
    std::string command;// = "create";
    std::cin >> command;
    
    if(command == "create") {
        NetworkEvent e = CREATE_GAME;
        socket.send((char *)&e, sizeof(e));
        
        int32_t gameID;
        size_t received;
        socket.receive((char *)&gameID, sizeof(gameID), received);
        
        std::cerr << "joined game #" << gameID << "!\n";
        return;
    } else if(command == "list") {
        NetworkEvent e = LIST_GAMES;
        socket.send((char *)&e, sizeof(e));
        
        uint32_t gameCount;
        size_t received;
        socket.receive((char *)&gameCount, sizeof(gameCount), received);
        
        for(unsigned int i = 0;i < gameCount; i++) {
            int32_t gameID;
            socket.receive((char *)&gameID, sizeof(gameID), received);
            std::cerr << "Found game #" << gameID << "\n";
        }
        
        if(gameCount == 0) {
            std::cerr << "no games found\n";
        }
    } else if(command == "join") {
        NetworkEvent e = JOIN_GAME;
        socket.send((char *)&e, sizeof(e));
        
        int32_t gameID;
        std::cin >> gameID;
        socket.send((char *)&gameID, sizeof(gameID));
        
        size_t received;
        int32_t returnedGameID;
        socket.receive((char *)&returnedGameID, sizeof(returnedGameID), received);
        if(returnedGameID != gameID) {
            std::cerr << "unable to join the game!\n";
        } else {
            std::cerr << "game joined\n";
            return;
        }
        
    } else {
        std::cout << "UNKOWN COMMAND!\n";
    }
    
    loadMainMenu(socket); // loop again!
}

int main(int argc, char *argv[])
{
    
    std::string serverIP = "127.0.0.1";
    if(argc >= 2) {
        serverIP = argv[1];
    }
    
    sf::TcpSocket socket;
    sf::Socket::Status status = socket.connect(serverIP, 54000);
    if (status != sf::Socket::Done)
    {
        std::cerr << "ERROR!!!!!!!!\n";
    }
    
    loadMainMenu(socket);
    
    Client client(socket);

    return 0;
}

