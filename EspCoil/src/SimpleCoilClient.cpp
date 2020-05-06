#include "SimpleCoilClient.h"
#include "Arduino.h"
#include "SimpleCoilMsg.h"

int headerLength = sizeof(TCPMESSAGE_PREFIX) -1;

const char* TEST_JOIN = "SimpleCoil:06JSON{\"playerID\":1,\"playername\":\"Player\"}";

SimpleCoilClient::SimpleCoilClient(IPAddress ip, uint16_t port, Client &client)
{
    this->_state = SC_DISCONNECTED;
    this->ip = ip;
    this->port = port;
    this->_client = &client;
}

bool SimpleCoilClient::connect()
{
    if (!connected())
    {
        int result = 0;
        result = _client->connect(this->ip, this->port);
        if (result == 1)
        {
            Serial.println("client connected");
            return true;
        }
        return false;
    }

    return true;
}

/** read a byte into result
 */
bool SimpleCoilClient::readByte(uint8_t *result)
{
    uint32_t previousMillis = millis();
    while (!_client->available())
    {
        //yield(); //TODO: not necessary on ESP32? what if Wifi and BLE coexist?
        uint32_t currentMillis = millis();
        if (currentMillis - previousMillis >= ((int32_t)SC_SOCKET_TIMEOUT * 1000))
        {
            return false;
        }
    }
    *result = _client->read(); //read one byte from socket
    Serial.print(".");
    return true;
}

bool SimpleCoilClient::readByte(uint8_t *buf, uint16_t *index)
{
    uint16_t currentIndex = *index;
    uint8_t *writeAddress = &(buf[currentIndex]);
    if (readByte(writeAddress))
    {
        *index = currentIndex + 1;
        return true;
    }
    return false;
}

/** read a packet into the buffer
 */
uint16_t SimpleCoilClient::readMessage(uint8_t *payloadStartIdx)
{

    uint16_t pcktLen = 0;  //total length package incl. 2 leading bytes
    uint8_t character = 0; //current value being read
    uint16_t msgLen = 0;   //length of the message

    //read leading 2 bytes to calculate msg length
    //returning 0 means that we haven't read a full set of 2 leading bytes
    if (!readByte(buffer, &pcktLen))
        return 0;
    if (!readByte(buffer, &pcktLen))
        return 0;

    *payloadStartIdx = pcktLen; //memorize the start of the message content

    msgLen = (buffer[*payloadStartIdx - 2] << 8) + buffer[*payloadStartIdx - 1];

    Serial.println("message length:");
    Serial.println(msgLen);

    //read as many bytes as needed to get the whole message into buffer
    for (uint16_t i = 0; i < msgLen; i++)
    {
        if (!readByte(&character))
            return 0;
        buffer[pcktLen] = character;
        pcktLen++;
    }

    Serial.println("message read...");
    return pcktLen;
}

bool SimpleCoilClient::loop()
{
    if (connected())
    {
        Serial.println("checking for message");
        unsigned long t = millis();
        if (_client->available())
        {
            Serial.println("data available");
            uint8_t payloadStartIdx;

            uint16_t pcktLen = readMessage(&payloadStartIdx);

            //uint8_t *payload; //pointer to the first byte of the payload

            if (pcktLen > 0) //we have read something...
            {

                lastInActivity = t;

                // ACTION: ping-pong
                if (pcktLen - payloadStartIdx == 4)
                {
                    Serial.println("received ping");
                    for (int i = payloadStartIdx; i < pcktLen; i++)
                    {
                        Serial.print((char)buffer[i]);
                    }
                    Serial.println("send pong...");
                    buffer[payloadStartIdx + 1] = 0x6F; //replace i with o
                    _client->write(buffer, pcktLen);
                }
                // ACTION: message received
                else if ((pcktLen - payloadStartIdx) > headerLength)
                {
                    //skip the protocol header
                    payloadStartIdx+=headerLength;

                    if (callback)
                    {
                        //check if we have a json message
                        Serial.println((char *) &buffer[payloadStartIdx]);
                        if (strncmp((char *) &buffer[payloadStartIdx], TCPPREFIX_JSON, sizeof(TCPPREFIX_JSON)-1) == 0)
                        {
                            Serial.println("we got some json");
                            // JSON Stuff...
                            uint8_t jsonStartIdx = payloadStartIdx + sizeof(TCPPREFIX_JSON) -1;
                            uint8_t *json = &buffer[jsonStartIdx];
                            callback(TCPPREFIX_JSON, json, pcktLen - jsonStartIdx);
                            this->_state=SC_JOINED;
                        }
                        else if (strncmp((char *) &buffer[payloadStartIdx], TCPPREFIX_MESG, sizeof(TCPPREFIX_MESG)) == 0)
                        {
                            uint8_t msgStartIdx = payloadStartIdx + sizeof(TCPMESSAGE_PREFIX) + sizeof(TCPPREFIX_MESG);

                            uint16_t ml = pcktLen - msgStartIdx;                         // message length in bytes
                            memmove(buffer + msgStartIdx - 1, buffer + msgStartIdx, ml); // move message inside buffer 1 byte to front
                            buffer[msgStartIdx + ml] = 0;                                // end the message as a 'C' string with \x00
                            char *message = (char *)buffer + msgStartIdx - 1;

                            callback(message, NULL, 0);
                        }
                    }
                }
                // ACTION: JOIN
                if(this->_state<SC_JOINED){
                    Serial.println("trying to join...");
                    uint16_t length = 0;
                    length = writeString(TEST_JOIN,buffer,length);
                    size_t written = write(buffer,length);
                    Serial.print("written ");
                    //Serial.print(written);
                    Serial.print(" bytes\n ");
                }

            }
        }
    }
    return false;
}

uint16_t SimpleCoilClient::writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string; //warum dieses assignment?
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8); // left length byte
    buf[pos-i-1] = (i & 0xFF); // right lenth byte
    return pos;
}

size_t SimpleCoilClient::write(const uint8_t *buffer, size_t size) {
    lastOutActivity = millis();
    return _client->write(buffer,size);
}

bool SimpleCoilClient::connected()
{
    bool rc;
    if (_client == NULL)
    {
        rc = false;
    }
    else
    {
        rc = (int)_client->connected();
        if (!rc)
        {
            if (this->_state == SC_CONNECTED)
            {
                this->_state = SC_CONNECTION_LOST;
                _client->flush();
                _client->stop();
            }
        }
    }
    return rc;
}

int SimpleCoilClient::state() {
    return this->_state;
}

SimpleCoilClient &SimpleCoilClient::setCallback(SC_CALLBACK_SIGNATURE)
{
    this->callback = callback;
    return *this;
}

