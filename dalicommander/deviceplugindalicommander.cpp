/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Bernhard Trinnes <bernhard.trinnes@guh.io>          *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "deviceplugindalicommander.h"
#include "plugininfo.h"


DevicePluginDaliCommander::DevicePluginDaliCommander()
{
}

DeviceManager::HardwareResources DevicePluginDaliCommander::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}


DeviceManager::DeviceSetupStatus DevicePluginDaliCommander::setupDevice(Device *device)
{
    if (device->deviceClassId() == daliOutputDeviceClassId) {
        if (!m_tcpSocket->isValid()){

            QHostAddress ipAddress = QHostAddress(device->paramValue(ipv4addressParamTypeId).toString());
            int port = device->paramValue(portParamTypeId).toInt();
            m_tcpSocket = new QTcpSocket(this);
            m_tcpSocket->connectToHost(ipAddress, port);
            connect(m_tcpSocket, &QTcpSocket::connected, this, &DevicePluginDaliCommander::onTcpSocketConnected);
            connect(m_tcpSocket, &QTcpSocket::disconnected, this, &DevicePluginDaliCommander::onTcpSocketDisconnected);
            connect(m_tcpSocket, &QTcpSocket::bytesWritten, this, &DevicePluginDaliCommander::onTcpSocketBytesWritten);
            connect(m_tcpSocket, &QTcpSocket::readyRead, this, &DevicePluginDaliCommander::onTcpSocketReadyRead);
            return DeviceManager::DeviceSetupStatusAsync;
        }
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}


DeviceManager::DeviceError DevicePluginDaliCommander::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == daliOutputDeviceClassId) {

        if (action.actionTypeId() == outputDataActionTypeId) {

            QByteArray data;
            data.append((char)0x02);
            data.append((char)0x00);
            data.append(getAddressByte(device));
            data.append((uint8_t)device->paramValue(outputDataAreaParamTypeId).toInt());
            m_tcpSocket->write(data);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginDaliCommander::deviceRemoved(Device *device)
{
    Q_UNUSED(device);
    if(myDevices().isEmpty()){
        m_tcpSocket->deleteLater();
    }
}

uint8_t DevicePluginDaliCommander::getAddressByte(Device *device)
{
    uint8_t inputAddress = device->paramValue(addressParamTypeId).toInt();
    uint8_t outputAddress;
    uint8_t requestType;

    if (device->paramValue(addressTypeParamTypeId).toString() == "command") {
        requestType = 1;
    } else if (device->paramValue(addressTypeParamTypeId).toString() == "parameter") {
        requestType = 0;
    }

    if (device->paramValue(addressTypeParamTypeId).toString() == "short") {
        outputAddress = ((inputAddress << 1) | requestType) & 0x7F ;                //0AAAAAAS (AAAAAA = 0 to 63, S = 0/1)
    } else if (device->paramValue(addressTypeParamTypeId).toString() == "group") {
        outputAddress = (((inputAddress << 1) & 0x1E ) | requestType) | 0x80 ;      //100AAAAS (AAAA = 0 to 15, S = 0/1)
    }else if (device->paramValue(addressTypeParamTypeId).toString() == "broadcast") {
        outputAddress = 0xFF;                                                       //0AAAAAAS (AAAAAA = 0 to 63, S = 0/1)
    }else if (device->paramValue(addressTypeParamTypeId).toString() == "special") {
        int command = 0x00;
        outputAddress = 0xA1 | (command & 0x1E);                                    //101CCCC1 (CCCC = command number)
    }

    // TODO add all special commans
    /*
     * A1 00 All special mode processes shall be terminated —
     * A3 XX Store value XX in the DTR —
     * A5 XX Initialize addressing commands for slaves with address XX —
     * A7 00 Generate a new random address —
     * A9 00 Compare the random address with the search address —
     * AB 00 Withdraw from the compare process —
     * B1 HH Store value HH as the high bits of the search address —
     * B3 MM Store value MM as the middle bits of the search address —
     * B5 LL Store value LL as the lower bits of the search address —
     * B7 XX Program the selected slave with short address XX —
     * B9 XX Check if the selected slave has short address XX YES/NO
     * BB 00 The selected slave returns its short address XX XX
     * BD 00 Go into physical selection mode
     */

    return outputAddress;
}


void DevicePluginDaliCommander::onTcpSocketConnected()
{
    foreach(Device *device, myDevices()) {
        if (!device->setupComplete()) {
            qDebug(dcDaliCommander()) << device->name() << "Setup finished" ;
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
        }
        device->setStateValue(connectedStateTypeId, true);
    }
}


void DevicePluginDaliCommander::onTcpSocketDisconnected()
{
    foreach(Device *device, myDevices()) {
        device->setStateValue(connectedStateTypeId, false);
    }
}


void DevicePluginDaliCommander::onTcpSocketBytesWritten()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(sender());
    tcpSocket->close();
}

void DevicePluginDaliCommander::onTcpSocketReadyRead()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(sender());
    QByteArray data = tcpSocket->readAll();
    int status = data.at(2);
    /*
    0x00:   Transfer successful, no response
    0x01:   Transfer successful, response received
    0x02:   Broadcast message received
    0xff: Transfer error
    */

    qDebug(dcDaliCommander()) << "Dali Message received" << data;
    foreach(Device *device, myDevices()) {
        if ((device->deviceClassId() == daliOutputDeviceClassId) || (status == 0x01)) {
            device->setStateValue(responseStateTypeId, data.at(3));
        } else if ((device->deviceClassId() == daliInputDeviceClassId) || (status == 0x02)) {
            device->setStateValue(inputDataStateTypeId, data.at(3));
        } else if (status == 0xff){
            qDebug(dcDaliCommander()) << "Dali Server Error";
        }
    }
}
