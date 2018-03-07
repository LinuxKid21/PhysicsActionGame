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
    Client(sf::TcpSocket &socket) : window(sf::VideoMode(192*3, 180*3), "THE GAME"), socket(socket)
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
    
    ~Client() {
        delete[] networkData;
    }

private:
    void onStart() {
        socket.setBlocking(false);
    }

    void update() {
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
                
            if(event == RECTANGLE_UPDATE) {
                int32_t id;
                sf::Vector2f pos;
                sf::Vector2f size;
                float rotation;
                stream.deserialize(id);
                stream.deserialize(pos.x);
                stream.deserialize(pos.y);
                stream.deserialize(size.x);
                stream.deserialize(size.y);
                stream.deserialize(rotation);
                
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
            } else {
                std::cout << "unkown type: " << event << "\n";
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
        
        for(int i = 0;i < 25; i++)
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
    
    //---------------------------------------------------------------------------------------------------- 
    sf::RenderWindow window;
    
    const float32 timeStep = 1.0f / 60.0f;
    sf::Clock clock;
    float timeSinceUpdate = 0;
    sf::View view = sf::View(sf::FloatRect(0, 0, 19.20, 10.80));

    std::vector<NetworkRectangle> rectangleEntities;
    
    sf::TcpSocket &socket;
    
    constexpr static size_t MAX_PACKET = 1024*1024*10; // arbitray value - 10 MB
    unsigned char *networkData = new unsigned char[MAX_PACKET]; // max network packet size is now 2048 bytes
};
