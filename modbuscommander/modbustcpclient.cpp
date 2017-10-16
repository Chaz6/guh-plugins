#include "modbustcpclient.h"
#include "extern-plugininfo.h"

ModbusTCPClient::ModbusTCPClient(QHostAddress IPv4Address, int port, int slaveAddress, QObject *parent) :
    QObject(parent),
    m_IPv4Address(IPv4Address),
    m_port(port),
    m_slaveAddress(slaveAddress)
{
    // TCP connction to target device
    m_mb = modbus_new_tcp((char *)m_IPv4Address.toString().data_ptr(), m_port);

    if(m_mb == NULL){
        qCWarning(dcModbusCommander()) << "Error Modbus TCP" << modbus_strerror(errno) ;
       this->deleteLater();
        return;
    }

    if(modbus_connect(m_mb) == -1){
        qCWarning(dcModbusCommander()) << "Error Connecting Modbus" << modbus_strerror(errno) ;
        return;
    }

    if(modbus_set_slave(m_mb, m_slaveAddress) == -1){
        qCWarning(dcModbusCommander()) << "Error Setting Slave ID" << modbus_strerror(errno) ;
        //free((void*)(m_mb));
        this->deleteLater();
        return;
    }
}

ModbusTCPClient::~ModbusTCPClient()
{
    if (m_mb != NULL) {
        modbus_close(m_mb);
    }
     modbus_free(m_mb);
}

bool ModbusTCPClient::valid()
{
    if(m_mb == NULL){
        return false;
    }

    if (modbus_read_input_registers(m_mb, 1, 1, NULL) == -1) {
        qWarning(dcModbusCommander()) << "Could not read register" << modbus_strerror(errno) ;
        return false;
    }
    return true;
}

void ModbusTCPClient::reconnect()
{
    if (!m_mb){
        return;
    }
    // Check if already connected
    if (modbus_read_input_registers(m_mb, 1, 1, NULL) == -1){
        qDebug(dcModbusCommander()) << "Could not read register:" << modbus_strerror(errno) ;
        // Try to connect to device
        if (modbus_connect(m_mb) == -1) {
            qCWarning(dcModbusCommander()) << "Connection failed: " << modbus_strerror(errno);
            return;
        }else{
            // recheck the connection
            if (modbus_read_input_registers(m_mb, 1, 1, NULL) == -1)
                return;
        }
    }
}

void ModbusTCPClient::setCoil(int coilAddress, bool status)
{
    if (!m_mb) {
        return;
    }

    if (modbus_write_bit(m_mb, coilAddress, status) == -1)
        qCWarning(dcModbusCommander()) << "Could not write Coil" << modbus_strerror(errno);
}

void ModbusTCPClient::setRegister(int registerAddress, int data)
{
    if (!m_mb) {
        return;
    }

    if (modbus_write_register(m_mb, registerAddress, data) == -1)
        qCWarning(dcModbusCommander()) << "Could not write Register" << modbus_strerror(errno);
}

bool ModbusTCPClient::getCoil(int coilAddress)
{
    if (!m_mb){
        return false;
    }

    uint8_t bits;
    if (modbus_read_bits(m_mb, coilAddress, 1, &bits) == -1){
        qCWarning(dcModbusCommander()) << "Could not read bits" << modbus_strerror(errno);
    }
    return bits;
}

int ModbusTCPClient::getRegister(int registerAddress)
{
    uint16_t reg;

    if (!m_mb){
        return 0;
    }

    if (modbus_read_registers(m_mb, registerAddress, 1, &reg) == -1){
        qCWarning(dcModbusCommander()) << "Could not read register" << modbus_strerror(errno);
    }
    return reg;
}
