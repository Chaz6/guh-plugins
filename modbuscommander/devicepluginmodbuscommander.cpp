/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Bernhard Trinnes <bernhard.trinnes@guh.io           *
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


DeviceManager::DeviceSetupStatus DevicePluginModbusCommander::setupDevice(Device *device)
{
    if ((device->deviceClassId() == modbusTCPWriteDeviceClassId) || (device->deviceClassId() == modbusTCPReadDeviceClassId))  {

        QHostAddress ipAddress = QHostAddress(device->paramValue(ipv4addressParamTypeId).toString());
        int slaveAddress = device->paramValue(slaveAddressParamTypeId).toInt();
        int port = device->paramValue(portParamTypeId).toInt();
        ModbusTCPClient *modbus = new ModbusTCPClient(ipAddress, slaveAddress, port, this);
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


DeviceManager::DeviceError DevicePluginModbusCommander::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == modbusTCPWriteDeviceClassId) {

        if (action.actionTypeId() == writeDataActionTypeId) {

            ModbusTCPClient *modbus = m_modbusSockets.key(device);
            int address = device->paramValue(registerAddressParamTypeId).toInt();

            if (device->paramValue(registerTypeParamTypeId).toString() == "coil") {
                bool data = action.param(dataParamTypeId).value().toBool();
                modbus->setCoil(address, data);

            } else if (device->paramValue(registerTypeParamTypeId).toString() == "register") {
                int data = action.param(dataParamTypeId).value().toInt();
                modbus->setRegister(address, data);
            }
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginModbusCommander::deviceRemoved(Device *device)
{
    if ((device->deviceClassId() == modbusTCPWriteDeviceClassId)|| (device->deviceClassId() == modbusTCPReadDeviceClassId)) {
        ModbusTCPClient *modbus = m_modbusSockets.key(device);
        m_modbusSockets.remove(m_modbusSockets.key(device));
        modbus->deleteLater();
    }
}

void DevicePluginModbusCommander::guhTimer()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == modbusTCPReadDeviceClassId) {
            ModbusTCPClient *modbus = m_modbusSockets.key(device);
            int reg = device->paramValue(registerAddressParamTypeId).toInt();

            if (device->paramValue(registerTypeParamTypeId) == "coil"){
                modbus->getCoil(reg);
            } else if (device->paramValue(registerTypeParamTypeId) == "register") {
                modbus->getRegister(reg);
            }
        }
    }
}
