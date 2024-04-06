# Prevac Protocol Handler

This project provides a comprehensive solution for handling communications using the PREVAC protocol, specifically tailored for the TM13/TM14 Thickness Monitor devices. It includes utilities for safe buffer manipulation, serial communication setup, message construction, parsing, and printing functionalities.

## Features

- **Safe Buffer Operations**: Utilizes `safeCopyFromBuffer` for error-checked data copying, ensuring data integrity during buffer operations.
- **PREVAC Message Handling**: Defines a `prevac_msg_t` structure for encapsulating PREVAC protocol messages, including methods for setting data, calculating CRC, and printing message details.
- **Serial Communication**: Manages serial port connections, data transmission, and reception through the `PrevacSerial` class, with support for setting connection parameters as defined in the TM13/TM14 Thickness Monitor user manual.

## Getting Started

### Prerequisites

- Windows environment (due to the use of Windows-specific serial communication handling).
- Microsoft Visual Studio or a compatible C++ compiler that supports C++17 (for `std::string_view`, etc.).

### Building the Project

1. Clone the repository to your local machine.
2. Open the project in Microsoft Visual Studio or your preferred development environment that supports Windows-specific development.
3. Ensure the Boost library paths are correctly set up if the project requires it (not applicable to the provided code directly).
4. Build the project to produce the executable.

### Running the Application

To run the application, navigate to the directory containing the built executable and run it through the command line or by double-clicking the executable file. Modify `main.cpp` to specify the correct COM port and other parameters based on your setup.

## Usage

The project can be used to establish communication with TM13/TM14 Thickness Monitor devices, send commands, and read responses. Here's a brief overview of how to use the main functionalities:

1. **Setting up Serial Communication**:
    ```cpp
    PrevacSerial serial;
    if (serial.establishConnection("COM4"))
    {
        std::cout << "Serial port COM4 opened successfully\n";
    }
    ```

2. **Creating and Sending a PREVAC Message**:
    ```cpp
    prevac_msg_t msg;
    msg.setData("YourDataHere");
    if (serial.sendMessage(msg))
    {
        std::cout << "Message sent successfully\n";
    }
    ```

3. **Receiving and Processing a PREVAC Message**:
    ```cpp
    prevac_msg_t receivedMsg;
    if (serial.receiveMessage(receivedMsg))
    {
        receivedMsg.printDetailed();
        // Further processing...
    }
    ```

## Contributing

Contributions to this project are welcome. Please feel free to fork the repository, make changes, and submit pull requests.
