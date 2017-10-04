/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.io>                  *
 *  Copyright (C) 2016 Christian Stachowitz                                *
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

#include "devicepluginkeba.h"

#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QPointer>
#include "plugininfo.h"
#include <QUdpSocket>

/* Notes:
  *
  * Test IPs: 192.168.0.66
  *
  */



DevicePluginKeba::DevicePluginKeba()
{
}

DeviceManager::HardwareResources DevicePluginKeba::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginKeba::setupDevice(Device *device)
{
    qCDebug(dcKebaKeContact()) << "Setting up a new device:" << device->name() << device->params();

    if(m_kebaDevices.isEmpty())
    {
        m_kebaSocket = new QUdpSocket(this);
        if (!m_kebaSocket->bind(QHostAddress::AnyIPv4,7090)) {
            qCWarning(dcKebaKeContact()) << "can't bind to port";
            delete m_kebaSocket;
            return DeviceManager::DeviceSetupStatusFailure;
        }
        connect(m_kebaSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
        qCDebug(dcKebaKeContact()) << "create keba socket";
    }
    QHostAddress address = QHostAddress(device->paramValue(wallboxIpParamTypeId).toString());

    //Check if the IP is empty
    if(address.isNull()){
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // check if IP is already added to another keba device
    if(m_kebaDevices.keys().contains(address)){
        return DeviceManager::DeviceSetupStatusFailure;
    }

    m_kebaDevices.insert(address, device);
    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginKeba::postSetupDevice(Device *device)
{
    qCDebug(dcKebaKeContact()) << "Post setup" << device->name();
    QByteArray datagram;
    datagram.append("report 2");
    m_kebaSocket->writeDatagram(datagram.data(),datagram.size(), QHostAddress(device->paramValue(wallboxIpParamTypeId).toString()) , 7090);
}

void DevicePluginKeba::deviceRemoved(Device *device)
{
    // Remove devices
    QHostAddress address = m_kebaDevices.key(device);
    m_kebaDevices.remove(address);

    if(m_kebaDevices.isEmpty()){
        m_kebaSocket->close();
        m_kebaSocket->deleteLater();
        qCDebug(dcKebaKeContact()) << "clear socket";
    }
}

void DevicePluginKeba::guhTimer()
{
    foreach (QHostAddress address, m_kebaDevices.keys()) {
        QByteArray datagram;
        datagram.append("report 2");
        qCDebug(dcKebaKeContact()) << "datagram : " << datagram;
        m_kebaSocket->writeDatagram(datagram.data(),datagram.size(), address , 7090);
        //set reachable false until successful reply from device
        m_kebaDevices.value(address)->setStateValue(deviceReachableStateTypeId,false);
    }
}

DeviceManager::DeviceError DevicePluginKeba::executeAction(Device *device, const Action &action)
{
    qCDebug(dcKebaKeContact()) << "Execute action" << device->name() << action.actionTypeId().toString();

    if (device->deviceClassId() == wallboxDeviceClassId) {

        // Print information that we are executing now the update action
        qCDebug(dcKebaKeContact()) << "Execute update action" << action.id();

        if(action.actionTypeId() == maxCurrentActionTypeId){
            // Print information that we are executing now the update action
            qCDebug(dcKebaKeContact()) << "update max current to : " << action.param(maxCurrentStateParamTypeId).value().toString();
            QByteArray datagram;
            datagram.append("curr " + QVariant(action.param(maxCurrentStateParamTypeId).value().toInt()*1000).toString());
            qCDebug(dcKebaKeContact()) << "datagram : " << datagram;
            m_kebaSocket->writeDatagram(datagram.data(),datagram.size(), QHostAddress(device->paramValue(wallboxIpParamTypeId).toString()) , 7090);
        }
        else if(action.actionTypeId() == outEnableActionTypeId){
            // Print information that we are executing now the update action
            qCDebug(dcKebaKeContact()) << "output enable : " << action.param(outEnableStateParamTypeId).value().toString();
            QByteArray datagram;
            if(action.param(outEnableStateParamTypeId).value().toBool()){
                datagram.append("ena 1");
            }
            else{
                datagram.append("ena 0");
            }
            qCDebug(dcKebaKeContact()) << "datagram : " << datagram;
            m_kebaSocket->writeDatagram(datagram.data(),datagram.size(), QHostAddress(device->paramValue(wallboxIpParamTypeId).toString()) , 7090);
        }

        return DeviceManager::DeviceErrorNoError;
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginKeba::readPendingDatagrams()
{
    QUdpSocket *socket= qobject_cast<QUdpSocket*>(sender());

    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort;

    while (socket->hasPendingDatagrams()) {
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        qCDebug(dcKebaKeContact()) << " got command from" << sender.toString() << senderPort;
    }

    if(!m_kebaDevices.keys().contains(sender)){
        qCDebug(dcKebaKeContact()) << " unknown sender:" << sender.toString() << senderPort;
        return;
    }

    // Convert the rawdata to a json document
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcKebaKeContact()) << "Failed to parse JSON data" << datagram << ":" << error.errorString();
        return;
    }

    // print the fetched data in json format to stdout
    //qCDebug(dcKebaKeContact()) << qUtf8Printable(jsonDoc.toJson());

    QVariantMap data = jsonDoc.toVariant().toMap();

    qCDebug(dcKebaKeContact()) << "IP" << sender << "device: " << m_kebaDevices.value(sender);

    if(data.contains("ID")){
        // check if ID matches report 2 or report 3
        if(data.value("ID").toString() == "2"){
            //set reachable
            m_kebaDevices.value(sender)->setStateValue(deviceReachableStateTypeId,true);
            //activity state
            if(data.value("State").toString() == "0"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"starting");
            }
            else if(data.value("State").toString() == "1"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"not ready for charging");
            }
            else if(data.value("State").toString() == "2"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"ready for charging");
            }
            else if(data.value("State").toString() == "3"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"charging");
            }
            else if(data.value("State").toString() == "4"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"error");
            }
            else if(data.value("State").toString() == "5"){
                m_kebaDevices.value(sender)->setStateValue(activityStateTypeId,"authorization rejected");
            }
            // plug state
            if(data.value("Plug").toString() == "0"){
                m_kebaDevices.value(sender)->setStateValue(plugStateStateTypeId,"unplugged");
            }
            else if(data.value("Plug").toString() == "1"){
                m_kebaDevices.value(sender)->setStateValue(plugStateStateTypeId,"plugged on charging station");
            }
            else if(data.value("Plug").toString() == "3"){
                m_kebaDevices.value(sender)->setStateValue(plugStateStateTypeId,"locked plug on charging station");
            }
            else if(data.value("Plug").toString() == "5"){
                m_kebaDevices.value(sender)->setStateValue(plugStateStateTypeId,"plugged on charging station and vehicle");
            }
            else if(data.value("Plug").toString() == "7"){
                m_kebaDevices.value(sender)->setStateValue(plugStateStateTypeId,"locked plug on charging station and vehicle");
            }
            //maximum current setting
            m_kebaDevices.value(sender)->setStateValue(maxCurrentStateTypeId,data.value("Curr user").toInt()/1000);
            //output setting
            m_kebaDevices.value(sender)->setStateValue(outEnableStateTypeId,data.value("Enable user").toBool());

            //request next report
            QByteArray datagram;
            datagram.append("report 3");
            qCDebug(dcKebaKeContact()) << "datagram : " << datagram;
            socket->writeDatagram(datagram.data(),datagram.size(), QHostAddress(m_kebaDevices.value(sender)->paramValue(wallboxIpParamTypeId).toString()) , 7090);
        }
        else if(data.value("ID").toString() == "3"){
            //power of current charging session
            m_kebaDevices.value(sender)->setStateValue(powerStateTypeId,data.value("E pres").toInt() / 1000);
            //current phase 1
            m_kebaDevices.value(sender)->setStateValue(currentStateTypeId,data.value("I1").toInt() * 1000);
        }
    }

}