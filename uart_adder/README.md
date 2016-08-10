UART Adder
==========

This example sets out to have a bit of compute outsourced to the FPGA.
It reuses the UART of uart_echo to both feed and retrieve data to/from
the FPGA. The operation is a mere addition of two 8 bit characters.

The 'uart_adder' example was prepared by Steffen Möller and Ruben
Undheim as a donation to the ice40 example collection of Paul Martin
under the same current or future license as the reminder of his code base.
