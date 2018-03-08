#include "Client.h"

#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>


// handles the CLI main menu
class MainMenu {
public:
    MainMenu(sf::TcpSocket &socket) : socket(socket) {
        std::cerr << "enter name: ";
        std::string name;
        std::getline(std::cin, name);
        
        
        unsigned char data[50];
        Serial serial(data, 50);
        if(name.size() >= 45) {
            // leave room for null terminator and message type
            name.erase(name.begin()+45, name.end());
        }
        serial.serialize(REGISTER_PLAYER);
        serial.serialize(name);
        socket.send(data, serial.getOffset());
        
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
                int32_t strLen;
                int32_t gameID;
                socket.receive((char *)&gameID, sizeof(gameID), received);
                socket.receive((char *)&strLen, sizeof(strLen), received);
                
                char data[50];
                socket.receive(data, strLen, received);
                
                std::string pName = std::string(data, strLen);
                std::cerr << "Found game #" << gameID << " created by " << pName << "\n";
            }
            
            if(gameCount == 0) {
                std::cerr << "no games found\n";
            }
        } else if(command == "join") {
            unsigned char data[8];
            Serial serial(data, 8);
            NetworkEvent e = JOIN_GAME;
            serial.serialize(e);
            
            int32_t gameID;
            std::cin >> gameID;
            serial.serialize(gameID);
            socket.send(data, 8);
            
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
            if(command.size() >= 1018) {
                // leave room for null terminator and message type
                command.erase(command.begin()+1018, command.end());
            }
            serial.serialize(CHAT_LOBBY);
            serial.serialize(command);
            socket.send(data, serial.getOffset());
        }
        
        
        
        socket.setBlocking(false);
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
            if(event == CHAT_LOBBY) {
                std::string str;
                stream.deserialize(str);
                
                std::cerr << str << "\n";
            } else {
                std::cout << "unkown type: " << event << "\n";
            }
        }
        socket.setBlocking(true);
        
        
        
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
    
    // connect, load the menu, then once that returns, play the game
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

