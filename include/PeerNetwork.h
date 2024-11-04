#ifndef PEERNETWORK_H
#define PEERNETWORK_H

#include <string>

class PeerNetwork {
public:
    PeerNetwork(bool isHost);
    ~PeerNetwork();

    void init(int port, const std::string& peerAddress);
    void cleanup();
    void sendMessage(const std::string& message);
    std::string receiveMessage();
    std::string receiveMessageNonBlocking();

private:
    void* context;
    void* socket;
    bool isHost;
};

#endif  // PEERNETWORK_H
