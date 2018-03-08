#pragma once
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <SFML/Network.hpp>
#include <cstdint>
#include <iostream>
#include "NetworkEvents.h"

class BaseRectangle {
public:
    BaseRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation, int id);
    void draw(sf::RenderWindow &window); 
    int32_t id;
protected:
    sf::RectangleShape shape;
};

// gets the attributes of the rectangle from a network connection
class NetworkRectangle : public BaseRectangle {
public:
    NetworkRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation, int id);
    void update(sf::Vector2f pos, sf::Vector2f size, float rotation);
};

// gets the attributes of the rectangle from physics simulation and shares
// that over the passed in socket
class PhysicsRectangle : public BaseRectangle {
public:
    PhysicsRectangle(b2World &physicsWorld, bool fixed, sf::Vector2f pos, sf::Vector2f size, float rotation, int &id);
    void update(sf::TcpSocket &socket);
private:
    b2Body *physicsBody;
    bool firstUpdate = true;
};

