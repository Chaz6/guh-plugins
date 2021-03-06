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

/*!
    \page unitec.html
    \title Unitec
    \brief Plugin for Unitech RF 433 MHz devices.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.unitec-elektro.de}{Unitec}
    devices.

    The unitec socket units have a learn function. If you plug in the switch, a red light will start to blink. This means
    the socket is in the learning mode. Now you can add a Unitec switch (48111) to guh with your desired Channel (A,B,C or D).
    In order to pair the socket you just have to press the power ON, and the switch has to be in the pairing mode.
    If the pairing was successfull, the switch will turn on. If the switches will be removed from the socket or there will
    be a power breakdown, the switch has to be re-paired. The device can not remember the teached channel.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/unitec/devicepluginunitec.json
*/

#include "devicepluginunitec.h"
#include "devicemanager.h"
#include "plugininfo.h"
#include "hardwaremanager.h"
#include "hardware/radio433/radio433.h"

#include <QDebug>
#include <QStringList>

DevicePluginUnitec::DevicePluginUnitec()
{
}

DeviceManager::DeviceSetupStatus DevicePluginUnitec::setupDevice(Device *device)
{
    if (device->deviceClassId() != switchDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    foreach (Device* d, myDevices()) {
        if (d->paramValue(switchChannelParamTypeId).toString() == device->paramValue(switchChannelParamTypeId).toString()) {
            qCWarning(dcUnitec) << "Unitec switch with channel " << device->paramValue(switchChannelParamTypeId).toString() << "already added.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginUnitec::executeAction(Device *device, const Action &action)
{   
    if (!hardwareManager()->radio433()->available())
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    QList<int> rawData;
    QByteArray binCode;

    if (action.actionTypeId() != switchPowerActionTypeId) {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Bin codes for buttons
    if (device->paramValue(switchChannelParamTypeId).toString() == "A" && action.param(switchPowerParamTypeId).value().toBool() == true) {
        binCode.append("111011000100111010111111");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "A" && action.param(switchPowerParamTypeId).value().toBool() == false) {
        binCode.append("111001100110100001011111");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "B" && action.param(switchPowerParamTypeId).value().toBool() == true) {
        binCode.append("111011000100111010111011");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "B" && action.param(switchPowerParamTypeId).value().toBool() == false) {
        binCode.append("111000111001100111101011");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "C" && action.param(switchPowerParamTypeId).value().toBool() == true) {
        binCode.append("111000000011011111000011");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "C" && action.param(switchPowerParamTypeId).value().toBool() == false) {
        binCode.append("111001100110100001010011");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "D" && action.param(switchPowerParamTypeId).value().toBool() == true) {
        binCode.append("111001100110100001011101");
    } else if (device->paramValue(switchChannelParamTypeId).toString() == "D" && action.param(switchPowerParamTypeId).value().toBool() == false) {
        binCode.append("111000000011011111001101");
    }

    // =======================================
    //create rawData timings list
    int delay = 500;

    // add sync code
    rawData.append(6);
    rawData.append(14);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(2);
            rawData.append(1);
        }else{
            rawData.append(1);
            rawData.append(2);
        }
    }

    // =======================================
    // send data to hardware resource
    if(hardwareManager()->radio433()->sendData(delay, rawData, 10)){
        qCDebug(dcUnitec) << "transmitted" << pluginName() << device->name() << "power: " << action.param(switchPowerParamTypeId).value().toBool();
    }else{
        qCWarning(dcUnitec) << "could not transmitt" << pluginName() << device->name() << "power: " << action.param(switchPowerParamTypeId).value().toBool();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    return DeviceManager::DeviceErrorNoError;
}
