#pragma once
class ServerLobby {
public:
    ServerLobby(int32_t gameID) : gameID(gameID) {
        ;
    }
    
    void start() {
        bool ready1 = false;
        bool ready2 = false;
        while(!ready1 || !ready2) {
            run(*socketP1, socketP2, ready1);
            if(socketP2)
                run(*socketP2, socketP1, ready2);
        }
        
        
        NetworkEvent e = START_GAME;
        socketP1->send((char *)&e, sizeof(e));
        socketP2->send((char *)&e, sizeof(e));
        
        ServerGame game(gameID);
        game.socket = socketP1;
        game.socketP2 = socketP2;
        game.start();
    }
    
    void run(sf::TcpSocket &socket, sf::TcpSocket *otherSocket, bool &ready) {
        socket.setBlocking(false);
        ReadStream stream(socket);
        while(!stream.isDone()) {
            NetworkEvent event;
            
            if(!stream.deserialize(event))
                break;
                
            if(event == READY_GAME) {
                ready = true;
            } else if(event == CHAT_LOBBY) {
                std::string str;
                stream.deserialize(str);
                
                if(otherSocket != nullptr) {
                    socket.setBlocking(true);
                    unsigned char data[1024];
                    Serial serial(data, 1024);
                    serial.serialize(CHAT_LOBBY);
                    serial.serialize(str);
                    // echo to the other player
                    otherSocket->send(data, serial.getOffset());
                    socket.setBlocking(false);
                }
            } else {
                std::cout << "unkown type: " << event << "\n";
            }
        }
        
        
        socket.setBlocking(true);
    }
    
    sf::TcpSocket *socketP1 = nullptr;
    sf::TcpSocket *socketP2 = nullptr;
    
    std::string nameP1;
    std::string nameP2;
    
    int32_t getGameID() const {return gameID;}
    
    bool lobbyFull = false;
private:
    int32_t gameID;
};