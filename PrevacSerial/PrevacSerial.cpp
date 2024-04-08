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
	{
#ifdef LOG_ON
		std::cerr << "Buffer overflow prevented in " << __FUNCSIG__ << '\n';
#endif
		return;
	}

	errno_t err{ memcpy_s(buffer + offset, buffer_size - offset, source, size) };
	if (err != 0)
	{
#ifdef LOG_ON
		std::cerr << "memcpy_s failed in " << __FUNCSIG__ << '\n';
#endif
		return;
	}

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
	m_dcbSerialParams.ByteSize = dataBits;       // Data bits.
	m_dcbSerialParams.Parity = parity;           // None parity.
	m_dcbSerialParams.StopBits = stopBits;       // Stop bits.
	m_dcbSerialParams.fDtrControl = flowControl; // None flow control.
	m_dcbSerialParams.BaudRate = baudRate;       // 57600 baud rate.
}

void PrevacSerial::setConnectionTimeouts(DWORD readIntervalTimeout, DWORD readTotalTimeoutMultiplier, DWORD readTotalTimeoutConstant, DWORD writeTotalTimeoutMultiplier, DWORD writeTotalTimeoutConstant)
{
	m_timeouts.ReadIntervalTimeout = readIntervalTimeout;                 // Maximum time between read chars.
	m_timeouts.ReadTotalTimeoutMultiplier = readTotalTimeoutMultiplier;   // Multiplier of characters.
	m_timeouts.ReadTotalTimeoutConstant = readTotalTimeoutConstant;       // Constant in milliseconds.
	m_timeouts.WriteTotalTimeoutMultiplier = writeTotalTimeoutMultiplier; // Multiplier of characters.
	m_timeouts.WriteTotalTimeoutConstant = writeTotalTimeoutConstant;     // Constant in milliseconds.
}

bool PrevacSerial::establishConnection(char const* portName, DWORD baudRate)
{
	m_hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hSerial == INVALID_HANDLE_VALUE)
	{
#ifdef LOG_ON
		std::cerr << "Error: Can't open serial port. Error code: " << GetLastError() << '\n';
#endif
		return false;
	}

	m_dcbSerialParams.DCBlength = sizeof(m_dcbSerialParams);
	if (!GetCommState(m_hSerial, &m_dcbSerialParams))
	{
#ifdef LOG_ON
		std::cerr << "Error: Can't get communication state. Error code: " << GetLastError() << '\n';
#endif
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	setConnectionParameters();
	if (!SetCommState(m_hSerial, &m_dcbSerialParams))
	{
#ifdef LOG_ON
		std::cerr << "Error: Can't set communication state. Maybe connection parameters are wrong. Error code: "
			<< GetLastError() << '\n';
#endif
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	setConnectionTimeouts();
	if (!SetCommTimeouts(m_hSerial, &m_timeouts))
	{
#ifdef LOG_ON
		std::cerr << "Error: Can't set communication timeouts. Maybe connection timeouts are wrong. Error code: "
			<< GetLastError() << '\n';
#endif
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
#ifdef LOG_ON
		std::cerr << "Error: Can't send message " << e.what() << '\n';
#endif
		clearBuffer_(buffer);
		return false;
	}
}

bool PrevacSerial::receiveMessage(prevac_msg_t& msg)
{
	uint8_t buffer[kdefault_max_prevac_msg_size];
	DWORD bytesRead;

	if (!readData(buffer, sizeof(buffer), bytesRead))
	{
#ifdef LOG_ON
		std::cerr << "Error: Can't read data. Error code: " << GetLastError() << '\n';
#endif
		return false;
	}

	// We can recieve max 263 bytes: where 8 from it all the parts of the message without data, and 255 bytes for the data.
	if (bytesRead < 9) // 9 bytes is the min bytes to recieve (all 9 parts of the message, where data is null).
	{
#ifdef LOG_ON
		std::cerr << "Error: Insufficient bytes read: " << bytesRead << " bytes\n";
#endif
		return false;
	}

	msg.header = buffer[0];
	msg.dataLen = buffer[1];

	DWORD expectedSize{ static_cast<DWORD>(kdefault_message_parts_count_without_data) + static_cast<DWORD>(msg.dataLen)};
	if (bytesRead != expectedSize)
	{
#ifdef LOG_ON
		std::cerr << "Error: Bytes read doesn't match expected structure size\n";
#endif
		return false;
	}

	if (buffer[0] != kdefault_header_value)
	{
#ifdef LOG_ON
		std::cerr << "Error: Invalid header or insufficient bytes read\n";
#endif
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
#ifdef LOG_ON
		std::cerr << "Error: Failed to safely copy message fields\n";
#endif
		return false;
	}

	return true;
}
