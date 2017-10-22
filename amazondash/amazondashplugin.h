/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2015 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
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

#ifndef DEVICEPLUGINAMAZONDASH_H
#define DEVICEPLUGINAMAZONDASH_H

#include "plugin/deviceplugin.h"
#include "types/action.h"

#include <QHash>
#include <QTimer>

#include <pcap.h>

class AmazonDashPlugin: public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginamazondash.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit AmazonDashPlugin();
    ~AmazonDashPlugin();

    virtual void init() override;

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;

private:
    void setupPcap();

private slots:
    void pollPcap();

private:
    bool m_discovering = false;
    pcap_t *m_pcap;
    QTimer m_pollTimer;
};

#endif // DEVICEPLUGINAMAZONDASH_H
