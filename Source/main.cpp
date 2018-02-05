#include "RectangleEntity.h"

#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>


int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    b2World world(b2Vec2(0.0f,10.f));
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;

    std::vector<RectangleEntity> rectangleEntities;
    // rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(1,0), sf::Vector2f(.5, 1.7), 45, false));
    rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(7.5,10), sf::Vector2f(15, 1), 0, true));

    sf::Clock clock;
    float timeSinceUpdate = 0;
    sf::View view(sf::FloatRect(0, 0, 19.20, 10.80));
    window.setView(view);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
                for(int i = 0;i < 25; i++)
                    rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.1, .1), 0, false));
            if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.5, 1.7), 45, false));
        }

        float delta = clock.restart().asSeconds();
        timeSinceUpdate += delta;
        while(timeSinceUpdate >= timeStep) {
            timeSinceUpdate -= timeStep;
            world.Step(timeStep, velocityIterations, positionIterations);
        }
            
        window.clear();
        for(auto &e : rectangleEntities) {
            e.draw(window);
        }
        window.display();
    }

    return 0;
}

