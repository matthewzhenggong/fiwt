
Flight In Wind Tunnel
======================

This project aims to improve the software structure of Flight-In-Wind-Tunnel Rig System. Although it is designed to solve a specific problem, it is thought to be helpful to provide some demo code for embed MCU(especially for PIC serials) and common tools.
* Code for a Time-triggered Co-operative Scheduler [task.h](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/task.h)/[task.c](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/task.c). If the MCU do not supported by a FreeRTOS, as dsPIC33E we used.
* Demo code to create a [task](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/echoTask.c) using [ProtoThread](http://dunkels.com/adam/pt/) for the scheduler
* Code to design a [Serial Stream](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/SerialStream.h)([SerialStream.c](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/SerialStream.c)) on Hardware Abstruct Layer for embeded system
* XBee Zigbee API [library](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/XBeeZBS2.h) for embeded system in pure C language using [Serial Stream](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/SerialStream.h)
* Demo [code](http://github.com/matthewzhenggong/fiwt/blob/master/FlightInWindTunnel.X/UART1.c) to combine UART and DMA to privide minimal CPU load for serial IO. Features include:
    - Ping-pang mode double-buffer-swap design to flush TX data, no wait, no lock, one interrupt per flush
    - DMA buffer used as loop queue for RX data, no wait, no lock, no interrupt
* [XBee Range Test Tools](http://github.com/matthewzhenggong/fiwt/blob/master/XbeeZBS2Test/README.md)

