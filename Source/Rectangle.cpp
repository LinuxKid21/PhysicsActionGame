#include "Rectangle.h"

BaseRectangle::BaseRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation) {
    shape = sf::RectangleShape(size);
    shape.setPosition(pos);
    shape.setRotation(rotation);
    shape.setOrigin(size.x/2, size.y/2);

    shape.setFillColor(sf::Color::Green);
}

void BaseRectangle::draw(sf::RenderWindow &window) {
    window.draw(shape);
}


NetworkRectangle::NetworkRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation) : BaseRectangle(pos, size, rotation) {
    ;
}
void NetworkRectangle::update() {
    ;
}


PhysicsRectangle::PhysicsRectangle(b2World &physicsWorld, bool fixed, sf::Vector2f pos, sf::Vector2f size, float rotation) : BaseRectangle(pos, size, rotation) {
    b2BodyDef bodyDef;
    if(!fixed)
        bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(shape.getPosition().x, shape.getPosition().y);
    bodyDef.angle = rotation*b2_pi/180.f;
    physicsBody = physicsWorld.CreateBody(&bodyDef);


    b2PolygonShape physicsShape;
    physicsShape.SetAsBox(size.x/2.f, size.y/2.f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &physicsShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    physicsBody->CreateFixture(&fixtureDef);
}
void PhysicsRectangle::update() {
    shape.setPosition(physicsBody->GetPosition().x, physicsBody->GetPosition().y);
    shape.setRotation(physicsBody->GetAngle()*180.f/b2_pi);
}









