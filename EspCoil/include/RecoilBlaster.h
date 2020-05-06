#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H
#include <stdbool.h>
#include <string.h>
#include "BLEDevice.h"
#include "RecoilBle.h"

using namespace std;

// RECOIL main service
static BLEUUID mainServiceUUID("e6f59d10-8230-4a5c-b22f-c062b1d329e3");

// RECOIL characteristics
static BLEUUID charIdUUID("e6f59d11-8230-4a5c-b22f-c062b1d329e3");
static BLEUUID charTelemetryUUID("e6f59d12-8230-4a5c-b22f-c062b1d329e3");
static BLEUUID charCommandUUID("e6f59d13-8230-4a5c-b22f-c062b1d329e3");
static BLEUUID charConfigUUID("e6f59d14-8230-4a5c-b22f-c062b1d329e3");

class RecoilBlaster
{
public:
  static RecoilBlaster& Instance();
  BLERemoteCharacteristic *pIdChar;
  BLERemoteCharacteristic *pTelemetryChar;
  BLERemoteCharacteristic *pCommandChar;
  BLEAdvertisedDevice *pDevice;
  ConnectedWeapon connectedType;
	string connectedUUID;
	string connectedName;
	string connectedID;
  void init();
  void connect();
  void startReload();
  void finishReload();
  void setShotMode();
  void setRecoil(bool enabled);
  
private:
  bool doConnect;
  bool isConnected;
  bool doScan;
  uint8_t lastTriggerCount;
  BLEServer *pServer;
  uint8_t commandID;
  BleChrCmd cmd = {};

  friend class RecoilClientCallback;
  friend class RecoilAdvertisedDeviceCallbacks;

  static void handleTelemetry(
      BLERemoteCharacteristic *pBLERemoteCharacteristic,
      uint8_t *pData,
      size_t length,
      bool isNotify);

  bool connectDevice();
  void sendCommand();
  void sendConfig();
};

class RecoilClientCallback : public BLEClientCallbacks
{
private:
  RecoilBlaster *pBlaster;

public:
  RecoilClientCallback(RecoilBlaster &blaster);
  void onConnect(BLEClient *pclient);
  void onDisconnect(BLEClient *pclient);
};

class RecoilAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
private:
  RecoilBlaster *pBlaster;

public:
  RecoilAdvertisedDeviceCallbacks(RecoilBlaster &blaster);
  void onResult(BLEAdvertisedDevice advertisedDevice);
};

#endif