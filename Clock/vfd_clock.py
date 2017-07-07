"""
a simple program that send the current system time to a serial VFD module

Reference:
=====================No Arrows====================
0x00  -  0  |  0x01  -  1  |  0x02  -  2  |  0x03  -  3  |  0x04  -  4
0x05  -  5  |  0x06  -  6  |  0x07  -  7  |  0x08  -  8  |  0x09  -  9
0x0a  -  A  |  0x0b  -  b  |  0x0c  -  C  |  0x0d  -  c  |  0x0e  -  d
0x0f  -  E  |  0x10  -  F  |  0x11  -  h  |  0x12  -  J  |  0x13  -  L
0x14  -  n  |  0x15  -  o  |  0x16  -  P  |  0x17  -  q  |  0x18  -  r
0x19  -  u  |  0x1a  -  y  |  0x1b -UpperA|  0x1c -LowerArrow
==================LowerArrowOnly==================
0x20  -  0  |  0x21  -  1  |  0x22  -  2  |  0x23  -  3  |  0x24  -  4
0x25  -  5  |  0x26  -  6  |  0x27  -  7  |  0x28  -  8  |  0x29  -  9
0x2a  -  A  |  0x2b  -  b  |  0x2c  -  C  |  0x2d  -  c  |  0x2e  -  d
0x2f  -  E  |  0x30  -  F  |  0x31  -  h  |  0x32  -  J  |  0x33  -  L
0x34  -  n  |  0x35  -  o  |  0x36  -  P  |  0x37  -  q  |  0x38  -  r
0x39  -  u  |  0x3a  -  y
==================UpperArrowOnly==================
0x40  -  0  |  0x41  -  1  |  0x42  -  2  |  0x43  -  3  |  0x44  -  4
0x45  -  5  |  0x46  -  6  |  0x47  -  7  |  0x48  -  8  |  0x49  -  9
0x4a  -  A  |  0x4b  -  b  |  0x4c  -  C  |  0x4d  -  c  |  0x4e  -  d
0x4f  -  E  |  0x50  -  F  |  0x51  -  h  |  0x52  -  J  |  0x53  -  L
0x54  -  n  |  0x55  -  o  |  0x56  -  P  |  0x57  -  q  |  0x58  -  r
0x59  -  u  |  0x5a  -  y
=======================Both=======================
0x60  -  0  |  0x61  -  1  |  0x62  -  2  |  0x63  -  3  |  0x64  -  4
0x65  -  5  |  0x66  -  6  |  0x67  -  7  |  0x68  -  8  |  0x69  -  9
0x6a  -  A  |  0x6b  -  b  |  0x6c  -  C  |  0x6d  -  c  |  0x6e  -  d
0x6f  -  E  |  0x70  -  F  |  0x71  -  h  |  0x72  -  J  |  0x73  -  L
0x74  -  n  |  0x75  -  o  |  0x76  -  P  |  0x77  -  q  |  0x78  -  r
0x79  -  u  |  0x7a  -  y
"""
import argparse
import serial
import sys
import time
from datetime import datetime


def open_serial(settings):
    port = settings[0]
    baudrate = settings[1]

    if 'linux' in sys.platform:
        port = '/dev/' + port
    ser = serial.Serial(port, baudrate)

    return ser


def change_display(data, mode):
    """Updates the display content."""
    for i in range(len(data)):
        data[i] = int(data[i])
    if mode < 1 or mode > 20:
        return data
    if mode < 11:  # if in modes 1-10
        if mode < 6:
            for i in range(mode):
                data[i] += 64
        else:
            mode -= 5
            for i in range(5):
                data[i] += 64
            for i in range(mode):
                data[(i + 1) * -1] += 32

    else:  # if in modes 11-20
        mode -= 10  # avoid getting list index out of range
        if mode < 6:
            for i in range(5):
                data[i] += 32
            for i in range(5 - mode):
                data[(i + 1) * -1] += 64
        else:
            mode -= 5
            for i in range(5 - mode):
                data[i] += 32

    return data


