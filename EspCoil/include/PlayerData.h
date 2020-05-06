#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H
#include <string>
#include <stdint.h>

using namespace std;

class PlayerData {
    public:
        static PlayerData& Instance();
        string playerName;
        int hitPoints;
        int hitPointsMax;
        int lives;
        int livesMax;
        float shields;
        float shieldsMax;
};

#endif