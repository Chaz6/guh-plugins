/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Bernhard Trinnes  <bernhard.trinnes@guh.guru>       *
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

#include "devicepluginserialport.h"
#include "plugininfo.h"

DevicePluginSerialPort::DevicePluginSerialPort()
{

}

DeviceManager::HardwareResources DevicePluginSerialPort::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}


DeviceManager::DeviceSetupStatus DevicePluginSerialPort::setupDevice(Device *device)
{

    if ((device->deviceClassId() == serialPortButtonDeviceClassId) || (device->deviceClassId() == serialPortButtonDeviceClassId)) {

        QString interface = device->paramValue(serialPortParamTypeId).toString();
        QSerialPort *serialPort = new QSerialPort(interface, this);
        if(!serialPort)
            return DeviceManager::DeviceSetupStatusFailure;

        serialPort->setBaudRate(QSerialPort::Baud1200);
        serialPort->setDataBits(QSerialPort::DataBits::Data8);
        serialPort->setParity(QSerialPort::Parity::NoParity);
        serialPort->setStopBits(QSerialPort::StopBits::TwoStop);

        if (!serialPort->open(QIODevice::ReadWrite)) {
            qCWarning(dcTools()) << "Could not open serial port" << interface << serialPort->errorString();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        qCDebug(dcTools()) << "Setup successfully serial port" << interface;
        connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialError(QSerialPort::SerialPortError)));
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}


DeviceManager::DeviceError DevicePluginSerialPort::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Create the list of available serial interfaces
    QList<DeviceDescriptor> deviceDescriptors;

    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        qCDebug(dcTools()) << "Found Serial interface:" << port.systemLocation() << port.portName();
        QString description = port.manufacturer() + " | " + port.description();
        DeviceDescriptor descriptor(deviceClassId, port.systemLocation(), description);
        ParamList parameters;
        parameters.append(Param(serialPortParamTypeId, port.systemLocation()));
        descriptor.setParams(parameters);
        deviceDescriptors.append(descriptor);
    }
    emit devicesDiscovered(deviceClassId, deviceDescriptors);
    return DeviceManager::DeviceErrorAsync;
}


DeviceManager::DeviceError DevicePluginSerialPort::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == serialPortButtonDeviceClassId ) {

        if (action.actionTypeId() == sendActionTypeId) {


            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginSerialPort::deviceRemoved(Device *device)
{
    Q_UNUSED(device);
}

