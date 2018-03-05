#pragma once
#include "../Share/NetworkEvents.h"
#include "../Share/Serial.h"
#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class Client {
public:
    Client() : window(sf::VideoMode(192*3, 180*3), "THE GAME")
    {
        window.setView(view);
        onStart();
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                handleGameEvent(event);
            }

            float delta = clock.restart().asSeconds();
            timeSinceUpdate += delta;

            update();

            window.clear();
            draw();
            window.display();
        }
        
        socket.disconnect();
    }

private:
    void onStart() {
        sf::Socket::Status status = socket.connect("127.0.0.1", 54000);
        if (status != sf::Socket::Done)
        {
            std::cerr << "ERROR!!!!!!!!\n";
        }
        
        loadMainMenu();
        
        socket.setBlocking(false);
    }

    void update() {
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
                    
                    int32_t id;
                    sf::Vector2f pos;
                    sf::Vector2f size;
                    float rotation;
                    serial.deserialize(id);
                    serial.deserialize(pos.x);
                    serial.deserialize(pos.y);
                    serial.deserialize(size.x);
                    serial.deserialize(size.y);
                    serial.deserialize(rotation);
                    
                    // search for the index
                    int idx = -1;
                    for(unsigned int i = 0;i < rectangleEntities.size(); i++) {
                        if(rectangleEntities[i].id == id) {
                            idx = static_cast<unsigned int>(i);
                            break;
                        }
                    }
                    
                    // if it doesn't exist, create it! Otherwise update it
                    if(idx == -1) {
                        rectangleEntities.emplace_back(pos, size, rotation, id);
                    } else {
                        rectangleEntities[idx].update(pos, size, rotation);
                    }
                    received -= 28;
                } else {
                    std::cout << "unkown type: " << event << " with recieved: " << received << " with offset: " << serial.getOffset() << "\n";
                    received = 0;
                }
            }
        }
    }

    void draw() {
        for(auto &e : rectangleEntities) {
            e.draw(window);
        }
    }

    // handles non-window events
    void handleGameEvent(const sf::Event &event) {
        float x = event.mouseButton.x/1920.f*19.2;
        float y = event.mouseButton.y/1080.f*10.8;
        
        
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            constexpr size_t size = 4 /*event*/ + 2*4 /* 2 floats */; 
            unsigned char data[size];
            Serial serial(data, size);
            serial.serialize(CREATE_RECTANGLE);
            serial.serialize(x);
            serial.serialize(y);
            
            socket.setBlocking(true);
            socket.send(data, size);
            socket.setBlocking(false);
        }
        else {
            return;
        }
        
    }
    
    void loadMainMenu() {
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
        
        loadMainMenu(); // loop again!
    }
    
    //---------------------------------------------------------------------------------------------------- 
    sf::RenderWindow window;
    
    const float32 timeStep = 1.0f / 60.0f;
    sf::Clock clock;
    float timeSinceUpdate = 0;
    sf::View view = sf::View(sf::FloatRect(0, 0, 19.20, 10.80));

    std::vector<NetworkRectangle> rectangleEntities;
    
    sf::TcpSocket socket;
    
    constexpr static size_t MAX_PACKET = 1048*100; // arbitray value - 100 kB
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
