#pragma once
class ServerGame {
public:
    ServerGame(int32_t id) : world(b2Vec2(0.0f,10.f)), gameID(id)
    {
    }
    
    ~ServerGame() {
        ;
    }
    
    void start() {
        onStart();
        
        // while a player is connected keep the simulation running
        while (socket || socketP2)
        {
            float delta = clock.restart().asSeconds();
            timeSinceUpdate += delta;

            // update the simulation
            while(timeSinceUpdate >= timeStep) {
                timeSinceUpdate -= timeStep;
                world.Step(timeStep, velocityIterations, positionIterations);
            }
            
            // send the data over the network
            update();
        }
    }
    
    sf::TcpSocket *socket = nullptr;
    sf::TcpSocket *socketP2 = nullptr;
    
    int32_t getGameID() const {return gameID;}
private:
    // initialize the starting platforms
    void onStart() {
        // rectangleEntities.push_back(RectangleEntity(world, sf::Vector2f(1,0), sf::Vector2f(.5, 1.7), 45, false));
        rectangleEntities.push_back(PhysicsRectangle(world, true, sf::Vector2f(7.5,10), sf::Vector2f(15, 1), 0, currentRectID));

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
            if(socket)
                e.update(*socket);
            if(socketP2)
                e.update(*socketP2);
        }
        
        bool p1_exited = false;
        bool p2_exited = false;
        if(socket)
            p1_exited = handleInput(*socket);
        if(socketP2)
            p2_exited = handleInput(*socketP2);
            
        if(p1_exited) {
            delete socket;
            socket = nullptr;
        }
        if(p2_exited) {
            delete socketP2;
            socketP2 = nullptr;
        }
    }
    
    // handle the network input
    bool handleInput(sf::TcpSocket &socket) {
        
        socket.setBlocking(false);
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
                
            if(event == CREATE_RECTANGLE) {
                float x;
                float y;
                stream.deserialize(x);
                stream.deserialize(y);
                
                rectangleEntities.push_back(PhysicsRectangle(world, false,
                    sf::Vector2f(x,y), sf::Vector2f(.1, .1), 0, currentRectID));
                
            } else if(event == EXIT_GAME) {
                return true;
            } else {
                std::cout << "unkown type: " << event << "\n";
            }
        }
        return false;
    }

    // construct a "table" (two legs and a top surface) that is centered at pos,
    // fits exactle inside size and where each component is thickness thick
    void makePlatform(sf::Vector2f pos, sf::Vector2f size, float thickness) {
        float legHeight = size.y-thickness;
        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x+size.x/2.f-thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x-size.x/2.f+thickness/2.f, pos.y+thickness/2.f),
                                                    sf::Vector2f(thickness, legHeight), 0, currentRectID));

        rectangleEntities.push_back(PhysicsRectangle(world, false,
                                                    sf::Vector2f(pos.x, pos.y -size.y/2.f+thickness/2.f),
                                                    sf::Vector2f(size.x, thickness), 0, currentRectID));
    }


    
    b2World world;
    
    const float32 timeStep = 1.0f / 60.0f;
    const int32 velocityIterations = 6;
    const int32 positionIterations = 2;
    sf::Clock clock;
    float timeSinceUpdate = 0;

    std::vector<PhysicsRectangle> rectangleEntities;
    int currentRectID = 0;
    int gameID;
};
