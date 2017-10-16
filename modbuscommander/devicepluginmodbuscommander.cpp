/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "devicepluginmodbuscommander.h"
#include "plugininfo.h"


DevicePluginModbusCommander::DevicePluginModbusCommander()
{
}

DeviceManager::HardwareResources DevicePluginModbusCommander::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}


DeviceManager::DeviceSetupStatus DevicePluginModbus::setupDevice(Device *device)
{
    if ((device->deviceClassId() == modbusTCPOutputDeviceClassId) || (device->deviceClassId() == modbusTCPInputDeviceClassId))  {
        ModbusTCPClient *modbus = new ModbusTCPClient(QHostAddress(device->paramValue(ipv4addressParamTypeId).toString()),
                                                      device->paramValue(slaveAddressParamTypeId).toInt(), device->paramValue(portParamTypeId).toInt(), this);
        if (!modbus->valid()) {
            qWarning(dcModbusCommander()) << "Could not create Modbus connection";
            return DeviceManager::DeviceSetupStatusFailure;
        }
        m_modbusSockets.insert(modbus, device);
        device->setStateValue(connectedStateTypeId, true);
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginModbusCommander::postSetupDevice(Device *device)
{
    if ((device->deviceClassId() == modbusTCPOutputDeviceClassId) ||(device->deviceClassId() == modbusTCPInputDeviceClassId)){

    }

}

DeviceManager::DeviceError DevicePluginModbusCommander::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == modbusTCPOutputDeviceClassId) {
        if (action.actionTypeId() == modbusButtonActionTypeId) {

            ModbusTCPClient *modbus = m_modbusSockets.key(device);
            int data = device->stateValue(dataStateTypeId).toInt();
            int address = device->paramValue(registerAddressParamTypeId).toInt();
            modbus->setRegister(address, data);
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == dataActionTypeId) {

            int data = action.param(dataStateParamTypeId).value().toInt();
            device->setStateValue(dataStateTypeId, data);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;

    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginModbusCommander::deviceRemoved(Device *device)
{
    if ((device->deviceClassId() == modbusTCPOutputDeviceClassId)|| (device->deviceClassId() == modbusTCPInputDeviceClassId)) {
        ModbusTCPClient *modbus = m_modbusSockets.key(device);
        m_modbusSockets.remove(m_modbusSockets.key(device));
        modbus->deleteLater();
    }
}
void DevicePluginModbusCommander::guhTimer()
{
    //foreach modbus device check connection status
    //foreach (Device *device, m_httpRequests) {
    //    QNetworkReply *reply = m_httpRequests.key(device);;
    //    networkManagerGet(reply->request());
    //}

}


