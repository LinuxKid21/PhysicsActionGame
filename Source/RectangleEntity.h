#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>

class RectangleEntity {
public:
    RectangleEntity(b2World &physicsWorld);

    void draw(sf::RenderWindow &window); 
private:
    b2Body* physicsBody;
    sf::RectangleShape shape;
};

