#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

class BaseRectangle {
public:
    BaseRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation);
    virtual void update() = 0;
    void draw(sf::RenderWindow &window); 
protected:
    sf::RectangleShape shape;
};

class NetworkRectangle : public BaseRectangle {
public:
    NetworkRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation);
    void update() override;
private:
    // socket info
};

class PhysicsRectangle : public BaseRectangle {
public:
    PhysicsRectangle(b2World &physicsWorld, bool fixed, sf::Vector2f pos, sf::Vector2f size, float rotation);
    void update() override;
private:
    b2Body *physicsBody;
};

