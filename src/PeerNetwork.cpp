#include "pch.h"
#include "PeerNetwork.h"
#include <zmq.h>
#include <stdexcept>
#include <iostream>

// Constructor for PeerNetwork
// Initializes the ZeroMQ context and socket
PeerNetwork::PeerNetwork(bool isHost) : isHost(isHost) {
    // Initialize ZeroMQ context
    context = zmq_ctx_new();
    // If context initialization fails, throw an exception
    if (!context) {
        throw std::runtime_error("Failed to create ZeroMQ context");
    }
    // Initialize a ZMQ_PAIR socket for peer-to-peer communication
    socket = zmq_socket(context, ZMQ_PAIR);
    // If socket creation fails, destroy context and throw an exception
    if (!socket) {
        zmq_ctx_destroy(context);
        throw std::runtime_error("Failed to create ZeroMQ socket");
    }
}
// Initializes the peer-to-peer connection
// Host binds to a local address and waits for connections, client connects to host
void PeerNetwork::init(int port, const std::string& peerAddress) {
    // Set up the bind and connect addresses using the given port and peer address
    std::string bindAddress = "tcp://*:" + std::to_string(port);
    std::string connectAddress = "tcp://" + peerAddress + ":" + std::to_string(port);

    // Return code for zmq operations
    int rc;
    if (isHost) {
        // If this instance is the host, bind the socket to the local address
        rc = zmq_bind(socket, bindAddress.c_str());
        if (rc != 0) {
            std::cerr << "Failed to bind host to " << bindAddress << ": " << zmq_strerror(zmq_errno()) << std::endl;
            throw std::runtime_error("Failed to bind host");
        }
        std::cout << "Host bound to " << bindAddress << std::endl;
    }
    else {
        rc = zmq_connect(socket, connectAddress.c_str());
        if (rc != 0) {
            std::cerr << "Failed to connect to host at " << connectAddress << ": " << zmq_strerror(zmq_errno()) << std::endl;
            throw std::runtime_error("Failed to connect to host");
        }
        std::cout << "Connected to host at " << connectAddress << std::endl;
    }
}

// Sends a message to the connected peer
void PeerNetwork::sendMessage(const std::string& message) {
    int result = zmq_send(socket, message.c_str(), message.size(), 0);
    if (result == -1) {
        std::cerr << "Failed to send message: " << zmq_strerror(zmq_errno()) << std::endl;
    }
}

// Receives a message from the connected peer (blocking)
std::string PeerNetwork::receiveMessage() {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    // Receive the message from the socket (blocking call)
    int result = zmq_msg_recv(&msg, socket, ZMQ_DONTWAIT);
    if (result == -1) {
        zmq_msg_close(&msg);
        return "";  // No message received
    }

    // Extract the received data from the message and store it in a std::string
    std::string receivedData(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
    zmq_msg_close(&msg);
    return receivedData;
}
// Receives a message non-blocking (returns immediately if no message is received)
std::string PeerNetwork::receiveMessageNonBlocking() {
    zmq_msg_t msg;
    zmq_msg_init(&msg);

    // Use zmq_recv with the ZMQ_DONTWAIT flag to make the receive non-blocking
    int result = zmq_msg_recv(&msg, socket, ZMQ_DONTWAIT);
    if (result == -1) {
        zmq_msg_close(&msg);
        return "";  // No message received (non-blocking)
    }

    // Successfully received a message
    std::string receivedData(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
    zmq_msg_close(&msg);
    return receivedData;
}

// Cleans up resources, closes the socket and destroys the context
void PeerNetwork::cleanup() {
    zmq_close(socket);
    zmq_ctx_destroy(context);
}
// Destructor for PeerNetwork, ensures resources are cleaned up
PeerNetwork::~PeerNetwork() {
    cleanup();
}
