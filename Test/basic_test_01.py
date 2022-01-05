#! /usr/bin/python

# Das ist mein erstes Programm

serialport = "com7"

import time

import ctlab

print("Test02")

lab = ctlab.ctlab(serialport)
lab.check_devices(verbose=True)

print("---------------------------------------")
result = lab.send_command_result(lab.unic, "idn?")
print(result)
print("---------------------------------------")

for index in range(256):
    cmd = "%i?" % index
    result = lab.send_command_result(lab.unic, cmd)
    print("%5d --> \t %s" % (index, result));
    time.sleep(0.1)

