#include "RectangleEntity.h"

RectangleEntity::RectangleEntity(b2World &physicsWorld, sf::Vector2f pos, sf::Vector2f size, float rotation, bool fixed) {
    shape = sf::RectangleShape(size);
    shape.setPosition(pos);
    shape.setRotation(rotation);
    shape.setOrigin(size.x/2, size.y/2);

    b2BodyDef bodyDef;
    if(!fixed)
        bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(shape.getPosition().x, shape.getPosition().y);
    bodyDef.angle = rotation*b2_pi/180.f;
    physicsBody = physicsWorld.CreateBody(&bodyDef);

    shape.setFillColor(sf::Color::Green);

    b2PolygonShape physicsShape;
    physicsShape.SetAsBox(size.x/2.f, size.y/2.f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &physicsShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    physicsBody->CreateFixture(&fixtureDef);
}

void RectangleEntity::draw(sf::RenderWindow &window) {
    shape.setPosition(physicsBody->GetPosition().x, physicsBody->GetPosition().y);
    shape.setRotation(physicsBody->GetAngle()*180.f/b2_pi);
    window.draw(shape);
}
