/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#include "amazondashplugin.h"
#include "plugin/device.h"
#include "plugininfo.h"
#include "types/event.h"

#include <QtConcurrent/QtConcurrent>

AmazonDashPlugin::AmazonDashPlugin()
{
    connect(&m_pollTimer, &QTimer::timeout, this, &AmazonDashPlugin::pollPcap);
}

AmazonDashPlugin::~AmazonDashPlugin()
{
    if (m_pcap) {
        pcap_close(m_pcap);
    }
}

void AmazonDashPlugin::init()
{
    setupPcap();
}

DeviceManager::HardwareResources AmazonDashPlugin::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus AmazonDashPlugin::setupDevice(Device *device)
{
    qCDebug(dcamazonDash()) << "Setup Amazon dash button" << device->name() << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

void AmazonDashPlugin::setupPcap()
{
    char errBuff[PCAP_ERRBUF_SIZE];

    m_pcap = pcap_open_live("any" /*iface*/, 65535 /*snaplen*/, 1 /*promisc*/, 0 /*to_ms*/, errBuff /**errbuf*/);
    if (!m_pcap) {
        qCWarning(dcamazonDash()) << "Error opening pcap. This plugin requires root permissions:" << errBuff;
        return;
    }
    qCDebug(dcamazonDash()) << "PCAP opened";

    struct bpf_program fp;
    bpf_u_int32 netp;
    bpf_u_int32 maskp;
    if (pcap_lookupnet("any", &netp, &maskp, errBuff) != 0) {
        qCWarning(dcamazonDash()) << "Error looking up network for interface any";
        return;
    }
    if (pcap_compile(m_pcap, &fp, "arp", 0, netp) != 0) {
        qCWarning(dcamazonDash()) << "Error compiling filter for ARP packets";
        return;
    }

    pcap_setfilter(m_pcap, &fp);

    pcap_setnonblock(m_pcap, 1, errBuff);

    m_pollTimer.start(50);
    qCDebug(dcamazonDash()) << "Polling for PCAP packets started...";
}

void AmazonDashPlugin::pollPcap()
{
    struct pcap_pkthdr *header;
    const u_char *pkt_data;

    while(pcap_next_ex(m_pcap, &header, &pkt_data) > 0) {
//        qCDebug(dcamazonDash()) << "ARP packet received";

        QByteArray data = QByteArray((char*)pkt_data, header->len).toHex();

        // ARP packet format:

        // Variant 1: direct broadcast message received:
        // ffffffffffff78e103c8006c0806000108000604000178e103c8006c0a0a0a740000000000000a0a0a01
        // |     1    ||    2     ||3 ||4 ||5 ||||||8 ||    9     ||  10  ||    11    ||  12  |

        // 1: DST MAC (ffffffffffff broadcast)
        // 2: SRC MAC (78e103xxxxxx Amazon block)
        // 3: Type (0806 ARP)
        // 4: HW Type (0001 Ethernet)
        // 5: Protocol (0800 IPv4)
        // 6: HW size (6)
        // 7: Protocol size (4)
        // 8: Opcode (1 request)
        // 9: Sender MAC (same as 2)
        // 10: Sender IP (0a0a0a74 -> 10.10.10.116)
        // 11: Target MAC (000000000000 -> unknown)
        // 12: Target IP (a0a0a01 -> 10.10.10.1)

        // Variant 2: VLAN enabled (802.1Q tag (0000) after SRC MAC)
        // 00010001000678e103c8006c00000806000108000604000178e103c8006c0a0a0a740000000000000a0a0a01

        data.remove(0, 12); // remove DST MAC
        if (!data.startsWith("78e103")) {
            // Not a ARP broadcast with a source from the amazon block
//            qCDebug(dcamazonDash()) << "ARP packet doesn't come from an Amazon registered MAC address" << data;
            continue;
        }
        QByteArray mac = data.left(12);
        QString macAddress;
        for (int i = 0; i < 6; i++) {
            if (!macAddress.isEmpty()) {
                macAddress.append(':');
            }
            macAddress.append(mac.at(i));
            macAddress.append(mac.at(i+1));
        }
        qCDebug(dcamazonDash()) << "Received an ARP broadcast packet from an Amazon device:" << macAddress;

        if (m_discovering) {
            DeviceDescriptor descriptor(dashButtonDeviceClassId, "Amazon Dash Button", macAddress);
            descriptor.setParams(ParamList() << Param(macAddressParamTypeId, macAddress));
            emit devicesDiscovered(dashButtonDeviceClassId, {descriptor});
            m_discovering = false;
        }

        QList<Device*> devices = myDevices();
        foreach (Device *dev, devices) {
            if (dev->paramValue(macAddressParamTypeId).toString() == macAddress) {
                Event evt(pressedEventTypeId, dev->id());
                emit emitEvent(evt);
                break;
            }
        }
    }
}

DeviceManager::DeviceError AmazonDashPlugin::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)

    m_discovering = true;
    return DeviceManager::DeviceErrorAsync;
}
