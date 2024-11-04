#pragma once

#include <zmq.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <tuple>
#include "GameState.h"

class JServer {
public:
    JServer(int port_req_rep);
    ~JServer();

    // Handles player number requests (REQ-REP communication)
    void handleConnectionRequests(GameState& gameState);

    // Adds a new client with a unique REP socket for each client
    void addClient(GameState& gameState, int clientPort);

    // Handles receiving client input (via the dedicated REP socket per client)
    void handleClientInput(GameState& gameState, zmq::socket_t* client_socket);

    // Returns the number of connected clients
    int getNumClients() const;

private:
    int port_req_rep;
    int num_clients;

    zmq::context_t context;
    zmq::socket_t rep_socket;

    std::vector<std::thread> client_threads;
    std::mutex client_mutex;

    std::unordered_map<std::string, std::tuple<size_t, double, double>> getMovingSprites(GameState& gameState);
    std::string serializeMovingSprites(const std::unordered_map<std::string, std::tuple<size_t, double, double>>& movingSprites);
};
