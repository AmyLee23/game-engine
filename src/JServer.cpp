#include "pch.h"
#include "JServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include "PhysicsEngine.h"
#include "GameWorld.h"
#include "GameTypes.h"

// Constructor for the JServer class
JServer::JServer(int port_req_rep)
    : port_req_rep(port_req_rep), num_clients(0), context(1), rep_socket(context, ZMQ_REP) {

    // Bind the REQ-REP socket for client player number requests
    rep_socket.bind("tcp://*:" + std::to_string(port_req_rep));
}

// Destructor for the JServer class
JServer::~JServer() {
    for (auto& th : client_threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

// Handles player number requests (REQ-REP communication)
void JServer::handleConnectionRequests(GameState& gameState) {
    zmq::message_t request_msg;
    if (rep_socket.recv(request_msg, zmq::recv_flags::dontwait)) {
        std::string request(static_cast<char*>(request_msg.data()), request_msg.size());
        std::cout << "Received request: " << request << std::endl;

        // Respond with the current number of clients + port for this client
        int playerNumber = num_clients + 1;
        int clientPort = port_req_rep + playerNumber;  // Unique port per client

        std::string reply = std::to_string(playerNumber) + " " + std::to_string(clientPort);
        zmq::message_t reply_msg(reply.size());
        memcpy(reply_msg.data(), reply.c_str(), reply.size());

        // Send the reply back to the client
        rep_socket.send(reply_msg, zmq::send_flags::none);
        std::cout << "Sent player number and port: " << reply << std::endl;

        // Add the client once the reply is sent
        addClient(gameState, clientPort);
    }
}

// Adds a new client and updates the client count
void JServer::addClient(GameState& gameState, int clientPort) {
    std::lock_guard<std::mutex> lock(client_mutex);
    num_clients++;

    // Create a new REP socket for this client
    zmq::socket_t* client_socket = new zmq::socket_t(context, ZMQ_REP);
    client_socket->bind("tcp://*:" + std::to_string(clientPort));

    std::cout << "Client " << num_clients << " connected on port " << clientPort << std::endl;

    // Spawn a thread to handle this client's input with the unique socket
    client_threads.push_back(std::thread(&JServer::handleClientInput, this, std::ref(gameState), client_socket));
}

// Handles receiving client input (via the dedicated REP socket per client)
void JServer::handleClientInput(GameState& gameState, zmq::socket_t* client_socket) {
    while (true) {
        zmq::message_t request;

        // Receive the action request from the client
        if (client_socket->recv(request, zmq::recv_flags::none)) {
            std::string client_input(static_cast<char*>(request.data()), request.size());
            //std::cout << "Received input: " << client_input << std::endl;

            // Deserialize the input (expected format: "name positionx positiony")
            std::istringstream iss(client_input);
            std::string sprite_name;
            size_t type;
            double position_x, position_y;
            iss >> sprite_name >> type >> position_x >> position_y;

            // Process the input
            auto* sprite = gameState.getGameWorld().getObjectByName(sprite_name);
            if (sprite) {
                double last_x = sprite->getPosition()->getX();
                double last_y = sprite->getPosition()->getY();
                sprite->getPosition()->setX(position_x);
                sprite->getPosition()->setY(position_y);

                // Check for collisions using the PhysicsEngine
                auto& physicsEngine = gameState.getPhysicsEngine();
                if (physicsEngine.checkCollisions(*sprite, gameState)) {
                    // Revert the position if collision detected
                    sprite->getPosition()->setX(last_x);
                    sprite->getPosition()->setY(last_y);
                }
            }

            // Get game state message
            std::string game_state_message = serializeMovingSprites(getMovingSprites(gameState));

            if (position_x == -1.0 && position_y == -1.0) {
                gameState.getGameWorld().removeObject(sprite_name);
            }

            // Send back updated game state
            zmq::message_t reply_msg(game_state_message.size());
            memcpy(reply_msg.data(), game_state_message.c_str(), game_state_message.size());
            client_socket->send(reply_msg, zmq::send_flags::none);
        }
    }
}

// Helper function for making map of sprites that move
std::unordered_map<std::string, std::tuple<size_t, double, double>> JServer::getMovingSprites(GameState& gameState) {
    GameTypes types = gameState.getGameTypes();
    std::unordered_map<std::string, std::tuple<size_t, double, double>> movingSprites;
    for (const auto& object_pair : gameState.getGameWorld().getObjects()) {
        NamedSprite* object = object_pair.second;
        const std::string& name = object_pair.first;
        /*if ((name.find("platform") != std::string::npos || name.find("Player") != std::string::npos)) {
            movingSprites[name] = { object->getPosition()->getX(), object->getPosition()->getY() };
        }*/
        size_t object_type = object->getTypeHash();
        if (object_type == types.getTypeHash("player") || object_type == types.getTypeHash("platform")) {
            movingSprites[name] = std::make_tuple(object_type, object->getPosition()->getX(), object->getPosition()->getY());
        }
    }
    return movingSprites;
}

// Helper function for serializing a map
std::string JServer::serializeMovingSprites(const std::unordered_map<std::string, std::tuple<size_t, double, double>>& movingSprites) {
    std::string game_state_message;
    for (const auto& sprite : movingSprites) {
        /*const std::string& name = sprite.first;
        const std::pair<double, double>& position = sprite.second;
        game_state_message += name + " " + std::to_string(position.first) + " " + std::to_string(position.second) + ";";*/
        const std::string& name = sprite.first;
        const std::tuple<size_t, double, double>& data = sprite.second;
        size_t type = std::get<0>(data);
        double x = std::get<1>(data);
        double y = std::get<2>(data);

        // Serialize name, type, and position
        game_state_message += name + " " + std::to_string(type) + " " + std::to_string(x) + " " + std::to_string(y) + ";";
    }
    return game_state_message;
}

// Returns the number of connected clients
int JServer::getNumClients() const {
    return num_clients;
}