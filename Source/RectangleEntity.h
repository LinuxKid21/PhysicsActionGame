#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

class RectangleEntity {
public:
    RectangleEntity(b2World &physicsWorld, sf::Vector2f pos, sf::Vector2f size, float rotation, bool fixed);

    void draw(sf::RenderWindow &window); 
private:
    b2Body* physicsBody;
    sf::RectangleShape shape;
};

