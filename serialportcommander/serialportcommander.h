#ifndef SERIALPORTCOMMANDER_H
#define SERIALPORTCOMMANDER_H

#include <QObject>
#include <QSerialPort>
#include "extern-plugininfo.h"
#include "devicemanager.h"

class SerialPortCommander : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortCommander(QSerialPort *serialPort ,QObject *parent = 0);
    ~SerialPortCommander();

    enum ComparisonType {
        IsExactly,
        Contains,
        ContainsNot,
        StartsWith,
        EndsWith
    };

    void addOutputDevice(Device *device);
    void addInputDevice(Device *device);
    void removeInputDevice(Device *device);
    bool isEmpty();
    bool hasOutputDevice();
    void removeOutputDevice();
    void sendCommand(QByteArray data);
    QSerialPort *serialPort();
    Device *outputDevice();

private:
    QList<Device *> m_inputDevices;
    Device *m_outputDevice;
    QSerialPort *m_serialPort;

signals:
    void commandReceived(Device *device);

public slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

};

#endif // SERIALPORTCOMMANDER_H
