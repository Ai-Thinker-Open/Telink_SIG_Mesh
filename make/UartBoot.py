#coding=utf-8
#!/usr/bin/env python

### ComSwireWriter.py ###
###    Autor: pvvx    ###
###    Edited: Aaron Christophel ATCnetz.de    ###
###    Edit : Pila    ###

import sys
import signal
import struct
import serial
import platform
import time
import argparse
import os
import io
import serial.tools.list_ports
import base64

boot_bin = base64.b64decode(b"""
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
EpgQSsAMH0gAGs9MEIqQDBlIEaqQDBJIIDqQDBqYH8owChvIeBo1vwnAIAwcqAgqNb8JwCAMG9gD8LnALbwRepAMF4gQOpAMHVgQ2pAMGxgfyjA
KGjhzkLnAIAweiAgaOb8JwCAMHXgDULnALDwQSpAMEwgR6pAMHkgQOpAMF+gfyjAKGLh9Cjm/CcAh7A4KOb8JwCr8EL7xv2G/4BqwDIXoEGqQDB
tYEDqQDB6YA/owChdIcUqQDBHIEMqQDBr4EaqQDBT4HzowChaIcFqQDB8oADqQDBrIENqQDBRoHPowChXIcJqQDB+IAL7xv2G/4BqwDIu4EDqQD
BYYHPowChTYcg7AGh/5c1nv+jAKFGhwmpAMHWgBmpAMGQgQypAMEzgfOjAKE6h4YFgACoBYAAAgMAAAEDAAABAQAAAgIAAAECAAAaqQDBs4ADqQ
DBrYA/owChI4cL7xv2G/4BqwDI+4ADqQDBQIEVqQDBHIE/owChFIcVqQDBs4ADqQDBVYEEqQDBBIE/owChCIcFqQDBqoAGqQDBXoEXqQDB+4DPo
wCh/IYMqQDBj4ADqXLA86MAofSGGakAwbeAA6lkwM+jAKHshhipAMGpgAOpAMEegQ2pAMEAgfOjAKHghhupbsAGqQDBKIEDqQDB74A/owCh1YYF
qQDBj4ADqQDBBIEUqQDB1IDPowChyYYMqWDAA6lDwPyjAKHChhSpAMGRgAOpAMEhgQSpAMHNgM+jAKG2hgSpAMGLgBupAMHigBSpAMHNgPOjAKG
qhg2pXMAeqSfAz6MAoaOGCKlkwAOpAMHHgAapAMG+gPOjAKGYhh2pR8ADqQDB6YANqQDBj4DzowChjYY/o4ChiobPoxChh4Y/o0ChhIbzowihgY
b8owKhfobPoyChe4Y/o4CheIY/o0ChdYY/owChcobPowChb4Y/owChbIbzowChaYbzowShZob8owGhY4b8owChYIbPowChXYY/owChWobPowChV
4b8owChVIbzowChUYb8owChTobzowChS4bPoxChSIY/o0ChRYY/owChQobPowChP4bzowChPIbzowChOYbPowChNoY/owChM4b8owChMIb8owCh
LYbPowChKob8owChJ4bzowChJIb8owChIYY/owChHoY/owChG4bzowihGIbPoyChFYb8owKhEob8owKhD4b8owKhDIY/o4ChCYbzowihBoY/o4C
hA4bPoyChAIbzowih/YX8owKh+oU/o4Ch94XPoyCh9IX8owKh8YX8owKh7oXPoyCh64XPoyCh6IU/o4Ch5YX8owKh4oXzowih34Xzowih3IXzow
ih2YU/o0Ch1oXzowSh04X8owGh0IX8owGhzYXzowShyoXPoxChx4XzowShxIU/o0ChwYX8owGhvoXzowShu4X8owGhuIU/o0ChtYXPoxChsoU/o
0Chr4XzowShrIX8owGhqYXzowShpoU/o0Cho4XPoxChoIX8owGhnYX8owGhmoXPoxChl4XPoxChlIXwZQD0APwN9i3+EaMDAibBIqMDAhrAAqbz
pwP6W/DwpCAARO6gARzoDrQk9iT+IOz/l4CaA6ENACnssQAHAA8DOfYJ/iDs/5eVmvBtRKMDAgjBiKMDAvjABqY/p92HAKb8p9qHBKbPp9eHwEY
QZQD0APwJ9gn+EvYS/hv2G/4cDCFAHAwEAyT0JPwbCAQgAbFI8EHoifYJ/hgIAUABoBgJCEAAqhHABLkMSASgIAMA9gD+CEABqhbAAqoLwQhICK
ICAxL2Ev4KQASADgoRSASggQMRQAsKEUgwoIEDEUARSAsDE0AQbQpICKCCAwpA8YeWAIAAAID//5QAgACaAIAAmwCAAJcAgAAA9gn2Cf4AqA7AD
wsYSBCiAgMS9hL+GkAAqQzBCgsaSCChigMaQHAHBwsaSBCgggMaQACp8sAECxlIIKIKAxL2Ev4aQO+HwEaWAIAAEGUA9gD+CfYJ/gCoGMAWCxxI
QKIiAxL2Ev4aQACpFsARChRIgKNbAiMDG/Yb/hNAAQMTwQ0LGlhAoYoDGlAQbQkLGkhApKIDGkAAqejBBgsaSH+kIgAaQAED68ADCxlYQKIKAxp
Q6oeWAIAAQAaAAAkLGkgAo9H3C8UA9AD8BwsYICCzGUgCogoDEvYS/hpAAaMY7HAHwEaeAIAABAyAAAn1Cf4A9AD8BQsYIASiQLMaQD67GUABog
GzGkBwB8BGAAyAADBlBfQt/Az0JPwo7AOh/5frniDsA6H/l+eeKOwDof+XwZsg7AOh/5e9myjsAaH/l1ubIOwBof+XV5swbXBlTQiIoUnw/5fKn
wGgAKH/l6qbSQgAof+XppvAoEDwAKH/l6GbRgsZSAGiEQMJ9gn+GUAIsxlICgMS9hL+GkAZSICiUgIKAxL2Ev4aQAGgAaH/lyubOQgBof+XJ5vA
oEDwAaH/lyKbAaACof+Xnp4yCAKh/5eansCgQPACof+XlZ4wCAGjMAqApAFICwJOwRFICwJNwRFIDAL2wMChSfCBoEDw/5eOnygLGUgEogoDEvY
S/hpAGkgEoYoDGkABoAyhAKIAo/+Xp54BoAGh/5fxnh8LGVgQogoDGlADogOhHAsZQAa7GUgRAwn2Cf4ZQAGzGUgKAxL2Ev4aQACgAKH/l/6eFA
wh7RQKC+xh2mHTI9oj0xijI1Ag7P+XKZ8QCQGiC0gaAvzAAKMjUHBtAaG2hwIJtIfARhwehAABAQAAggWAAIAFgACIBYAAYACAAEAGgAAmDIAAD
B2EAKgTAACeAIAA8GVUDStYAKsNwFMLGUgCogoDEvYS/hpAUAkBogtIGgL8wACjK1BODCNYAKsPwGJJEvKhSVLoA7uaAgnASQsaWGpQmygrIQaj
K1AAoyNQ8G0jSQSr+cib8EMK0xifBuBJAPIjSsDoAPJjSsDoAPKjSsDoPg4wUAP1AKtpweNKAKsLwACn/5cSmTBYgKNb8cDoMFABt+NKuwL0zCr
tNAsZWGlQmSiRIJtJk0EHoytQzIfgSQDyI0rA6ADyY0rA6ADyo0rA6CkLGFDpSirsAtL/lzuZIQsZSAKiCgMS9hL+GkAeCQGiC0gaAvzAKu0hC9
aH4EkA8iNKwOgA8mNKwOgA8qNKwOgZCxhQGwuYAiDIKu0aCxlYaVCZKJEgm0mTQQejK1Ai7AyygKFJ8P+X35iMhyrtEwuzhyjt/5dCmRELGygb9
Br8G/7rQSpCBaMrUHyHDQt0hwwdhAAkDIAAngCAABwehADEEwAAlBMAADgfhADsEwAA5BMAAP//BwDUEwAAzBMAAH4AgADcEwAAAGUAkK2YQqD/
l3aZgKAAof+XVJ2AoAKh/5cumv+XeJ7/lyif/IfARgAAAAAAAAAAAWQCZAIIaQYBUAJsAWxwB3gfhABwB8BGwEbARsBGwEYDC2KiGkADChBYAKI
aQHAHdACAAEwAgAAwZQCQ6Zj/l1eZ/5ftn0T/LgscQMCjAwAApcCrNcAr7CMDG8EqChNIAKsEwSkLGlgpCxpQMG0pCQlYCeOJ8AOgAwALAxv2G/
4TQKWhJAoRQAGyE0AAoiALGlDrh/+XtJ8gCxlYAK0kwAvsZLv/opMDZLGRA4sC1cIAoICkJPQa6RBQELOZAvjIzIcBpcL1xsQVC5pI2UgJ8gClE
QO/wD+iEACC8BDoEQqA6BhAAPpYQLWHAqwFwASstMENC5kCscnShwwLmQLPyKyHwEYsH4QAPgaAAAQdhAAwH4QACB2EAAQAhAB4H4QAAB2EAPcD
AAAAwIQAAICEAABlAKMtChNAAbITQAGyE0D/owGyE0ABshNAAbITQIKgZKH+l1WfNKCAof6XUZ8LoDih/pdNn4ygAqH+l0mfAqCiof6XRZ8noAC
h/pdBnyigAKH+lz2fKaAAof6XOZ8qoACh/pc1nxYLFgoTUASyE1AEohULGkD6o1vxFAoTIBQLG0gBqxXAAaBMof6XIZ8BohALGkD/lzifAKMPCh
NAAbITQA4LGUgMogoDEvYS/hpAAG0BoDyh/pcLn+iHYACAAAQEBARADIAASAyAAFAHgAB9AIAATweAACAMgAC1BYAAEGUbCyCkHEAAohpAgqD+l
8+eQKMB7BkDCfYJ/oKg/pfnnvSgBaH+l+Oe76Aoof6X357woACh/pfbnvGgAKH+l9ee8qAIof6X054EoACh/5cPmP6gEKH+l8ue/KD+l6ieAeyh
Awn2Cf78oP6XwZ4QbWEAgACEEAAARBAAAAQQAACyDwAAihAAACAgICAgDQp1YXJ0X2Jvb3QgcmVhZHkNCgAAAABFcnJvDQoAAFIwLjAxDQoAT0t
fMDENCgBGYWlsDQoAAE9LXzAyDQoAT0tfMDMNCgCXBAEAAAACAADwAQCEJXiW
""")