def send_data(current_time, ser, around):
    current_time = change_display(current_time, around)
    ser.write(b'\xff')
    ser.write(b'\x00')
    ser.write(b'\x00')
    ser.write(b'\xff')
    ser.write(display[current_time[0]])
    ser.write(display[current_time[1]])
    ser.write(display[current_time[2]])
    ser.write(display[current_time[3]])
    ser.write(display[current_time[4]])
    ser.write(b'\xff')


def get_time():
    current_time_raw = str(datetime.now())
    current_time_h1 = current_time_raw[11]
    current_time_h2 = current_time_raw[12]
    current_time_min1 = current_time_raw[14]
    current_time_min2 = current_time_raw[15]
    return [current_time_h1, current_time_h2, current_time_min1, current_time_min2]


def get_settings():
    parser = argparse.ArgumentParser(description='Control a VFD module to display the current time.')
    parser.add_argument('port', help='the serial port to use')
    parser.add_argument('--baudrate', help='baudrate of the serial port, default is 4800')
    args = parser.parse_args()
    try:
        baudrate = int(args.baudrate)
    except:
        if args.baudrate == None:
            baudrate = 4800
        else:
            print('Error: baudrate must be an integer!')
            parser.print_help()
            sys.exit(2)
    port = args.port
    return port, baudrate


def main():
    settings = get_settings()
    ser = open_serial(settings)
    _middle = 1
    around = 0
    middle = 1

    while True:
        current_time = get_time()
        send_data((current_time[:2] + [middle] + current_time[2:]), ser, around)
        time.sleep(0.99)

        _middle = _middle * -1
        if _middle == 1:
            middle = 1
        if _middle == -1:
            middle = 12

        around = around + 1
        if around == 21:
            around = 0


display = (b'\x00', b'\x01', b'\x02', b'\x03',
           b'\x04', b'\x05', b'\x06', b'\x07',
           b'\x08', b'\x09', b'\x0a', b'\x0b',
           b'\x0c', b'\x0d', b'\x0e', b'\x0f',
           b'\x10', b'\x11', b'\x12', b'\x13',
           b'\x14', b'\x15', b'\x16', b'\x17',
           b'\x18', b'\x19', b'\x1a', b'\xff',
           b'\xff', b'\xff', b'\xff', b'\xff',

           b'\x20', b'\x21', b'\x22', b'\x23',
           b'\x24', b'\x25', b'\x26', b'\x27',
           b'\x28', b'\x29', b'\x2a', b'\x2b',
           b'\x2c', b'\x2d', b'\x2e', b'\x2f',
           b'\x30', b'\x31', b'\x32', b'\x33',
           b'\x34', b'\x35', b'\x36', b'\x37',
           b'\x38', b'\x39', b'\x3a', b'\xff',
           b'\xff', b'\xff', b'\xff', b'\xff',

           b'\x40', b'\x41', b'\x42', b'\x43',
           b'\x44', b'\x45', b'\x46', b'\x47',
           b'\x48', b'\x49', b'\x4a', b'\x4b',
           b'\x4c', b'\x4d', b'\x4e', b'\x4f',
           b'\x50', b'\x51', b'\x52', b'\x53',
           b'\x54', b'\x55', b'\x56', b'\x57',
           b'\x58', b'\x59', b'\x5a', b'\xff',
           b'\xff', b'\xff', b'\xff', b'\xff',

           b'\x60', b'\x61', b'\x62', b'\x63',
           b'\x64', b'\x65', b'\x66', b'\x67',
           b'\x68', b'\x69', b'\x6a', b'\x6b',
           b'\x6c', b'\x6d', b'\x6e', b'\x6f',
           b'\x70', b'\x71', b'\x72', b'\x73',
           b'\x74', b'\x75', b'\x76', b'\x77',
           b'\x78', b'\x79', b'\x7a', b'\xff',
           b'\xff', b'\xff', b'\xff', b'\xff')

main()
