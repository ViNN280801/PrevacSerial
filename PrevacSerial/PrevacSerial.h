#pragma once
#include <windows.h>

#include "PrevacMessageType.h"

/// @brief Manages serial communication for PREVAC protocol messages.
class PrevacSerial {
private:
	HANDLE m_hSerial{ INVALID_HANDLE_VALUE }; ///< Handle for the serial connection.
	DCB m_dcbSerialParams{};                  ///< Structure containing the control settings for a serial communications device.
	COMMTIMEOUTS m_timeouts{};                ///< Structure containing the time-out parameters for a serial communications device.

	/**
	 * @brief Clears out pointer on the buffer.
	 *		  Checks if buffer not null inside to avoid double free.
	 *		  Clears out the pointer (sets it to `nullptr`).
	 *
	 * @param buffer Pointer to clear.
	*/
	void clearBuffer_(uint8_t* buffer);

	/**
	 * @brief Copies data from source to buffer at the specified offset and updates the offset.
	 * @param buffer Pointer to the destination buffer.
	 * @param buffer_size Size of the buffer.
	 * @param offset Reference to the current offset in the buffer, will be updated after copy.
	 * @param source Pointer to the source data to copy.
	 * @param size Number of bytes to copy.
	 */
	void copyToBuffer_(uint8_t* buffer, size_t buffer_size, size_t& offset, const void* source, size_t size);

	/**
	 * @brief Constructs a PREVAC protocol message and writes it into a buffer.
	 *		  This method serializes the components of a prevac_msg_t structure into a contiguous
	 *		  block of memory, effectively creating a message ready to be sent over a serial connection.
	 *        It handles the serialization process by copying each field of the message structure
	 *        into the provided buffer, respecting the order and size of each field.
	 *
	 * @param[out] buffer Pointer to the buffer where the serialized message will be stored.
	 *                    The buffer must be large enough to hold the entire message.
	 *                    The caller is responsible for allocating and deallocating the buffer.
	 * @param buffer_size Size of the buffer.
	 *
	 * @param[in] msg The prevac_msg_t structure that contains the message data to be serialized.
	 *                This structure provides the content and format of the message according
	 *                to the PREVAC protocol specifications.
	 */
	void buildMessage_(uint8_t* buffer, size_t buffer_size, prevac_msg_t const& msg);

public:
	PrevacSerial() = default;
	~PrevacSerial();

	/**
	 * @brief Sets the connection parameters for the serial communication.
	 *
	 * Configures the serial port connection parameters according to the specifications provided
	 * in the TM13/TM14 Thickness Monitor user manual. The function allows customization of data bits,
	 * parity, stop bits, flow control, and baud rate, though the default values are aligned with the
	 * device's requirements.
	 *
	 * @param dataBits Number of data bits per byte. The TM13/TM14 uses 8 data bits.
	 * @param parity Type of parity to use. The TM13/TM14 requires None (0).
	 *               Possible values are: NOPARITY (0), ODDPARITY (1), EVENPARITY (2),
	 *               MARKPARITY (3), and SPACEPARITY (4).
	 * 
	 * @param stopBits Number of stop bits to use. The TM13/TM14 uses 1 stop bit.
	 *                 Possible values are: ONESTOPBIT (0), ONE5STOPBITS (1), and TWOSTOPBITS (2).
	 * 
	 * @param flowControl Type of flow control to use. The TM13/TM14 requires None.
	 *                    Specify as DWORD values corresponding to the desired flow control settings.
	 *                    Common values include 0 (None), XON/XOFF, and RTS/CTS.
	 * 
	 * @param baudRate Communication speed in bits per second (bps). For the TM13/TM14, this is fixed at 57600 bps.
	 *                 Default value is CBR_57600. Other standard baud rates can be specified but may not be
	 *                 applicable for the TM13/TM14.
	 *
	 * @note This function should be called to configure the serial port before attempting to communicate with
	 *       the TM13/TM14 Thickness Monitor to ensure compatibility with the device's communication parameters.
	 */
	void setConnectionParameters(BYTE dataBits, BYTE parity, BYTE stopBits,
		DWORD flowControl, DWORD baudRate = CBR_57600);

	/**
	 * @brief Establishes a serial connection to a specified port with a given baud rate.
	 *
	 * Default connection parameters from the user manual (TM13/TM14 Thickness Monitor).
	 * 3.2 Connection parameters:
	 *    Data bits: 8
	 *    Parity: None
	 *    Stop bits: 1
	 *    Flow control: None
	 *    Baud rate: 57600 (fixed value)
	 *
	 * @param portName Name of the port to connect to (e.g., "COM1", "COM2", etc.).
	 * @param baudRate Baud rate for the connection.
	 * @return True if connection was successfully established, False otherwise.
	 */
	bool establishConnection(const char* portName, DWORD baudRate = CBR_57600);

	/**
	 * @brief Writes data to the serial port.
	 * @param data Pointer to the data to write.
	 * @param size Number of bytes to write.
	 * @return True if data was successfully written, False otherwise.
	 */
	bool writeData(const uint8_t* data, size_t size);

	/**
	 * @brief Reads data from the serial port.
	 * @param buffer Pointer to the buffer to store read data.
	 * @param bufferSize Size of the buffer, indicating max bytes to read.
	 * @param bytesRead Reference to DWORD to store the number of bytes actually read.
	 * @return True if data was successfully read, False otherwise.
	 */
	bool readData(uint8_t* buffer, size_t bufferSize, DWORD& bytesRead);

	/**
	 * @brief Sends a PREVAC protocol message over the serial connection.
	 * @param msg The PREVAC protocol message to send.
	 * @return True if the message was successfully sent, False otherwise.
	 */
	bool sendMessage(prevac_msg_t const& msg);

	/**
	 * @brief Receives a PREVAC protocol message over the serial connection.
	 * @param msg Reference to a prevac_msg_t structure to store the received message.
	 * @return True if a message was successfully received, False otherwise.
	 */
	bool receiveMessage(prevac_msg_t& msg);
};
