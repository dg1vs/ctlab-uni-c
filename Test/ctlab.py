#! /usr/bin/env python
# ctlab.py

import serial
import re


class ctlab(object):

    def __init__(self, serialport: object) -> object:

        self.ada = -1
        self.dcg = -1
        self.edl = -1
        self.dds = -1
        self.div = -1
        self.unic = -1

        self.ser = serial.Serial(serialport, 38400, timeout=0.3)
        self.ser.parity = 'N'
        self.ser.bytesize = 8
        self.ser.stopbits = 1

    def send_command_only(self, device, command):

        def xorstr(s):
            result = 0
            for c in s:
                result = result ^ ord(c)
            return result

        if self.ser.inWaiting() != 0:
            print(("ERROR: obsolete buffer content:", self.ser.readline().strip()))
        self.ser.flushInput()

        cmdstr1 = "%i:%s!" % (device, command)
        cmdstr2 = "%02x" % (xorstr(cmdstr1))
        cmdstr3 = "\r"
        cmdstr = cmdstr1 + cmdstr2 + cmdstr3

        self.ser.write(cmdstr.encode('utf-8'))

        #print("cmdstr =", cmdstr)
        return cmdstr

    def send_command(self, device, command):
        while True:
            scmd = self.send_command_only(device, command)
            # wait for result
            while self.ser.inWaiting() == 0:
                None
            answer = self.ser.readline().strip()
            resdev = re.search(r"#(.*?):", answer)
            if resdev != None:
                resdev = resdev.group(1)
            rescode = re.search(r"\[(.*?)\]", answer)
            if rescode != None:
                rescode = rescode.group(1)

            if rescode == "OK":
                return

            print(("ERROR: command (%s) answer: %s (device: %s, code %s)" % (scmd, answer, resdev, rescode)))

    def send_command_result(self, device, command):
        while True:
            cmdstr = self.send_command_only(device, command)
            while self.ser.inWaiting() == 0:
                None
            result = self.ser.readline().strip()
            # TODO
            #if result.find("7 [CHKSUM]") == -1:
            if result == cmdstr:
                return None
            return result.decode('utf-8')

    def read_value(self, device, commnad):
        result = None
        while result == None:
            value = self.send_command_result(device, commnad)
            # print ("command %s   result %s" % ( command, value))
            result = re.search(r"=(.*?)$", value)
        return float(result.group(1).strip())

    @staticmethod
    def get_device_type(value):
        result = re.search(r"\[(.*?)\s", value)
        if result != None:
            return result.group(1).strip()
        return result

    def get_device_options(self, value):
        result = re.search(r"\[.*;(.*?)\]", value)
        if result != None:
            return result.group(1).strip()
        return result

    def check_devices(self, verbose=True):

        if verbose:
            print("Scan for devices:")
        for index in range(9):
            result = self.send_command_result(index, "idn?")
            if result != None:
                devicetype = self.get_device_type(result)
                if verbose:
                    print(("Index %d: Device: %3s" % (index, devicetype)))
                    options = self.get_device_options(result)
                if verbose:
                    if options != None:
                        print(("    Options: %s" % options))
                    else:
                        print()
                # search for device indices
                if (devicetype == "ADA") and (self.ada < 0):
                    self.ada = index
                if (devicetype == "DCG") and (self.dcg < 0):
                    self.dcg = index
                if (devicetype == "DDS") and (self.dds < 0):
                    self.dds = index
                if (devicetype == "EDL") and (self.edl < 0):
                    self.edl = index
                if (devicetype == "UNIC") and (self.unic < 0):
                    self.unic = index

    def dcg_measure_power(self):
        return self.read_value(self.dcg, "msw?")

    def dcg_measure_voltage(self):
        return self.read_value(self.dcg, "msv?")

    def dcg_set_voltage(self, voltage):
        self.send_command(self.dcg, "dcv=%.3f" % voltage)

    def dcg_measure_current_ma(self):
        return self.read_value(self.dcg, "msa 1?")

    def dcg_set_current(self, current):
        self.send_command(self.dcg, "dca=%.3f" % (current))

    def dcg_set_current_ma(self, current):
        self.send_command(self.dcg, "dca 1=%.3f" % (current))

    def dds_set_frequency(self, frequency):
        self.send_command(self.dds, "frq=%.1f" % frequency)

    def dds_set_amplitude(self, amplitude):
        self.send_command(self.dds, "lvl=%.1f" % amplitude)

    def ada_set_da_voltage(self, pin, voltage):
        self.send_command(self.ada, "val%d=%.4f" % (20 + pin, frequency))

    def ada_measure_adcint(self, pin):
        return self.read_value(self.ada, "val %d?" % (0 + pin))

    def ada_measure_adc(self, pin):
        return self.read_value(self.ada, "val %d?" % (10 + pin))

    def ada_set_direction(self, port, value):
        self.send_command(self.ada, "dir%d=%d" % (port, value))

    def ada_set_port(self, port, value):
        self.send_command(self.ada, "pio%d=%d" % (port, value))

    def ada_get_port(self, port):
        return int(self.read_value(self.ada, "pio %d?" % port))
