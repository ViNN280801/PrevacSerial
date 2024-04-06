#include <iostream>

#include "PrevacSerial.h"
#include "Utilities.h"

#define COM_PORT "COM4"

int main() {
	prevac_msg_t msg;
	msg.setData("Helloabrkjlnrejvrgr;toglrjkas;efjkrelghrgehfg34hifug4o5pj4rnjgvelsfkal;ewfhregj;fjkerjfksrjlfk;awfnkrejlgnksandfkjndsljfndljfnlfl;ksnfkjanfljkajglkdnsakgjslkfjmdsnkfkssafljsghiw4oh4iugopiaqojfoi34jgjr4ouhgtjgpoeks;fglajwefiJAOUIJEDIOPW#$pfjoifgoi4jgpjepij");

	msg.print();
	msg.printDetailed();
	msg.printDataAsString();

	PrevacSerial serial;
	if (serial.establishConnection(COM_PORT))
	{
		std::cout << "Serial port " << COM_PORT << " opened successfully\n";

		// Preparing message.
		prevac_msg_t msg;
		msg.print();
		msg.printDetailed();

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
