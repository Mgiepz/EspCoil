/*
 A minimalistic client for SimpleCoil in dedicated server mode.
 inspired by https://github.com/knolleary/pubsubclient
*/

#ifndef SIMPLE_COIL_CLIENT_H
#define SIMPLE_COIL_CLIENT_H

#include <Arduino.h>
#include "Client.h"
#include <functional>
#include <string.h>

// SC_MAX_MESSAGE_SIZE : Maximum size for SimpleCoil Messages
#ifndef SC_MAX_MESSAGE_SIZE
#define SC_MAX_MESSAGE_SIZE 512 //? welcher default?
#endif

// SC_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef SC_SOCKET_TIMEOUT
#define SC_SOCKET_TIMEOUT 15
#endif

// Possible values for client.state() //better use enum?
#define SC_CONNECTION_TIMEOUT     -4
#define SC_CONNECTION_LOST        -3
#define SC_CONNECT_FAILED         -2
#define SC_DISCONNECTED           -1
#define SC_CONNECTED               0
#define SC_JOINED                  1

#define SC_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback

class SimpleCoilClient
{
private:
    Client *_client;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    IPAddress ip;
    uint16_t port;
    uint8_t buffer[SC_MAX_MESSAGE_SIZE];
    SC_CALLBACK_SIGNATURE;
    int _state;
    uint16_t readMessage(uint8_t*);
    bool readByte(uint8_t * result);
    bool readByte(uint8_t * result, uint16_t * index);
    uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);
public:
    SimpleCoilClient(IPAddress ip, uint16_t port, Client& client);
    size_t write(const uint8_t *buffer, size_t size);
    SimpleCoilClient& setCallback(SC_CALLBACK_SIGNATURE);
    bool connect();
    void disconnect();
    bool loop();
    bool connected();
    int state();
};

#endif