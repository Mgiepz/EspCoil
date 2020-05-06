#include "RecoilBlaster.h"
#include "BLEDevice.h"
#include <Arduino.h>
#include <functional>

RecoilClientCallback::RecoilClientCallback(RecoilBlaster &blaster)
{
    this->pBlaster = &blaster;
}

void RecoilClientCallback::onConnect(BLEClient *pclient)
{
    // do nothing?
}

void RecoilClientCallback::onDisconnect(BLEClient *pclient)
{
    Serial.println("onDisconnect");
    this->pBlaster->isConnected = false;
    this->pBlaster->connectedType = CW_NONE;
    this->pBlaster->connectedUUID = nullptr;
}

RecoilAdvertisedDeviceCallbacks::RecoilAdvertisedDeviceCallbacks(RecoilBlaster &blaster)
{
    this->pBlaster = &blaster;
}

/**
* Scan for BLE servers and find the first one that advertises the service we are looking for.
* Called for each advertising BLE server.
*/
void RecoilAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.getName().rfind("SRG1", 0) == 0)
    {
        BLEDevice::getScan()->stop();
        this->pBlaster->pDevice = new BLEAdvertisedDevice(advertisedDevice);
        this->pBlaster->doConnect = true;
        this->pBlaster->doScan = true;
    } // Found our server
}

RecoilBlaster &RecoilBlaster::Instance()
{
    static RecoilBlaster singleton;
    return singleton;
};

void RecoilBlaster::init()
{
    doConnect = false;
    isConnected = false;
    doScan = false;

    Serial.println("Starting RECOIL BLE Client application...");
    BLEDevice::init("RecoilBlaster");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new RecoilAdvertisedDeviceCallbacks(*this));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);

    cmd = {};

    lastTriggerCount = 0;
    commandID = 0x00;
}

void RecoilBlaster::connect()
{
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true)
    {
        if (connectDevice())
        {
            Serial.println("connected to Recoil Blaster");
        }
        else
        {
            Serial.println("failed to connect to Recoil Blaster... nothing more will be done!");
        }
        doConnect = false;
    }

    // If we are connected to a peer BLE Server...
    if (isConnected)
    {
        // do nothing so far...
    }
    else if (doScan)
    {
        BLEDevice::getScan()->start(0);
    }
}

void RecoilBlaster::handleTelemetry(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{

    BleChrTelemetry *data = (BleChrTelemetry *)pData;

    uint8_t triggerCount = data->Pressed[0] & 0x0F;
    uint8_t reloadCount = data->Pressed[0] & 0xF0;
    uint8_t shotsRemaining = data->WeaponAmmo;
    uint8_t gunId = data->GunID;

    if (triggerCount != RecoilBlaster::Instance().lastTriggerCount)
    {
        RecoilBlaster::Instance().lastTriggerCount = triggerCount;
        Serial.println("triggerPulled Remaining AMO:" + data->WeaponAmmo);
    }
}

bool RecoilBlaster::connectDevice()
{
    Serial.print("Forming a connection to ");
    Serial.println(pDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println("... created client");

    pClient->setClientCallbacks(new RecoilClientCallback(*this));

    // Connect to Blaster.
    pClient->connect(pDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println("... connected to blaster");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(mainServiceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(mainServiceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println("... found a blaster");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pIdChar = pRemoteService->getCharacteristic(charIdUUID);
    if (pIdChar == nullptr)
    {
        Serial.print("Failed to find ID characteristic UUID: ");
        Serial.println(charIdUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println("... found ID characteristic");

    // Read the value of the characteristic.
    if (pIdChar->canRead())
    {
        connectedID = pIdChar->readValue();
        Serial.print("connectedID=");
        Serial.println(connectedID.c_str());
    }

    pTelemetryChar = pRemoteService->getCharacteristic(charTelemetryUUID);

    if (pTelemetryChar->canNotify())
    {
        pTelemetryChar->registerForNotify(handleTelemetry);
    }

    pCommandChar = pRemoteService->getCharacteristic(charCommandUUID);

    isConnected = true;
    return true;
}

void RecoilBlaster::sendCommand()
{
    if (pCommandChar->canWrite())
    {
        Serial.print("sending command...");
        cmd.CmdId = commandID;
        commandID += COMMAND_ID_INCREMENT;
        cmd.GunID = 0x01; //this is the player id
        pCommandChar->writeValue((uint8_t*)&cmd, sizeof(BleChrCmd), true);
    }
}

void RecoilBlaster::startReload()
{
    cmd.Command = ACT_RELOADON;
    sendCommand();
}

void RecoilBlaster::finishReload()
{
    cmd.Command = ACT_RELOADOFF;
    cmd.WeaponType = 0x00;// Weapon Profile (we always use the first one and replace just that)
    cmd.WeaponAmmo = 0x1e;//=30
    sendCommand();
}
