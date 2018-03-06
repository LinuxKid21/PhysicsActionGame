#include "Client.h"

#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>



class MainMenu {
public:
    MainMenu(sf::TcpSocket &socket) : socket(socket) {
        firstMenu();
    }
private:
    enum SCREENS {
        FIRST_SCREEN,
        LOBBY_SCREEN,
        DONE,
    };
    
    void firstMenu() {
        std::cerr << "enter command: ";
        std::string command;
        std::cin >> command;
        
        SCREENS dest = FIRST_SCREEN;
        if(command == "create") {
            NetworkEvent e = CREATE_GAME;
            socket.send((char *)&e, sizeof(e));
            
            int32_t gameID;
            size_t received;
            socket.receive((char *)&gameID, sizeof(gameID), received);
            
            std::cerr << "joined game #" << gameID << "!\n";
            dest = LOBBY_SCREEN;
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
            }
            
            dest = LOBBY_SCREEN;
        } else {
            std::cout << "UNKOWN COMMAND!\n";
        }
        
        if(dest == FIRST_SCREEN) {
            firstMenu();
        } else if(dest == LOBBY_SCREEN) {
            std::cerr << "\n\nenter \"ready\" (without quotes) or type anything else to chat\n";
            std::cerr << "enter a blank line to get incoming chat\n";
            lobbyScreen();
        }
    }
    
    
    void lobbyScreen() {
        std::string command;
        std::getline(std::cin, command);
        
        bool ready = false;
        if(command == "ready") {
            NetworkEvent e = READY_GAME;
            socket.send((char *)&e, sizeof(e));
            
            
            size_t received;
            socket.receive((char *)&e, sizeof(e), received);
            if(e != START_GAME) {
                std::cerr << "server sent unexpected message! (expected start message)\n";
            }
            ready = true;
        } else if(command == "") {
            /* do nothing! */
            /* will reach the end and let incoming chat in though */
        } else {
            unsigned char data[1024]; // 1Kb message limit
            Serial serial(data, 1024);
            if(command.size() >= 1024) {
                // leave room for null terminator and message type
                command.erase(command.begin()+1018, command.end());
            }
            serial.serialize(CHAT_LOBBY);
            serial.serialize(command);
            socket.send(data, serial.getOffset());
        }
        
        
        
        constexpr static size_t MAX_PACKET = 1024;
        unsigned char networkData[MAX_PACKET]; // 1Kb
        size_t leftOver = 0; // leftOver is how much from the last packet that applies to a new one (already filled)
        std::size_t received;
        
        socket.setBlocking(false);
        sf::Socket::Status status;
        while((status = socket.receive(networkData+leftOver, MAX_PACKET-leftOver, received)) == sf::Socket::Partial) {
            leftOver += received;
        }
        socket.setBlocking(true);

        received += leftOver; // make it the total received
        if (status == sf::Socket::Done)
        {
            Serial serial(networkData, MAX_PACKET);
            while(received > 0) {
                NetworkEvent event;
                serial.deserialize(event);
                if(event == CHAT_LOBBY) {
                    std::string str;
                    serial.deserialize(str);
                    
                    std::cerr << str << "\n";

                    received -= 8 + str.size();
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
        
        
        
        
        if(!ready) lobbyScreen();
    }
    
    sf::TcpSocket &socket;
};



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
    
    MainMenu menu(socket);
    
    Client client(socket);

    return 0;
}

