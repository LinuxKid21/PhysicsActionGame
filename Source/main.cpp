#include "Rectangle.h"

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

// evil global variable!
// initialized first thing in main
sf::Font font;

class State {
public:
};

class Client {
public:
};

class Server {
public:
    //---------------------------------------------------------------------------------------------------- 
    Server(sf::RenderWindow &window) : world(b2Vec2(0.0f,10.f)), window(window)
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

            while(timeSinceUpdate >= timeStep) {
                timeSinceUpdate -= timeStep;
                world.Step(timeStep, velocityIterations, positionIterations);
                update();
            }

            window.clear();
            draw();
            window.display();
        }
    }
private:
    //---------------------------------------------------------------------------------------------------- 
    void onStart() {
        // rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(1,0), sf::Vector2f(.5, 1.7), 45, false));
        rectangleEntities.push_back(PhysicsRectangle(world, true, sf::Vector2f(7.5,10), sf::Vector2f(15, 1), 0));

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

    void draw() {
        for(auto &e : rectangleEntities) {
            e.draw(window);
        }
    }

    // handles non-window events
    void handleGameEvent(const sf::Event &event) {
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
            for(int i = 0;i < 25; i++)
                rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.1, .1), 0));
        if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            rectangleEntities.push_back(PhysicsRectangle(world, false, sf::Vector2f(event.mouseButton.x/1920.f*19.2,event.mouseButton.y/1080.f*10.8), sf::Vector2f(.5, 1.7), 45));
    }

    //---------------------------------------------------------------------------------------------------- 
    void makePlatform(sf::Vector2f pos, sf::Vector2f size, float thickness) {
        float legHeight = size.y-thickness;
        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x+size.x/2.f-thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x-size.x/2.f+thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x, pos.y -size.y/2.f+thickness/2.f),
                                                    sf::Vector2f(size.x, thickness), 0));
    }





    //---------------------------------------------------------------------------------------------------- 
    sf::RenderWindow &window;
    b2World world;
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;
    sf::Clock clock;
    float timeSinceUpdate = 0;
    sf::View view = sf::View(sf::FloatRect(0, 0, 19.20, 10.80));

    std::vector<PhysicsRectangle> rectangleEntities;
};



int main()
{
    if(!font.loadFromFile("FreeSans.ttf")) {
        std::cerr << "Failed to load the font file 'FreeSans.ttf'\n";
    }

    sf::RenderWindow window(sf::VideoMode(192*3, 180*3), "THE GAME");
    
    class Button {
    public:
        Button(std::string name, std::function<std::vector<Button>()> func) {
            name = name;
            func = func;
        }
        std::string name;
        // returns the new set of buttons (for the next menu) or empty if no change
        std::function<std::vector<Button>()> func;
    };
    std::vector<Button> buttons;

    buttons.push_back(Button(
        "Play Game",
        [&window]() -> std::vector<Button> {
            std::cout << "Let's play!\n";

            // play the game!
            Server server(window);
            return std::vector<Button>();
        }
    ));

    buttons.push_back(Button(
        "Test Functionality",
        [](){
            std::vector<Button> buttons;
            std::cout << "Clicked!\n";
            buttons.push_back(Button(
                "Hello!",
                [](){
                    std::cout << "Second layer!\n";
                    return std::vector<Button>();
                }
            ));
            return buttons;
        }
    ));

    // main menu
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // event.mouseButton.x
            }
        }


        window.clear();
        int i = 1;
        for(auto &b : buttons) {
            const sf::Vector2f SIZE(400, 100);
            const int MARGIN = 10;
            sf::RectangleShape rectangle(SIZE);
            rectangle.setPosition(sf::Vector2f(10, MARGIN*i + SIZE.y*(i-1)));
            window.draw(rectangle);
            i++;
        }
        window.display();
    }




    return 0;
}

