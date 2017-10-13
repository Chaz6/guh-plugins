#include "serialportcommander.h"

SerialPortCommander::SerialPortCommander(QSerialPort *serialPort, QObject *parent) :
    QObject(parent),
    m_serialPort(serialPort)
{
    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialError(QSerialPort::SerialPortError)));
    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}


SerialPortCommander::~SerialPortCommander()
{
    m_serialPort->close();
    m_serialPort->deleteLater();
}


void SerialPortCommander::addOutputDevice(Device* device)
{
    m_outputDevice = device;
    return;
}


void SerialPortCommander::removeOutputDevice()
{
    m_outputDevice = NULL;
}


void SerialPortCommander::addInputDevice(Device* device)
{
    m_inputDevices.append(device);
}


void SerialPortCommander::removeInputDevice(Device* device)
{
    m_inputDevices.removeOne(device);
}


bool SerialPortCommander::isEmpty()
{
    return(!hasOutputDevice() || m_inputDevices.empty());
}


bool SerialPortCommander::hasOutputDevice()
{
    if (m_outputDevice == NULL) {
        return false;
    } else {
        return true;
    }
}


QSerialPort * SerialPortCommander::serialPort()
{
    return m_serialPort;
}


Device * SerialPortCommander::outputDevice()
{
    return m_outputDevice;
}


void SerialPortCommander::onReadyRead()
{
    QByteArray data;
    while (!m_serialPort->atEnd()) {
        data = m_serialPort->read(100);
    }
    qDebug(dcSerialPortCommander()) << "Message received" << data;

    foreach (Device *device, m_inputDevices) {
        if (device->paramValue(comparisonTypeParamTypeId).toString() == "Is exactly") {
            if (data == device->paramValue(inputCommandParamTypeId)) {
                emit commandReceived(device);
            }
        } else if (device->paramValue(comparisonTypeParamTypeId).toString() == "Contains") {
            if (data.contains(device->paramValue(inputCommandParamTypeId).toByteArray())) {
                emit commandReceived(device);
            }
        } else if (device->paramValue(comparisonTypeParamTypeId) == "Contains not") {
            if (!data.contains(device->paramValue(inputCommandParamTypeId).toByteArray())) {
               emit commandReceived(device);
            }
        } else if (device->paramValue(comparisonTypeParamTypeId) == "Starts with") {
            if (data.startsWith(device->paramValue(inputCommandParamTypeId).toByteArray())) {
                emit commandReceived(device);
            }
        } else if (device->paramValue(comparisonTypeParamTypeId) == "Ends with") {
            if (data.endsWith(device->paramValue(inputCommandParamTypeId).toByteArray())) {
                emit commandReceived(device);
            }
        }
    }
}

void SerialPortCommander::onSerialError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error);
}

void SerialPortCommander::sendCommand(QByteArray data)
{
    m_serialPort->write(data);
}
