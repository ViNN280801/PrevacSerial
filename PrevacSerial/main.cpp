#include <iostream>

#include "PrevacSerial.h"
#include "Utilities.h"

#define COM_PORT "COM3"

int main()
{
	PrevacSerial serial;
	if (serial.establishConnection(COM_PORT))
	{
		std::cout << "Serial port " << COM_PORT << " opened successfully\n";

		// Preparing message.
		prevac_msg_t msg;
		msg.setMessage("Hello, World!");
		msg.print();

		// Send the message.
		if (serial.sendMessage(msg))
			std::cout << "Message sent successfully\n";
		else
			std::cerr << "Failed to send message\n";

		// Example: Receiving a message.
		prevac_msg_t receivedMsg;
		if (serial.receiveMessage(receivedMsg))
		{
			std::cout << "Message received successfully\n";
			// Process receivedMsg...
		}
		else
			std::cerr << "Failed to receive message\n";
	}
	else
	{
		std::cerr << "Failed to open serial port " << COM_PORT << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
