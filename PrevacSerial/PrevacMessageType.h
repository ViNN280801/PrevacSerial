#pragma once
#include <cstdint>
#include <string_view>

/* All default values are in 3.1 section of the TM13/TM14 Thikness Motor user manual. */
static constexpr uint8_t const kdefault_max_data_len{ 0xff }; ///< Maximum length of the data frame.
static constexpr uint8_t const kdefault_header_value{ 0xaa }; ///< Default value for the header of the Prevac message.
static constexpr uint8_t const kdefault_device_addr{ 0xc8 };  ///< Default value for the device address of the Prevac message.
static constexpr uint8_t const kdefault_device_group{ 0x91 }; ///< Default value for the device group of the Prevac message.
static constexpr uint8_t const kdefault_logic_group{ 0xc8 };  ///< Default value for the logic group of the Prevac message.
static constexpr uint8_t const kdefault_driver_addr{ 0x01 };  ///< Default value for the driver address of the Prevac message.

/**
 * @struct prevac_msg_t
 * @brief Structure representing a message according to the PREVAC protocol.
 */
struct prevac_msg_t {
	uint8_t header;					      ///< Protocol header, always 0xAA.
	uint8_t dataLen;			          ///< Length of the data field.
	uint8_t deviceAddr;				      ///< Hardware device address. Default value is 0xC8.
	uint8_t deviceGroup;			      ///< Type of the device. EBV Powers Supply (0x91), TM13/TM14(0xA1).
	uint8_t logicGroup;				      ///< Group of devices in link layer. Default value is 0xC8.
	uint8_t driverAddr;				      ///< Address of sender. Usually 0x01.
	uint8_t functionCode;			      ///< Code function of procedure.
	uint8_t data[kdefault_max_data_len];  ///< Data needed to realize defined functions.
	uint8_t crc;					      ///< CRC checksum.

	/// @brief Default ctor with default values which are specified from user manual (TM13/TM14 Thickness Monitor).
	prevac_msg_t();

	/// @brief Parametrized ctor.
	prevac_msg_t(uint8_t header_, uint8_t dataLen_, uint8_t deviceAddr_,
		uint8_t deviceGroup_, uint8_t  logicGroup_, uint8_t driverAddr_,
		uint8_t functionCode_, uint8_t* data_, uint8_t crc_);

	/// @brief Parametrized with another instance of `prevac_msg_t` ctor.
	prevac_msg_t(prevac_msg_t const& msg_);

	/**
	 * @brief Calculates CRC (Cyclic Redundancy Code)
	 *        From the user manual: CRC is simple modulo 256 calculate without protocol header byte.
	 */
	void calculateCRC();

	/**
	 * @brief Fills the data with specified string.
	 * @param data_ String to assign the data field. Accepts types: char, string, const char[], etc.
	 */
	void setData(std::string_view data_);

	/**
	 * @brief Calculates the total size of the message, combining fixed and variable parts.
	 * @return Total size of the message in bytes.
	 */
	constexpr size_t size() const {
		return sizeof(header) + sizeof(dataLen) + sizeof(deviceAddr) +
			sizeof(deviceGroup) + sizeof(logicGroup) + sizeof(driverAddr) +
			sizeof(functionCode) + dataLen + sizeof(crc);
	}

	/**
	 * @brief Prints the message in a compact hexadecimal format.
	 *
	 * This method outputs the message content as a single line of hexadecimal values,
	 * providing a compact representation of the message. The output format follows this pattern:
	 * <header> <dataLen> <deviceAddr> <deviceGroup> <logicGroup> <driverAddr> <functionCode> <dataField...> <crc>
	 * Each field is printed as a two-digit hexadecimal number, upper case, and separated by spaces.
	 * The <dataField...> part represents the data field content up to the length specified by dataLen.
	 */
	void print() const;

	/**
	 * @brief Prints the message in a detailed, readable format with field names.
	 *
	 * This method outputs each field of the message on a new line with a descriptive label,
	 * making it easy to read and understand the message's content. The output format is as follows:
	 * Header: <header>
	 * Data Length: <dataLen>
	 * Device Address: <deviceAddr>
	 * Device Group: <deviceGroup>
	 * Logic Group: <logicGroup>
	 * Driver Address: <driverAddr>
	 * Function Code: <functionCode>
	 * Data: <dataField...>
	 * CRC: <crc>
	 * The header, device address, device group, logic group, driver address, function code, and CRC are
	 * printed as two-digit hexadecimal numbers. The data length is printed as a decimal number,
	 * and the data field is printed in hexadecimal format, showing each byte as a two-digit number.
	 */
	void printDetailed() const;

	/// @brief Prints to the terminal data that converted from hexadecimal format to string.
	void printDataAsString() const;
};
