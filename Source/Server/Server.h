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
                if(event == READY_GAME) {
                    ready = true;
                    received -= 4;
                } else if(event == CHAT_LOBBY) {
                    std::string str;
                    auto preOffset = serial.getOffset() - 4;
                    serial.deserialize(str);
                    
                    if(otherSocket != nullptr) {
                        // echo to the other player
                        otherSocket->send(networkData+preOffset, serial.getOffset()-preOffset+1);
                    }
                    
                    received -= 8 + str.size();
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
    }
    
    sf::TcpSocket *socketP1 = nullptr;
    sf::TcpSocket *socketP2 = nullptr;
    
    int32_t getGameID() const {return gameID;}
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
        sockets.push_back(new sf::TcpSocket());
        sf::Socket::Status status;
        if ((status = listener.accept(*sockets.back())) != sf::Socket::Done && status != sf::Socket::NotReady)
        {
            std::cerr << "could not accept connection!\n";
        }
        sockets.back()->setBlocking(false);
    }
    
    void readConnectionData() {
        for(unsigned int i = 0;i < sockets.size(); i++) {
            if(_readConnectionData(*sockets[i])) {
                sockets.erase(sockets.begin() + i);
                i--;
            }
        }
    }
    
    bool _readConnectionData(sf::TcpSocket &socket) {
        size_t leftOver = 0; // leftOver is how much from the last packet that applies to a new one (already filled)
        std::size_t received;
        
        sf::Socket::Status status;
        while((status = socket.receive(networkData+leftOver, MAX_PACKET-leftOver, received)) == sf::Socket::Partial) {
            leftOver += received;
        }
        socket.setBlocking(true);

        bool transferSocket = false;
        received += leftOver; // make it the total received
        if (status == sf::Socket::Done)
        {
            Serial serial(networkData, MAX_PACKET);
            while(received > 0) {
                NetworkEvent event;
                serial.deserialize(event);
                if(event == CREATE_GAME) {
                    socket.send((char *)&currentGameID, 4);
                    
                    //games.push_back(new ServerGame(currentGameID));
                    //games.back()->socket = &socket;
                    //std::thread t(&ServerGame::start, std::ref(*games.back()));
                    //t.detach();
                    lobbies.push_back(new ServerLobby(currentGameID));
                    lobbies.back()->socketP1 = &socket;
                    std::thread t(&ServerLobby::start, std::ref(*lobbies.back()));
                    t.detach();
                    currentGameID++;
                    
                    transferSocket = true;
                    
                    received -= 4;
                } else if(event == LIST_GAMES) {
                    uint32_t gameCount = lobbies.size();
                    socket.send((char *)&gameCount, 4);
                    for(auto l : lobbies) {
                        int32_t id = l->getGameID();
                        socket.send((char *)&id, 4);
                    }
                    // uint32_t gameCount = games.size();
                    // socket.send((char *)&gameCount, 4);
                    // for(auto g : games) {
                    //     int32_t id = g->getGameID();
                    //     socket.send((char *)&id, 4);
                    // }
                } else if(event == JOIN_GAME) {
                    int32_t id;
                    serial.deserialize(id);
                    /*for(auto g : games) {
                        if(g->getGameID() == id) {
                            g->socketP2 = &socket;
                            transferSocket = true;
                            break;
                        }
                    }*/
                    for(auto l : lobbies) {
                        if(l->getGameID() == id) {
                            l->socketP2 = &socket;
                            transferSocket = true;
                            break;
                        }
                    }
                    if(transferSocket) {
                        socket.send((char *)&id, 4);
                    } else {
                        int32_t badID = -1; // report no such game (-1)
                        socket.send((char *)&badID, 4);
                    }
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
        socket.setBlocking(false);
        return transferSocket;
    }
    
    std::vector<sf::TcpSocket *> sockets;
    
    std::vector<ServerGame *> games;
    
    std::vector<ServerLobby *> lobbies;
    
    sf::TcpListener listener;
    
    int32_t currentGameID = 0;
    
    constexpr static size_t MAX_PACKET = 1024*1; // arbitray value - 1 kB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
