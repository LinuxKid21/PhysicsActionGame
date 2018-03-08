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
#include "ServerLobby.h"


class Server {
public:
    // the server object continually spins on waiting for new connections and
    // handling existing connections which have not yet entered the lobby stage
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
            
            // naive because it does this even if there is a ton of traffic,
            // but as it stands this is better than consuming 100% CPU usage
            // in very small traffic conditions
            sf::sleep(sf::seconds(0.1f));
        }
    }
private:
    // handles accepting new connections
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
    
    // handles reading in data from all connection sockets
    void readConnectionData() {
        for(unsigned int i = 0;i < sockets.size(); i++) {
            // hand each socket to be processed.
            // if it returns true that means the player has moved onto
            // a lobby so the socket is no longer handled here
            if(_readConnectionData(*sockets[i], playerNames[i])) {
                sockets.erase(sockets.begin() + i);
                playerNames.erase(playerNames.begin() + i);
                i--;
            }
        }
    }
    
    // handle each socket connections
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
};
