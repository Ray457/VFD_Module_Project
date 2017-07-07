# VFD_Module_Project
Source of a DIY vacuum fluorescent display module, and a python script to show current time

Running as a clock, with the python script running on a Raspberry Pi:
![VFD Clock](/photos/running_module.jpg?raw=true)

VFD pinout info:
http://www.crystalradio.cn/thread-455661-1-1.html

I drew the schmatics for the module on a paper and couldn't find it anymore.. 
Filament supply is 2.5V DC and cathode voltage is 17V. It seems alright to drive the filament with DC here.
Anodes are driven by individual transistors.

I used a Chinese 8051-based microcontroller - http://www.stcmicro.com/index.html - with part number STC12C5204AD.
The C code probably also runs on other micros but some changes may need to be made:
* include line
* the value 'i < 600' in 'delay()', as the clock frequency may differ
* UART code
* GPIO code
