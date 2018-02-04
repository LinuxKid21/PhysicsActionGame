#include "RectangleEntity.h"

RectangleEntity::RectangleEntity(b2World &physicsWorld) {
    shape = sf::RectangleShape(sf::Vector2f(.5, 1.7));

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 0.0f);
    physicsBody = physicsWorld.CreateBody(&bodyDef);

    shape.setFillColor(sf::Color::Green);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    physicsBody->CreateFixture(&fixtureDef);
}

void RectangleEntity::draw(sf::RenderWindow &window) {
    shape.setPosition(physicsBody->GetPosition().x, physicsBody->GetPosition().y);
    window.draw(shape);
}
