The goal of this project is to define a standard Application Programming Interface (API) for access to the CAN bus. The API provides functionality for ISO/OSI Layer-2 (Data Link Layer). It is not the intention of CANpie to incorporate higher layer functionality (e.g. CANopen, J1939). The API covers support for classic CAN frames as well as for ISO CAN FD frames.

Where ever it is possible CANpie is independent from the used hardware and operating system. The function calls are unique for different kinds of hardware. Also CANpie provides a method to gather information about the features of the CAN hardware. This is especially important for an application designer, who wants to write the code only once.

The API is designed for embedded control applications as well as for PC interface boards.