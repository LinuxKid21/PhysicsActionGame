#include "RectangleEntity.h"

#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>


int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");

    const b2Vec2 gravity(0.0f,10.f);
    b2World world(gravity);
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;

    RectangleEntity e(world);

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
        }

        float delta = clock.restart().asSeconds();
        timeSinceUpdate += delta;
        while(timeSinceUpdate >= timeStep) {
            timeSinceUpdate -= timeStep;
            world.Step(timeStep, velocityIterations, positionIterations);
        }
            
        window.clear();
        e.draw(window);
        window.display();
    }

    return 0;
}

