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


class ServerGame {
public:
    ServerGame(int32_t id) : world(b2Vec2(0.0f,10.f)), gameID(id)
    {
    }
    
    ~ServerGame() {
        ;
    }
    
    void start() {
        onStart();
        while (true)
        {
            /*sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                handleGameEvent(event);
            }*/

            float delta = clock.restart().asSeconds();
            timeSinceUpdate += delta;

            while(timeSinceUpdate >= timeStep) {
                timeSinceUpdate -= timeStep;
                world.Step(timeStep, velocityIterations, positionIterations);
                update();
            }
        }
    }
    
    sf::TcpSocket *socket = nullptr;
    sf::TcpSocket *socketP2 = nullptr;
    
    int32_t getGameID() const {return gameID;}
private:
    void onStart() {
        // rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(1,0), sf::Vector2f(.5, 1.7), 45, false));
        rectangleEntities.push_back(PhysicsRectangle(world, true, sf::Vector2f(7.5,10), sf::Vector2f(15, 1), 0, currentRectID));

        float width = 2;
        float height = 2;
        float thickness = .25;
        makePlatform(sf::Vector2f(1+width/2.f,10-.5-height/2.f), sf::Vector2f(width, height), thickness);
        makePlatform(sf::Vector2f(1+width/2.f + 2*(width-thickness),10-.5-height/2.f), sf::Vector2f(width, height), thickness);
        makePlatform(sf::Vector2f(1+width/2.f + 4*(width-thickness),10-.5-height/2.f), sf::Vector2f(width, height), thickness);

        makePlatform(sf::Vector2f(1+width/2.f + width-thickness,10-.5-height/2.f - height), sf::Vector2f(width, height), thickness);
        makePlatform(sf::Vector2f(1+width/2.f + 3*(width-thickness),10-.5-height/2.f - height), sf::Vector2f(width, height), thickness);

        makePlatform(sf::Vector2f(1+width/2.f + 2*(width-thickness),10-.5-height/2.f - height*2), sf::Vector2f(width, height), thickness);
    }
    

    void update() {
        for(auto &e : rectangleEntities) {
            e.update(*socket);
            if(socketP2)
                e.update(*socketP2);
        }
        
        handleInput(*socket);
        if(socketP2)
            handleInput(*socketP2);
    }
    
    void handleInput(sf::TcpSocket &socket) {
        socket.setBlocking(false);
        size_t leftOver = 0; // leftOver is how much from the last packet that applies to a new one (already filled)
        std::size_t received;
        
        sf::Socket::Status status;
        while((status = socket.receive(networkData+leftOver, MAX_PACKET-leftOver, received)) == sf::Socket::Partial) {
            leftOver += received;
        }

        received += leftOver; // make it the total received
        if (status == sf::Socket::Done)
        {
            Serial serial(networkData, MAX_PACKET);
            while(received > 0) {
                NetworkEvent event;
                serial.deserialize(event);
                if(event == CREATE_RECTANGLE) {
                    float x;
                    float y;
                    serial.deserialize(x);
                    serial.deserialize(y);
                    
                    rectangleEntities.push_back(PhysicsRectangle(world, false,
                        sf::Vector2f(x,y), sf::Vector2f(.1, .1), 0, currentRectID));
                    
                    received -= 12;
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
    }

    // handles non-window events
    void handleGameEvent(const sf::Event &event) {
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
            for(int i = 0;i < 25; i++)
                rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.1, .1), 0, currentRectID));
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.5, 1.7), 45, currentRectID));
    }

    //---------------------------------------------------------------------------------------------------- 
    void makePlatform(sf::Vector2f pos, sf::Vector2f size, float thickness) {
        float legHeight = size.y-thickness;
        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x+size.x/2.f-thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x-size.x/2.f+thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x, pos.y -size.y/2.f+thickness/2.f),
                                                    sf::Vector2f(size.x, thickness), 0, currentRectID));
    }


    
    b2World world;
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;
    sf::Clock clock;
    float timeSinceUpdate = 0;

    std::vector<PhysicsRectangle> rectangleEntities;
    int currentRectID = 0;
    int gameID;
    
    constexpr static size_t MAX_PACKET = 1024*1024*1; // arbitray value - 1 MB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
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
                    
                    games.push_back(new ServerGame(currentGameID));
                    games.back()->socket = &socket;
                    std::thread t(&ServerGame::start, std::ref(*games.back()));
                    t.detach();
                    currentGameID++;
                    
                    transferSocket = true;
                    
                    received -= 4;
                } else if(event == LIST_GAMES) {
                    uint32_t gameCount = games.size();
                    socket.send((char *)&gameCount, 4);
                    for(auto g : games) {
                        int32_t id = g->getGameID();
                        socket.send((char *)&id, 4);
                    }
                } else if(event == JOIN_GAME) {
                    int32_t id;
                    serial.deserialize(id);
                    for(auto g : games) {
                        if(g->getGameID() == id) {
                            g->socketP2 = &socket;
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
    sf::TcpListener listener;
    
    int32_t currentGameID = 0;
    
    constexpr static size_t MAX_PACKET = 1024*1024*1; // arbitray value - 1 MB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
