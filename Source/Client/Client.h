#pragma once
#include "../Share/NetworkEvents.h"
#include "../Share/Serial.h"
#include "../Share/Rectangle.h"

#include <iostream>
#include <vector>
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
    }

private:
    void onStart() {
        sf::Socket::Status status = socket.connect("127.0.0.1", 54000);
        if (status != sf::Socket::Done)
        {
            std::cerr << "ERROR!!!!!!!!\n";
        }
        socket.setBlocking(false);
    }

    void update() {
        std::size_t received;
        if (socket.receive(networkData+leftOver, MAX_PACKET-leftOver, received) == sf::Socket::Done)
        {
            Serial serial(networkData, MAX_PACKET);
            leftOver = 0; // reset leftOver
            while(received > 0) {
                if(received < 4) {
                    std::cerr << "invalid (too small) packet received\n";
                    return;
                }
                NetworkEvent event;
                serial.deserialize(event);
                if(event == RECTANGLE_UPDATE) {
                    if(received < 28) { // rectangle update size
                        // packet incomplete, get next time!
                        leftOver = received;
                        memcpy(networkData, networkData+MAX_PACKET-leftOver, leftOver);
                        return;
                    }
                    
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
        ;
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
    size_t leftOver = 0; // leftOver is how much from the last packet that applies to a new one (already filled)
    unsigned char networkData[MAX_PACKET]; // max network packet size is now 2048 bytes
};
