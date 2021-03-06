/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Alexander Lampret <alexander.lampret@gmail.com>     *
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

#ifndef DEVICEPLUGINUSBWDE_H
#define DEVICEPLUGINUSBWDE_H

#include "plugin/deviceplugin.h"
#include "devicemanager.h"
#include "plugintimer.h"

#include <QSerialPort>

class DevicePluginUsbWde : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginusbwde.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginUsbWde();
    ~DevicePluginUsbWde();

    void init() override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

private:
    PluginTimer *m_pluginTimer = nullptr;
    Device *m_bridgeDevice = nullptr;
    QSerialPort *m_serialPort = nullptr;
    QByteArray  m_readData;
    QHash<int, Device *> m_deviceList;

    void createNewSensor(int channel);

private slots:
    void onPluginTimer();
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
};

#endif // DEVICEPLUGINUSBWDE_H
