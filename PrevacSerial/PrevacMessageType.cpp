#include <algorithm>
#include <iomanip>
#include <iostream>

#include "PrevacMessageType.h"

static void checkDataLen(uint8_t& dataLen)
{
	if (dataLen > kdefault_max_data_len)
	{
		std::cout << "Warning: length of the data exceeds max value: " << dataLen
			<< ". Data field will assign only " << kdefault_max_data_len + 1 << " bytes\n";
		dataLen = kdefault_max_data_len;
	}
}

prevac_msg_t::prevac_msg_t() : header(0xAA), dataLen(0x00), deviceAddr(0xC8), deviceGroup(0xA1),
logicGroup(0xC8), driverAddr(0x01), functionCode(0x00), crc(0x00)
{
	// Initialize data with zeroes.
	std::fill(std::begin(data), std::end(data), 0x00);
	calculateCRC();
}

prevac_msg_t::prevac_msg_t(uint8_t header_, uint8_t dataLen_, uint8_t deviceAddr_,
	uint8_t deviceGroup_, uint8_t logicGroup_, uint8_t driverAddr_,
	uint8_t functionCode_, uint8_t* data_, uint8_t crc_)
	: header(header_), dataLen(dataLen_), deviceAddr(deviceAddr_),
	deviceGroup(deviceGroup_), logicGroup(logicGroup_),
	driverAddr(driverAddr_), functionCode(functionCode_), crc(crc_)
{
	checkDataLen(dataLen);

	// Ensure the copy does not exceed `kdefault_max_data_len`.
	if (data_ != nullptr && dataLen <= kdefault_max_data_len)
		std::copy(data_, data_ + dataLen, data);
	else
		// If data_ is null or dataLen_ exceeds `kdefault_max_data_len`, zero out the data.
		std::fill(std::begin(data), std::end(data), 0);
	calculateCRC();
}

prevac_msg_t::prevac_msg_t(prevac_msg_t const& msg_)
	: header(msg_.header), dataLen(msg_.dataLen), deviceAddr(msg_.deviceAddr),
	deviceGroup(msg_.deviceGroup), logicGroup(msg_.logicGroup), driverAddr(msg_.driverAddr),
	functionCode(msg_.functionCode), crc(msg_.crc)
{
	checkDataLen(dataLen);

	errno_t err{ memcpy_s(data, sizeof(data), msg_.data, dataLen) };
	if (err != 0)
	{
		std::cerr << "memcpy_s failed in ctor prevac_msg_t::prevac_msg_t(prevac_msg_t const&). Data field will set to 0\n";
		std::fill(std::begin(data), std::end(data), 0);
	}
	calculateCRC();
}

void prevac_msg_t::calculateCRC()
{
	// Using more bytes type for ensuring avoiding of overflow while summarizing all fields.
	// 2^16 - 1 = 65535 bytes. Max bytes when summarizing = 7 * 256 = 1792 < 65535. 
	// Where 7 - count of fields in message without header (and CRC appropriately).
	uint16_t sum{};

	sum += dataLen + deviceAddr + deviceGroup + logicGroup + driverAddr + functionCode;
	for (auto const& byte : data)
		sum += byte;

	// Calculating CRC as modulo 256 from sum.
	crc = static_cast<uint8_t>(sum % 256);
}

void prevac_msg_t::setData(std::string_view data_)
{
	if (kdefault_max_data_len < data_.length())
	{
		std::cout << "Warning: Specified data length is " << static_cast<int>(dataLen)
			<< ", passed string has length: " << data_.length() + 1 // +1 because any string has null terminator \0.
			<< ". Assigning only " << static_cast<int>(kdefault_max_data_len) + 1 << " bytes\n";
		dataLen = kdefault_max_data_len; // Adjust dataLen to maximum allowed to prevent overflow.
	}
	else
		dataLen = static_cast<uint8_t>(data_.length());

	// Copy data from string_view to the data array
	for (uint8_t i{}; i < dataLen; ++i)
		data[i] = static_cast<uint8_t>(data_[i]);

	// Zero out the rest of the data if the provided data is shorter than `kdefault_max_data_len`.
	if (dataLen < kdefault_max_data_len)
		std::memset(data + dataLen, 0, kdefault_max_data_len - dataLen);

	// Recalculate CRC code because data is set up.
	calculateCRC();
}

void prevac_msg_t::print() const
{
	std::cout << std::hex << std::uppercase << std::setfill('0');
	std::cout << std::setw(2) << static_cast<int>(header) << " "
		<< std::setw(2) << static_cast<int>(dataLen) << " "
		<< std::setw(2) << static_cast<int>(deviceAddr) << " "
		<< std::setw(2) << static_cast<int>(deviceGroup) << " "
		<< std::setw(2) << static_cast<int>(logicGroup) << " "
		<< std::setw(2) << static_cast<int>(driverAddr) << " "
		<< std::setw(2) << static_cast<int>(functionCode) << " ";
	for (uint8_t i{}; i < dataLen; ++i)
		std::cout << std::setw(2) << static_cast<int>(data[i]) << " ";
	std::cout << std::setw(2) << static_cast<int>(crc) << std::endl;
}

void prevac_msg_t::printDetailed() const
{
	std::cout << "Header: " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(header) << std::endl
		<< "Data Length: " << std::dec << static_cast<int>(dataLen) + 1 << std::endl
		<< "Device Address: " << std::hex << std::setw(2) << static_cast<int>(deviceAddr) << std::endl
		<< "Device Group: " << std::setw(2) << static_cast<int>(deviceGroup) << std::endl
		<< "Logic Group: " << std::setw(2) << static_cast<int>(logicGroup) << std::endl
		<< "Driver Address: " << std::setw(2) << static_cast<int>(driverAddr) << std::endl
		<< "Function Code: " << std::setw(2) << static_cast<int>(functionCode) << std::endl
		<< "Data: ";
	for (uint8_t i{}; i < dataLen; ++i)
		std::cout << std::hex << std::setw(2) << static_cast<int>(data[i]) << " ";
	std::cout << std::endl << "CRC: " << std::setw(2) << static_cast<int>(crc) << std::endl;
}

void prevac_msg_t::printDataAsString() const
{
	std::cout << "Data(str): ";
	for (uint8_t i{}; i < dataLen; ++i)
		std::cout << static_cast<char>(data[i]);
	std::endl(std::cout);
}
