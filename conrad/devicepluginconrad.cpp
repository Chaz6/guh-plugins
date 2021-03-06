/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
    \page conrad.html
    \title Conrad
    \brief Plugin for Conrad 433 MHz devices.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.conrad.at}{Conrad}
    devices.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/conrad/devicepluginconrad.json
*/

#include "devicepluginconrad.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"
#include "hardware/radio433/radio433.h"

#include <QDebug>
#include <QStringList>


DevicePluginConrad::DevicePluginConrad()
{

}

DeviceManager::DeviceSetupStatus DevicePluginConrad::setupDevice(Device *device)
{
    if (device->deviceClassId() == conradShutterDeviceClassId)
        return DeviceManager::DeviceSetupStatusSuccess;

    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginConrad::executeAction(Device *device, const Action &action)
{

    if (!hardwareManager()->radio433()->available()) {
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    QList<int> rawData;
    QByteArray binCode;

    int repetitions = 10;

    if (action.actionTypeId() == conradShutterUpActionTypeId) {
        binCode = "10101000";
    } else if (action.actionTypeId() == conradShutterDownActionTypeId) {
        binCode = "10100000";
    } else if (action.actionTypeId() == conradShutterSyncActionTypeId) {
        binCode = "10100000";
        repetitions = 20;
    } else {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // append ID
    binCode.append("100101010110011000000001");

    //QByteArray remoteId = "100101010110011000000001";
    //    QByteArray motionDetectorId = "100100100101101101101010";
    //QByteArray wallSwitchId = "000001001101000010110110";
    //    QByteArray randomID     = "100010101010111010101010";



    // =======================================
    //create rawData timings list
    int delay = 650;

    // sync signal
    rawData.append(1);
    rawData.append(10);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(1);
            rawData.append(2);
        }
        if(c == '1'){
            rawData.append(2);
            rawData.append(1);
        }
    }

    // =======================================
    // send data to driver
    if(hardwareManager()->radio433()->sendData(delay, rawData, repetitions)){
        qCDebug(dcConrad) << "Transmitted successfully" << device->name() << action.actionTypeId();
    }else{
        qCWarning(dcConrad) << "Could not transmitt" << pluginName() << device->name() << action.actionTypeId();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginConrad::radioData(const QList<int> &rawData)
{
    // filter right here a wrong signal length
    if(rawData.length() != 65){
        return;
    }

    qCDebug(dcConrad) << rawData;

    int delay = rawData.first()/10;
    QByteArray binCode;

    // =======================================
    // average 650
    if(delay > 600 && delay < 750){
        // go trough all 64 timings (without sync signal)
        for(int i = 1; i <= 64; i+=2 ){
            int div;
            int divNext;

            // if short
            if(rawData.at(i) <= 900){
                div = 1;
            }else{
                div = 2;
            }
            // if long
            if(rawData.at(i+1) < 900){
                divNext = 1;
            }else{
                divNext = 2;
            }

            //              _
            // if we have  | |__ = 0 -> in 4 delays => 100
            //              __
            // if we have  |  |_ = 1 -> in 4 delays => 110

            if(div == 1 && divNext == 2){
                binCode.append('0');
            }else if(div == 2 && divNext == 1){
                binCode.append('1');
            }else{
                return;
            }
        }
    }else{
        return;
    }

    qCDebug(dcConrad) << binCode.left(binCode.length() - 24) << "  ID = " << binCode.right(24);
}
