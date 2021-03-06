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

#ifndef DEVICEPLUGINOPENWEATHERMAP_H
#define DEVICEPLUGINOPENWEATHERMAP_H

#include "plugintimer.h"
#include "plugin/deviceplugin.h"
#include "network/networkaccessmanager.h"

#include <QTimer>
#include <QHostAddress>

class DevicePluginOpenweathermap : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginopenweathermap.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginOpenweathermap();
    ~DevicePluginOpenweathermap();

    void init() override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;

private:
    PluginTimer *m_pluginTimer = nullptr;
    QList<QNetworkReply *> m_autodetectionReplies;
    QList<QNetworkReply *> m_searchReplies;
    QList<QNetworkReply *> m_searchGeoReplies;
    QHash<QNetworkReply *, Device *> m_weatherReplies;

    // Autodetection data
    QHostAddress m_wanIp;
    QString m_country;
    QString m_cityName;
    double m_longitude;
    double m_latitude;

    QString m_apiKey;

    void update(Device *device);
    void searchAutodetect();
    void search(QString searchString);
    void searchGeoLocation(double lat, double lon);

    void processAutodetectResponse(QByteArray data);
    void processSearchResponse(QByteArray data);
    void processGeoSearchResponse(QByteArray data);

    void processSearchResults(const QList<QVariantMap> &cityList);
    void processWeatherData(const QByteArray &data, Device *device);

private slots:
    void networkManagerReplyReady();
    void onPluginTimer();

};

#endif // DEVICEPLUGINOPENWEATHERMAP_H