def sws_encode_blk(blk):
	pkt=[]
	d = bytearray([0xe8,0xef,0xef,0xef,0xef])
	for el in blk:
		if (el & 0x80) != 0:
			d[0] &= 0x0f
		if (el & 0x40) != 0:
			d[1] &= 0xe8
		if (el & 0x20) != 0:
			d[1] &= 0x0f
		if (el & 0x10) != 0:
			d[2] &= 0xe8
		if (el & 0x08) != 0:
			d[2] &= 0x0f
		if (el & 0x04) != 0:
			d[3] &= 0xe8
		if (el & 0x02) != 0:
			d[3] &= 0x0f
		if (el & 0x01) != 0:
			d[4] &= 0xe8
		pkt += d 
		d = bytearray([0xef,0xef,0xef,0xef,0xef])
	return pkt

# encode command stop into 10-bit swire words
def sws_code_end():
	return sws_encode_blk([0xff])
# encode the command for writing data into 10-bit swire words
def sws_wr_addr(addr, data):
	return sws_encode_blk(bytearray([0x5a, (addr>>16)&0xff, (addr>>8)&0xff, addr & 0xff, 0x00]) + bytearray(data)) + sws_encode_blk([0xff])
# send block to USB-COM
def wr_usbcom_blk(serialPort, blk):
	return serialPort.write(blk)
