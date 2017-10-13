/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Bernhard Trinnes  <bernhard.trinnes@guh.io>         *
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

#include "devicepluginserialportcommander.h"
#include "plugininfo.h"

DevicePluginSerialPortCommander::DevicePluginSerialPortCommander()
{

}

DeviceManager::HardwareResources DevicePluginSerialPortCommander::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}


DeviceManager::DeviceSetupStatus DevicePluginSerialPortCommander::setupDevice(Device *device)
{

    QString interface = device->paramValue(serialPortParamTypeId).toString();

    if (device->deviceClassId() == serialPortOutputDeviceClassId) {

        if (!m_serialPortCommanders.contains(interface)) {

            QSerialPort *serialPort = new QSerialPort(interface, this);
            if(!serialPort)
                return DeviceManager::DeviceSetupStatusFailure;

            serialPort->setBaudRate(device->paramValue(baudRateParamTypeId).toInt());
            serialPort->setDataBits(QSerialPort::DataBits(device->paramValue(dataBitsParamTypeId).toInt()));
            serialPort->setParity(QSerialPort::Parity(device->paramValue(parityParamTypeId).toInt()));
            serialPort->setStopBits(QSerialPort::StopBits(device->paramValue(stopBitsParamTypeId).toInt()));

            if (!serialPort->open(QIODevice::ReadWrite)) {
                qCWarning(dcSerialPortCommander()) << "Could not open serial port" << interface << serialPort->errorString();
                return DeviceManager::DeviceSetupStatusFailure;
            }

            qCDebug(dcSerialPortCommander()) << "Setup successfully serial port" << interface;

            SerialPortCommander *serialPortCommander = new SerialPortCommander(serialPort, this);
            connect(serialPortCommander, &SerialPortCommander::commandReceived, this, &DevicePluginSerialPortCommander::onCommandReceived);
            m_serialPortCommanders.insert(interface, serialPortCommander);

        } else {
            SerialPortCommander *serialPortCommander = m_serialPortCommanders.value(interface);
            if (serialPortCommander->hasOutputDevice())
                return DeviceManager::DeviceSetupStatusFailure;
            serialPortCommander->addOutputDevice(device);

        }
        return DeviceManager::DeviceSetupStatusSuccess;

    } else if (device->deviceClassId() == serialPortInputDeviceClassId) {

        if (!m_serialPortCommanders.contains(interface)) {

            QSerialPort *serialPort = new QSerialPort(interface, this);
            if(!serialPort)
                return DeviceManager::DeviceSetupStatusFailure;

            serialPort->setBaudRate(device->paramValue(baudRateParamTypeId).toInt());
            serialPort->setDataBits(QSerialPort::DataBits(device->paramValue(dataBitsParamTypeId).toInt()));
            serialPort->setParity(QSerialPort::Parity(device->paramValue(parityParamTypeId).toInt()));
            serialPort->setStopBits(QSerialPort::StopBits(device->paramValue(stopBitsParamTypeId).toInt()));

            if (!serialPort->open(QIODevice::ReadWrite)) {
                qCWarning(dcSerialPortCommander()) << "Could not open serial port" << interface << serialPort->errorString();
                return DeviceManager::DeviceSetupStatusFailure;
            }

            qCDebug(dcSerialPortCommander()) << "Setup successfully serial port" << interface;
            connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialError(QSerialPort::SerialPortError)));
            connect(serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

            SerialPortCommander *serialPortCommander = new SerialPortCommander(serialPort, this);
            m_serialPortCommanders.insert(interface, serialPortCommander);

        } else {
            SerialPortCommander *serialPortCommander = m_serialPortCommanders.value(interface);
            //connect(serialPortCommander, SIGNAL(commandReceived(Device *), this, SLOT(onCommandReceived(Device *));
            serialPortCommander->addInputDevice(device);
        }
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}


DeviceManager::DeviceError DevicePluginSerialPortCommander::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Create the list of available serial interfaces
    QList<DeviceDescriptor> deviceDescriptors;

    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        qCDebug(dcSerialPortCommander()) << "Found Serial interface:" << port.systemLocation() << port.portName();
        QString description = port.manufacturer() + " | " + port.description();
        DeviceDescriptor descriptor(deviceClassId, port.systemLocation(), description);
        ParamList parameters;
        parameters.append(Param(serialPortParamTypeId, port.systemLocation()));
        descriptor.setParams(parameters);
        deviceDescriptors.append(descriptor);
    }

    if (deviceClassId == serialPortInputDeviceClassId) {
        //TODO add all currenly used devices
        //
    }

    if (deviceClassId == serialPortOutputDeviceClassId) {
        //TODO add currently only as input used devices
    }

    emit devicesDiscovered(deviceClassId, deviceDescriptors);
    return DeviceManager::DeviceErrorAsync;
}


DeviceManager::DeviceError DevicePluginSerialPortCommander::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == serialPortOutputDeviceClassId ) {

        if (action.actionTypeId() == outputDataActionTypeId) {

            QString interface = device->paramValue(serialPortParamTypeId).toString();
            SerialPortCommander *serialPortCommander = m_serialPortCommanders.value(interface);
            serialPortCommander->sendCommand(action.param(outputDataAreaParamTypeId).value().toByteArray());

            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginSerialPortCommander::deviceRemoved(Device *device)
{
    QString interface = device->paramValue(serialPortParamTypeId).toString();
    SerialPortCommander *serialPortCommander = m_serialPortCommanders.value(interface);

    if (device->deviceClassId() == serialPortInputDeviceClassId) {
        serialPortCommander->removeInputDevice(device);
    }

    if (device->deviceClassId() == serialPortOutputDeviceClassId) {
        serialPortCommander->removeOutputDevice();
    }

    if (serialPortCommander->isEmpty())
        m_serialPortCommanders.remove(interface);
}


void DevicePluginSerialPortCommander::onCommandReceived(Device *device)
{
    emitEvent(Event(commandReceivedEventTypeId, device->id()));
}
