#!/usr/bin/env python
import smbus
import time

bus = smbus.SMBus(1)
address = 0x27

while (1):
	ADC = {};
	for i in range(0, 8):
		LBS = bus.read_byte_data(address, 0x00+i*2)
		MBS = bus.read_byte_data(address, 0x00+i*2+1)
		ADC[i] = MBS*256 + LBS

	print ADC
	time.sleep(0.2)
