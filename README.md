usbfmtx
=======

Linux program to configure/control a USB FM transmitter dongle i.e Keene USB FM Transmitter.

It uses the Qt gui to provide a simple interface that allows all the accessible parameters to
be changed. 

The program was built using QT Creator but you can build it without.

I am not an expert at c++ or QT so some of the code may be naive and less than elegant but
I wanted it easy to read and it works.

Dependencies
------------

I am not sure what are classed as standard libraries on various distributions but you will
need the following:

libudev


Installation
------------

Get the latest code from https://github.com/martst/usbfmtx

If you are using QT Creator load usbfmtx.pro and then build

Otherwise in a terminal in the usbfxtx directory

qmake usbfmtx.pro
make

To execute the program

./usbfmtx   


Usage
-----

Plug your usb fm tx dongle in first. Run the program and the top right box will show
the internal device name of the dongle. Press on slide the buttons as you wish.


Credits and license
-------------------

Usbfmtx is designed and written by Martin Stubbs, and it is licensed under the 
GNU General Public License.

Many of the ideas, techniques and routines have been copied from elsewhere and 
these are acknowledged as appropriate. 