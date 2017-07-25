/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.io>                  *
 *  Copyright (C) 2016 nicc                                                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page multisensor.html
    \title MultiSensor
    \brief Plugin for TI SensorTag.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows finding and controlling the Bluetooth Low Energy SensorTag from Texas Instruments.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details on how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/multisensor/devicepluginmultisensor.json
*/

#ifdef BLUETOOTH_LE

#include "plugininfo.h"
#include "devicemanager.h"
#include "bluetooth/bluetoothlowenergydevice.h"
#include "devicepluginmultisensor.h"

DevicePluginMultiSensor::DevicePluginMultiSensor()
{

}

DeviceManager::HardwareResources DevicePluginMultiSensor::requiredHardware() const
{
    return DeviceManager::HardwareResourceBluetoothLE;
}

DeviceManager::DeviceError DevicePluginMultiSensor::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    if (deviceClassId != sensortagDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    if (!discoverBluetooth())
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginMultiSensor::bluetoothDiscoveryFinished(const QList<QBluetoothDeviceInfo> &deviceInfos)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (auto deviceInfo, deviceInfos) {
        if (deviceInfo.name().contains("SensorTag")) {
            if (!verifyExistingDevices(deviceInfo)) {
                DeviceDescriptor descriptor(sensortagDeviceClassId, "SensorTag", deviceInfo.address().toString());
                ParamList params;
                params.append(Param(nameParamTypeId, deviceInfo.name()));
                params.append(Param(macParamTypeId, deviceInfo.address().toString()));
                descriptor.setParams(params);
                deviceDescriptors.append(descriptor);
            }
        }
    }

    emit devicesDiscovered(sensortagDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceSetupStatus DevicePluginMultiSensor::setupDevice(Device *device)
{
    qCDebug(dcMultiSensor) << "Setting up MultiSensor" << device->name() << device->params();

    if (device->deviceClassId() == sensortagDeviceClassId) {
        auto address = QBluetoothAddress(device->paramValue(macParamTypeId).toString());
        auto name = device->paramValue(nameParamTypeId).toString();
        auto deviceInfo = QBluetoothDeviceInfo(address, name, 0);

        QSharedPointer<SensorTag> tag{new SensorTag(deviceInfo, QLowEnergyController::PublicAddress, this)};
        connect(tag.data(), &SensorTag::valueChanged, this,
                [device, this](StateTypeId state, QVariant value) { device->setStateValue(state, value); });
        connect(tag.data(), &SensorTag::event, this,
                [device, this](EventTypeId event) { emit emitEvent(Event(event, device->id())); });
        m_tags.insert(tag, device);

        tag->connectDevice();

        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}


void DevicePluginMultiSensor::deviceRemoved(Device *device)
{
    if (!m_tags.values().contains(device))
        return;

    auto tag= m_tags.key(device);
    m_tags.remove(tag);
}

bool DevicePluginMultiSensor::verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo)
{
    foreach (Device *device, myDevices()) {
        if (device->paramValue(macParamTypeId).toString() == deviceInfo.address().toString())
            return true;
    }

    return false;
}

#endif // BLUETOOTH_LE