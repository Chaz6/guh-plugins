/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINELGATO_H
#define DEVICEPLUGINELGATO_H

#include "plugintimer.h"
#include "plugin/deviceplugin.h"
#include "hardware/bluetoothlowenergy/bluetoothlowenergydevice.h"

#include "nuimo.h"

class DevicePluginSenic : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginsenic.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginSenic();

    void init() override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;

private:
    QHash<Nuimo *, Device *> m_nuimos;
    PluginTimer *m_reconnectTimer = nullptr;

    bool verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo);

private slots:
    void onReconnectTimeout();
    void onBluetoothDiscoveryFinished();

    void onButtonPressed();
    void onButtonReleased();
    void onSwipeDetected(const Nuimo::SwipeDirection &direction);
    void onRotationValueChanged(const uint &value);

};

#endif // DEVICEPLUGINELGATO_H
