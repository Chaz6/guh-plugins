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

#ifndef DEVICEPLUGINMULTISENSOR_H
#define DEVICEPLUGINMULTISENSOR_H


#include <QPointer>
#include <QHash>
#include "plugin/deviceplugin.h"
#include "devicemanager.h"
#include "plugintimer.h"
#include "hardware/bluetoothlowenergy/bluetoothlowenergydevice.h"
#include "sensortag.h"

class DevicePluginMultiSensor : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginmultisensor.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMultiSensor();

    void init() override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

private:
    PluginTimer *m_measureTimer = nullptr;
    QHash<Device *, SensorTag *> m_sensors;

    bool verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo);

private slots:
    void onPluginTimer();

    void onSensorLeftButtonPressed();
    void onSensorRightButtonPressed();

    void onBluetoothDiscoveryFinished();
};

#endif // DEVICEPLUGINMULTISENSOR_H
