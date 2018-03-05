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
    ServerGame() : world(b2Vec2(0.0f,10.f))
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
    
    sf::TcpSocket *socket;
private:
    void onStart() {
        // rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(1,0), sf::Vector2f(.5, 1.7), 45, false));
        rectangleEntities.push_back(PhysicsRectangle(world, true, sf::Vector2f(7.5,10), sf::Vector2f(15, 1), 0, *socket, currentRectID));

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
            e.update();
        }
    }

    // handles non-window events
    void handleGameEvent(const sf::Event &event) {
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
            for(int i = 0;i < 25; i++)
                rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.1, .1), 0, *socket, currentRectID));
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.5, 1.7), 45, *socket, currentRectID));
    }

    //---------------------------------------------------------------------------------------------------- 
    void makePlatform(sf::Vector2f pos, sf::Vector2f size, float thickness) {
        float legHeight = size.y-thickness;
        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x+size.x/2.f-thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, *socket, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x-size.x/2.f+thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, *socket, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x, pos.y -size.y/2.f+thickness/2.f),
                                                    sf::Vector2f(size.x, thickness), 0, *socket, currentRectID));
    }


    
    b2World world;
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;
    sf::Clock clock;
    float timeSinceUpdate = 0;

    std::vector<PhysicsRectangle> rectangleEntities;
    int currentRectID = 0;
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
        
        std::thread t1(&Server::acceptNewConnections, this);
        std::thread t2(&Server::readConnectionData, this);
        
        t1.join();
        t2.join();
    }
private:
    void acceptNewConnections() {
        while(true) {
            socketsMutex.lock();
            sockets.push_back(new sf::TcpSocket());
            socketsMutex.unlock();
            if (listener.accept(*sockets.back()) != sf::Socket::Done)
            {
                std::cerr << "could not accept connection!\n";
            }
            games.push_back(new ServerGame());
            games.back()->socket = sockets.back();
            std::thread t(&ServerGame::start, std::ref(*games.back()));
            t.detach();
        }
    }
    
    void readConnectionData() {
        while(true) {
            socketsMutex.lock();
            for(unsigned int i = 0;i < sockets.size(); i++) {
                _readConnectionData(*sockets[i]);
            }
            socketsMutex.unlock();
        }
    }
    
    void _readConnectionData(sf::TcpSocket &socket) {
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
                if(event == RECTANGLE_UPDATE) {
                    
                    received -= 28;
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
        socket.setBlocking(true);
    }
    
    std::vector<sf::TcpSocket *> sockets;
    std::mutex socketsMutex;
    
    std::vector<ServerGame *> games;
    sf::TcpListener listener;
    
    constexpr static size_t MAX_PACKET = 1048*100; // arbitray value - 100 kB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