# send and receive block to USB-COM
def	rd_wr_usbcom_blk(serialPort, blk):
	i = wr_usbcom_blk(serialPort, blk)
	return i == len(serialPort.read(i))
# send swire command write to USB-COM
def sws_wr_addr_usbcom(serialPort, addr, data):
	return wr_usbcom_blk(serialPort, sws_wr_addr(addr, data))
# send and receive swire command write to USB-COM


def activate(serialPort, tact_ms):

	serialPort.setDTR(True)
	serialPort.setRTS(True)
	time.sleep(0.05)
	serialPort.setDTR(False)
	serialPort.setRTS(False)
	# Stop CPU|: [0x0602]=5
	sws_wr_addr_usbcom(serialPort, 0x06f, bytearray([0x20])) # soft reset mcu
	blk = sws_wr_addr(0x0602, bytearray([0x05]))
	if tact_ms > 0:
		tact = tact_ms/1000.0
		t1 = time.time()
		while time.time()-t1 < tact:
			for i in range(5):
				wr_usbcom_blk(serialPort, blk)
			serialPort.reset_input_buffer()
	#--------------------------------
	# Duplication with syncronization
	time.sleep(0.01)
	serialPort.reset_input_buffer()
	rd_wr_usbcom_blk(serialPort, sws_code_end())
	rd_wr_usbcom_blk(serialPort, blk)
	time.sleep(0.01)
	serialPort.reset_input_buffer()

def load_ram(_port):
	size = len(boot_bin)
	addr = 0x40000
	bin_addr = 0
	while size > 0:
		data = boot_bin[bin_addr:bin_addr+0x100]
		if not len(data) > 0: # end of stream
			break
		_port.write(sws_wr_addr(addr, data))
		_port.reset_input_buffer()
		bin_addr += len(data)
		addr += len(data)
		size -= len(data)

	# print('\rBin bytes writen:', bin_addr)		
	# print('CPU go Start...')
	_port.write(sws_wr_addr(0x0602, b'\x88')) # cpu go Start
	time.sleep(0.07)
	_port.flushInput()
	_port.flushOutput()
	_port.reset_input_buffer()
	_port.reset_output_buffer()
	time.sleep(0.07)
	return True

def uart_boot(serialPort):
	activate(serialPort, 80)
	return load_ram(serialPort)
