/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINWEMO_H
#define DEVICEPLUGINWEMO_H

#include "plugintimer.h"
#include "plugin/deviceplugin.h"

#include <QNetworkReply>

class DevicePluginWemo : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginwemo.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWemo();
    ~DevicePluginWemo();

    void init() override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;
    void deviceRemoved(Device *device) override;

private:
    PluginTimer *m_pluginTimer = nullptr;
    QHash<QNetworkReply *, Device *> m_refreshReplies;
    QHash<QNetworkReply *, Device *> m_setPowerReplies;
    QHash<QNetworkReply *, ActionId> m_runningActionExecutions;

    void refresh(Device* device);
    bool setPower(Device *device, const bool &power, const ActionId &actionId);

    void processRefreshData(const QByteArray &data, Device *device);
    void processSetPowerData(const QByteArray &data, Device *device, const ActionId &actionId);

private slots:
    void onNetworkReplyFinished();
    void onPluginTimer();
    void onUpnpDiscoveryFinished();
    void onUpnpNotifyReceived(const QByteArray &notification);

};

#endif // DEVICEPLUGINWEMO_H
