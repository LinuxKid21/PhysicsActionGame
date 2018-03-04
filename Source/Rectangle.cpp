#include "Rectangle.h"
#include "Serial.h"

BaseRectangle::BaseRectangle(sf::Vector2f pos, sf::Vector2f size, float rotation, int _id) {
    shape = sf::RectangleShape(size);
    shape.setPosition(pos);
    shape.setRotation(rotation);
    shape.setOrigin(size.x/2, size.y/2);

    shape.setFillColor(sf::Color::Green);
    id = _id;
}

void BaseRectangle::draw(sf::RenderWindow &window) {
    window.draw(shape);
}




NetworkRectangle::NetworkRectangle(sf::Vector2f pos, sf::Vector2f size,
    float rotation, int id) : BaseRectangle(pos, size, rotation, id) {}

void NetworkRectangle::update(sf::Vector2f pos, sf::Vector2f size, float rotation) {
    shape.setPosition(pos);
    shape.setRotation(rotation);
    shape.setSize(size);
    shape.setOrigin(size.x/2, size.y/2);
}




PhysicsRectangle::PhysicsRectangle(b2World &physicsWorld, bool fixed,
    sf::Vector2f pos, sf::Vector2f size, float rotation,
    sf::TcpSocket &socket, int &_id)
    : BaseRectangle(pos, size, rotation, _id), socket(socket)
{
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
    _id++;
}
void PhysicsRectangle::update() {
    shape.setPosition(physicsBody->GetPosition().x, physicsBody->GetPosition().y);
    shape.setRotation(physicsBody->GetAngle()*180.f/b2_pi);
    
    
    //std::cout << "ID: " << id << "\n";
    const int size = 4 /* event size */ + 4 /* id size */ + 4*5 /* 5 float size */;
    unsigned char data[size];
    
    Serial serial(data, size);
    
    serial.serialize(RECTANGLE_UPDATE);
    serial.serialize(id);
    serial.serialize(shape.getPosition().x);
    serial.serialize(shape.getPosition().y);
    serial.serialize(shape.getSize().x);
    serial.serialize(shape.getSize().y);
    serial.serialize(shape.getRotation());
    
    if (socket.send(data, size) != sf::Socket::Done)
    {
        std::cerr << "unable to send data!\n";
    }
}









