#pragma once
#include "Rectangle.h"

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
        
    }

    void update() {
        unsigned short port;
        sf::IpAddress sender;
        std::size_t received;
        while (socket.receive(networkData, 100, received, sender, port) == sf::Socket::Done)
        {
            std::cout << "Received " << received << " bytes from " << sender << " on port " << port << std::endl;
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

    std::vector<PhysicsRectangle> rectangleEntities;
    
    sf::UdpSocket socket;
    unsigned char networkData[2048]; // max network packet size is now 2048 bytes
};
