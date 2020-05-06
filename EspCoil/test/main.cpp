#include <iostream>
#include <stdio.h>
#include <cstring>
#include "RecoilBle.h"

int main()
{
    BleChrCmd cmd = {};
    //cmd.CmdCounter = 0x11;
    cmd.Command = ACT_REBOOT_DFU;
    //cmd.WeaponAmmo = 10;

    //print the command for debugging
    unsigned char *buffer = (unsigned char *)malloc(sizeof(cmd));
    memcpy(buffer, (const unsigned char *)&cmd, sizeof(cmd));
    printf("Command Bytes:\n");
    for (int i = 0; i < sizeof(cmd); i++)
        printf("%02X ", buffer[i]);
    printf("\n");
    //free(buffer);

    return 0;
}
