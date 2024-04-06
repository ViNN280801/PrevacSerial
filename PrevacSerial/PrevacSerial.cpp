#include <exception>
#include <iostream>
#include <cstring>

#include "PrevacSerial.h"
#include "Utilities.h"

void PrevacSerial::clearBuffer_(uint8_t* buffer)
{
	if (buffer)
		delete[] buffer;
	buffer = nullptr;
}

void PrevacSerial::copyToBuffer_(uint8_t* buffer, size_t buffer_size, size_t& offset, const void* source, size_t size)
{
	if (offset + size > buffer_size)
		throw std::runtime_error("Buffer overflow prevented in copyToBuffer_");

	errno_t err{ memcpy_s(buffer + offset, buffer_size - offset, source, size) };
	if (err != 0)
		throw std::runtime_error("memcpy_s failed in copyToBuffer_");

	offset += size;
}

void PrevacSerial::buildMessage_(uint8_t* buffer, size_t buffer_size, prevac_msg_t const& msg)
{
	size_t offset{};
	copyToBuffer_(buffer, buffer_size, offset, &msg.header, sizeof(msg.header));
	copyToBuffer_(buffer, buffer_size, offset, &msg.dataLen, sizeof(msg.dataLen));
	copyToBuffer_(buffer, buffer_size, offset, &msg.deviceAddr, sizeof(msg.deviceAddr));
	copyToBuffer_(buffer, buffer_size, offset, &msg.deviceGroup, sizeof(msg.deviceGroup));
	copyToBuffer_(buffer, buffer_size, offset, &msg.logicGroup, sizeof(msg.logicGroup));
	copyToBuffer_(buffer, buffer_size, offset, &msg.driverAddr, sizeof(msg.driverAddr));
	copyToBuffer_(buffer, buffer_size, offset, &msg.functionCode, sizeof(msg.functionCode));
	copyToBuffer_(buffer, buffer_size, offset, msg.data, msg.dataLen);
	copyToBuffer_(buffer, buffer_size, offset, &msg.crc, sizeof(msg.crc));
}

PrevacSerial::~PrevacSerial()
{
	if (m_hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
	}
}

void PrevacSerial::setConnectionParameters(BYTE dataBits, BYTE parity, BYTE stopBits, DWORD flowControl, DWORD baudRate)
{
	m_dcbSerialParams.ByteSize = 8;                      // Data bits.
	m_dcbSerialParams.Parity = NOPARITY;                 // None parity.
	m_dcbSerialParams.StopBits = ONE5STOPBITS;           // Stop bits.
	m_dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE; // None flow control.
	m_dcbSerialParams.BaudRate = baudRate;               // 57600 baud rate.
}

bool PrevacSerial::establishConnection(const char* portName, DWORD baudRate)
{
	m_hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hSerial == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error: Can't open serial port. Error code: " << GetLastError() << '\n';
		return false;
	}

	m_dcbSerialParams.DCBlength = sizeof(m_dcbSerialParams);
	if (!GetCommState(m_hSerial, &m_dcbSerialParams))
	{
		std::cerr << "Error: Can't get comm state. Error code: " << GetLastError() << '\n';
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	// Setting connection params.
	setConnectionParameters(8, NOPARITY, ONE5STOPBITS, DTR_CONTROL_DISABLE, baudRate);

	if (!SetCommState(m_hSerial, &m_dcbSerialParams))
	{
		std::cerr << "Error: Can't set comm state. Error code: " << GetLastError() << '\n';
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	// Setting communication timeouts.
	m_timeouts.ReadIntervalTimeout = 50;
	m_timeouts.ReadTotalTimeoutConstant = 50;
	m_timeouts.ReadTotalTimeoutMultiplier = 10;
	m_timeouts.WriteTotalTimeoutConstant = 50;
	m_timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(m_hSerial, &m_timeouts))
	{
		std::cerr << "Error: Can't set communication timeouts. Error code: " << GetLastError() << '\n';
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	return true;
}

bool PrevacSerial::writeData(const uint8_t* data, size_t size)
{
	DWORD bytesWritten;
	return WriteFile(m_hSerial, data, size, &bytesWritten, NULL) && size == bytesWritten;
}

bool PrevacSerial::readData(uint8_t* buffer, size_t bufferSize, DWORD& bytesRead) { return ReadFile(m_hSerial, buffer, bufferSize, &bytesRead, NULL); }

bool PrevacSerial::sendMessage(prevac_msg_t const& msg)
{
	size_t messageSize{ msg.size() };
	uint8_t* buffer{ new uint8_t[messageSize] };

	try
	{
		buildMessage_(buffer, messageSize, msg);
		bool result{ writeData(buffer, messageSize) };
		clearBuffer_(buffer);
		return result;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: Can't send message " << e.what() << '\n';
		clearBuffer_(buffer);
		return false;
	}
}

bool PrevacSerial::receiveMessage(prevac_msg_t& msg)
{
	uint8_t buffer[kdefault_max_data_len];
	DWORD bytesRead;

	if (!readData(buffer, sizeof(buffer), bytesRead))
	{
		std::cerr << "Error: Can't read data. Error code: " << GetLastError() << '\n';
		return false;
	}

	if (bytesRead < sizeof(prevac_msg_t) - kdefault_max_data_len + 1 || buffer[0] != 0xAA)
	{
		std::cerr << "Error: Invalid header or insufficient bytes read\n";
		return false;
	}

	size_t expectedDataLen{ buffer[1] };
	if (bytesRead != sizeof(prevac_msg_t) - kdefault_max_data_len + expectedDataLen)
	{
		std::cerr << "Error: Bytes read doesn't match expected structure size\n";
		return false;
	}

	// Direct parsing assuming fixed-size leading fields and dynamic data field length.
	size_t offset{};
	size_t bufferSize{ sizeof(buffer) };

	if (!safeCopyFromBuffer(msg.header, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.dataLen, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.deviceAddr, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.deviceGroup, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.logicGroup, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.driverAddr, offset, buffer, bufferSize) ||
		!safeCopyFromBuffer(msg.functionCode, offset, buffer, bufferSize))
	{
		std::cerr << "Error: Failed to safely copy message fields\n";
		return false;
	}

	return true;
}
