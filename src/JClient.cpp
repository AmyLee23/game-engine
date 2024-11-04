#include "pch.h"
#include "JClient.h"
#include <iostream>
#include <zmq.hpp>
#include <string>
#include <unordered_map>

// Constructor for the JClient class
JClient::JClient(const std::string& server_ip, int port_req_rep)
    : server_ip(server_ip), port_req_rep(port_req_rep),
    context(1), req_socket(context, ZMQ_REQ), action_socket(nullptr) {

    // Connect the REQ-REP socket to request connection
    req_socket.connect("tcp://" + server_ip + ":" + std::to_string(port_req_rep));
}

// Destructor for the JClient class
JClient::~JClient() {
    if (action_socket) {
        delete action_socket;
    }
}

// Function to connect to the server
void JClient::connectToServer() {
    // Send a request to get the player number and port
    zmq::message_t request(5);
    memcpy(request.data(), "READY", 5);
    req_socket.send(request, zmq::send_flags::none);

    // Receive the reply from the server with the player number and port
    zmq::message_t reply;
    req_socket.recv(reply, zmq::recv_flags::none);

    // Parse the reply to get the player number and port
    std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    std::istringstream iss(reply_str);
    std::string player_num, port_str;
    iss >> player_num >> port_str;

    player_name = "Player" + player_num;
    int action_port = std::stoi(port_str);

    std::cout << "Connected to the server as " << player_name << " on port " << action_port << std::endl;

    // Create a new REP socket for sending actions and receiving updated game states
    action_socket = new zmq::socket_t(context, ZMQ_REQ);
    action_socket->connect("tcp://" + server_ip + ":" + std::to_string(action_port));
}

std::unordered_map<std::string, std::tuple<size_t, double, double>> JClient::deserializeMovingSprites(std::string game_state_message) {
    // Map of moving sprites
    std::unordered_map<std::string, std::tuple<size_t, double, double>> movingSprites;

    // Split the game state update by semicolons
    std::istringstream iss(game_state_message);
    std::string sprite_info;

    // Process each sprite's info
    while (std::getline(iss, sprite_info, ';')) {
        if (sprite_info.empty()) continue;  // Skip any empty entries

        std::istringstream sprite_stream(sprite_info);
        std::string name;
        size_t type;
        double x, y;

        sprite_stream >> name >> type >> x >> y;
        if (!name.empty()) {
            movingSprites[name] = std::make_tuple(type, x, y);
        }
    }
    return movingSprites;
}

// Function to send player input (REQ-REP) to the server and wait for updated game state
std::unordered_map<std::string, std::tuple<size_t, double, double>> JClient::sendInput(const std::string& name, size_t type, const std::pair<double, double>& position) {
    if (!action_socket) {
        std::cerr << "Action socket is not initialized!" << std::endl;
        return std::unordered_map<std::string, std::tuple<size_t, double, double>>{};
    }

    // Serialize the player's input (player name and position)
    std::string message = name + " " + std::to_string(type) + " " + std::to_string(position.first) + " " + std::to_string(position.second);

    // Send the serialized message to the server
    zmq::message_t input_msg(message.size());
    memcpy(input_msg.data(), message.c_str(), message.size());
    action_socket->send(input_msg, zmq::send_flags::none);

    //std::cout << "Sent player input to the server: " << message << std::endl;

    // Wait for the server's reply
    zmq::message_t reply_msg;
    action_socket->recv(reply_msg, zmq::recv_flags::none);
    std::string reply(static_cast<char*>(reply_msg.data()), reply_msg.size());

    //std::cout << "Received game state update from the server: " << reply << std::endl;

    // Return reply
    std::unordered_map<std::string, std::tuple<size_t, double, double>> movingSprites = deserializeMovingSprites(reply);
    return movingSprites;
}

// Deleting a client
 void JClient::deleteClient(size_t type) {
    if (!action_socket) {
        std::cerr << "Action socket is not initialized!" << std::endl;
        return;
    }

    // Serialize the player's input (player name and position)
    std::string message = player_name + " " + std::to_string(type) + " " + std::to_string(-1) + " " + std::to_string(-1);

    // Send the serialized message to the server
    zmq::message_t input_msg(message.size());
    memcpy(input_msg.data(), message.c_str(), message.size());
    action_socket->send(input_msg, zmq::send_flags::dontwait);
}

// Getter function to retrieve the player's name
std::string JClient::getPlayerName() const {
    return player_name;
}

