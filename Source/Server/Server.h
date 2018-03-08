#pragma once
#include "../Share/Rectangle.h"
#include "../Share/Serial.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <SFML/Network.hpp>

#include "ServerGame.h"

class ServerLobby {
public:
    ServerLobby(int32_t gameID) : gameID(gameID) {
        ;
    }
    
    void start() {
        bool ready1 = false;
        bool ready2 = false;
        while(!ready1 || !ready2) {
            run(*socketP1, socketP2, ready1);
            if(socketP2)
                run(*socketP2, socketP1, ready2);
        }
        
        
        NetworkEvent e = START_GAME;
        socketP1->send((char *)&e, sizeof(e));
        socketP2->send((char *)&e, sizeof(e));
        
        ServerGame game(gameID);
        game.socket = socketP1;
        game.socketP2 = socketP2;
        game.start();
    }
    
    void run(sf::TcpSocket &socket, sf::TcpSocket *otherSocket, bool &ready) {
        socket.setBlocking(false);
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
                
            if(event == READY_GAME) {
                ready = true;
            } else if(event == CHAT_LOBBY) {
                std::string str;
                stream.deserialize(str);
                
                if(otherSocket != nullptr) {
                    socket.setBlocking(true);
                    unsigned char data[1024];
                    Serial serial(data, 1024);
                    serial.serialize(CHAT_LOBBY);
                    serial.serialize(str);
                    // echo to the other player
                    otherSocket->send(data, serial.getOffset());
                    socket.setBlocking(false);
                }
            } else {
                std::cout << "unkown type: " << event << "\n";
            }
        }
        
        
        socket.setBlocking(true);
    }
    
    sf::TcpSocket *socketP1 = nullptr;
    sf::TcpSocket *socketP2 = nullptr;
    
    std::string nameP1;
    std::string nameP2;
    
    int32_t getGameID() const {return gameID;}
    
    bool lobbyFull = false;
private:
    int32_t gameID;
};

class Server {
public:
    //---------------------------------------------------------------------------------------------------- 
    Server()
    {
        if (listener.listen(54000) != sf::Socket::Done)
        {
            std::cerr << "ERROR!\n";
        }
        
        listener.setBlocking(false);
        while(true) {
            acceptNewConnections();
            readConnectionData();
        }
    }
private:
    void acceptNewConnections() {
        sf::TcpSocket *socket = new sf::TcpSocket();
        sf::Socket::Status status = listener.accept(*socket);
        if (status == sf::Socket::NotReady) {
            
        } else if(status != sf::Socket::Done) {
            std::cerr << "could not accept connection!\n";
            delete socket;
        } else {
            socket->setBlocking(false);
            sockets.push_back(socket);
            playerNames.push_back("");
        }
    }
    
    void readConnectionData() {
        for(unsigned int i = 0;i < sockets.size(); i++) {
            if(_readConnectionData(*sockets[i], playerNames[i])) {
                sockets.erase(sockets.begin() + i);
                playerNames.erase(playerNames.begin() + i);
                i--;
            }
        }
    }
    
    bool _readConnectionData(sf::TcpSocket &socket, std::string &name) {
        
        bool transferSocket = false;
        socket.setBlocking(false);
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
                
            if(event == REGISTER_PLAYER) {
                socket.setBlocking(true);
                stream.deserialize(name);
                
                socket.setBlocking(false);
            } else if(event == CREATE_GAME) {
                socket.setBlocking(true);
                socket.send((char *)&currentGameID, 4);
                
                
                lobbies.push_back(new ServerLobby(currentGameID));
                lobbies.back()->socketP1 = &socket;
                lobbies.back()->nameP1 = name;
                std::thread t(&ServerLobby::start, std::ref(*lobbies.back()));
                t.detach();
                currentGameID++;
                
                transferSocket = true;
                
                socket.setBlocking(false);
            } else if(event == LIST_GAMES) {
                socket.setBlocking(true);
                uint32_t gameCount = lobbies.size();
                socket.send((char *)&gameCount, 4);
                for(auto l : lobbies) {
                    if(!l->lobbyFull) {
                        unsigned char data[50];
                        Serial serial(data, 50);
                        serial.serialize(l->getGameID());
                        serial.serialize(l->nameP1);
                        socket.send(data, serial.getOffset());
                    }
                }
                
                socket.setBlocking(false);
            } else if(event == JOIN_GAME) {
                socket.setBlocking(true);
                int32_t id;
                stream.deserialize(id);
                
                for(auto l : lobbies) {
                    if(l->getGameID() == id && !l->lobbyFull) {
                        l->socketP2 = &socket;
                        l->nameP2 = name;
                        transferSocket = true;
                        l->lobbyFull = true;
                        break;
                    }
                }
                if(transferSocket) {
                    socket.send((char *)&id, 4);
                } else {
                    int32_t badID = -1; // report no such game (-1)
                    socket.send((char *)&badID, 4);
                }
                socket.setBlocking(false);
            } else {
                std::cout << "unkown type: " << event << "\n";
            }
        }
        socket.setBlocking(false);
        return transferSocket;
    }
    
    std::vector<sf::TcpSocket *> sockets;
    std::vector<std::string> playerNames; // playerNames[i] is associated with sockets[i]
    
    std::vector<ServerLobby *> lobbies;
    
    sf::TcpListener listener;
    
    int32_t currentGameID = 0;
    
    constexpr static size_t MAX_PACKET = 1024*1; // arbitray value - 1 kB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
