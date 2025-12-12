#!/usr/bin/env python3

import sys
import os
import time
import argparse
import platform

def check_dependencies():
    missing_deps = []

    try:
        import serial
    except ImportError:
        missing_deps.append("pyserial - serial communication lib")

    try:
        import serial.tools.list_ports
    except ImportError:
        missing_deps.append("pyserial tool module")

    try:
        import crcmod
    except ImportError:
        try:
            import binascii
        except ImportError:
            missing_deps.append("crcmod or binascii - crc calculate module")

    if missing_deps:
        print("error: lack of necessary Python components:")
        for dep in missing_deps:
            print(f"  - {dep}")
        print("\nPlease use the following command to install:")
        if platform.system() == "Windows":
            print("pip install pyserial crcmod")
        else:
            print("sudo pip3 install pyserial crcmod")
        return False

    return True

class YModemSender:

    SOH = 0x01
    STX = 0x02
    EOT = 0x04
    ACK = 0x06
    NAK = 0x15
    CAN = 0x18
    CRC = 0x43
    identify_flag = 0

    def __init__(self, port, baudrate=115200, timeout=3, verbose=False):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.verbose = verbose
        self.serial = None
        self.packet_size = 1024  #  default

        self._init_crc16()

    def _init_crc16(self):
        try:
            import crcmod
            self.crc16 = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0x0000, xorOut=0x0000)
            self.use_crcmod = True
        except ImportError:
            self.use_crcmod = False
            if self.verbose:
                print("use inner crc method")

    def calculate_crc16(self, data):
        if self.use_crcmod:
            return self.crc16(data)
        else:
            crc = 0x0000
            for byte in data:
                crc ^= byte << 8
                for _ in range(8):
                    if crc & 0x8000:
                        crc = (crc << 1) ^ 0x1021
                    else:
                        crc <<= 1
                    crc &= 0xFFFF
            return crc

    def open_serial(self):
        try:
            import serial
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=self.timeout
            )
            return True
        except Exception as e:
            print(f"can not open serial {self.port}: {e}")
            return False

    def close_serial(self):
        if self.serial and self.serial.is_open:
            self.serial.close()

    def send_byte(self, byte):
        self.serial.write(bytes([byte]))

    def receive_byte(self, timeout=None):
        old_timeout = self.serial.timeout
        if timeout is not None:
            self.serial.timeout = timeout

        data = self.serial.read(1)

        if timeout is not None:
            self.serial.timeout = old_timeout

        return data[0] if data else None

    def wait_for_signal(self, signal, timeout=10):
        start_time = time.time()
        while time.time() - start_time < timeout:
            received = self.receive_byte(timeout=1)
            if received == signal:
                return True
            elif received is not None and self.verbose:
                print(f"receive unexpected signal : 0x{received:02X}")
        return False

    def send_reboot(self):
        reboot_msg="reboot\r\n"
        self.serial.write(reboot_msg.encode('utf-8'))
        return True

    def send_packet(self, packet_num, data, is_header=False):
        start_byte = self.STX if len(data) > 128 else self.SOH

        packet_num_byte = packet_num % 256
        packet_num_complement = 255 - packet_num_byte

        packet = bytearray()
        packet.append(start_byte)
        packet.append(packet_num_byte)
        packet.append(packet_num_complement)

        if start_byte == self.STX:
            data_length = 1024
        else:
            data_length = 128

        packet.extend(data)
        if is_header == True:
            packet.extend([0x00] * (data_length - len(data)))
        else:
            packet.extend([0x1A] * (data_length - len(data)))

        # calculate crc
        data_for_crc = packet[3:]
        crc = self.calculate_crc16(data_for_crc)
        packet.append((crc >> 8) & 0xFF)
        packet.append(crc & 0xFF)

        # send data
        self.serial.write(packet)

        if self.verbose:
            print(f"send {packet_num}, len: {len(data)} byte")

        # wait for ack
        response = self.receive_byte(timeout=10)
        if response == self.ACK:
            return True
        elif response == self.NAK:
            if self.verbose:
                print(f"datapack {packet_num} rejected, retry...")
            return False
        elif response == self.CAN:
            print("canceled by receiver")
            return None
        else:
            if self.verbose:
                print(f"unknown ack: {response}")
            return False

    def send_file(self, filename, counter):
        if not os.path.exists(filename):
            print(f"err: file '{filename}' not exist")
            return False

        file_size = os.path.getsize(filename)
        file_basename = os.path.basename(filename)

        if counter == 0:
            print(f"ready to send: {file_basename} ({file_size} byte)")

        print(f"wait receiver ready signal {counter}...")
        if not self.wait_for_signal(self.CRC, timeout=2):
            identify_flag = 0
            return False

        identify_flag = 1
        print("start ymodem transmission...")

        # send head
        header_data = f"{file_basename}\0{file_size}\0".encode('utf-8')
        if not self.send_packet(0, header_data, is_header=True):
            print("err: fail to send fileheader")
            return False

        # wait init 0x43 C
        if self.wait_for_signal(self.CRC, timeout=10):
            # print("")
            pass
        else:
            print("err: no get init C")
            return False

        # send file
        packet_num = 1
        with open(filename, 'rb') as file:
            while True:
                data = file.read(self.packet_size)
                if not data:
                    break

                # retry
                max_retries = 5
                for retry in range(max_retries):
                    result = self.send_packet(packet_num, data)
                    if result is True:
                        break
                    elif result is None:
                        return False
                    elif retry == max_retries - 1:
                        print(f"err: pack {packet_num} fail to send, reach to max retry times")
                        return False
                    else:
                        time.sleep(1)

                packet_num += 1
                if packet_num > 255:
                    packet_num = 1

                # show process
                progress = min(file.tell() / file_size * 100, 100)
                print(f"\rprogress: {progress:.1f}%", end='', flush=True)

        print("\nsend end signal...")

        # send EOT
        self.send_byte(self.EOT)

        # wait final ACK
        if self.wait_for_signal(self.NAK, timeout=10):
            # print("")
            pass
        else:
            print("err: fail to get NAK")
            return False

        # send EOT
        self.send_byte(self.EOT)

        # wait ACK
        if self.wait_for_signal(self.ACK, timeout=10):
            # print("")
            pass
        else:
            print("err: fail to get ACK")
            return False

        # wait 0x43 C
        if self.wait_for_signal(self.CRC, timeout=10):
            # print("")
            pass
        else:
            print("err: fail to get C")
            return False

        enddata=[0]*128
        result = self.send_packet(0, enddata)
        if result is True:
            print("get Final ACK!")
            return True
        elif result is None:
            print("err: fail to get Final ACK")
            return False


def list_serial_ports():
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()

        if not ports:
            print("can not find usable serial port")
            return

        print("serial port available:")
        for port in ports:
            print(f"  {port.device} - {port.description}")
    except ImportError:
        print("can't list serial port (pyserial not installed)")

def main():
    parser = argparse.ArgumentParser(
        description="YMODEM file send tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
example:
  %(prog)s COM1 firmware.bin
  %(prog)s /dev/ttyUSB0 image.bin -b 9600
  %(prog)s --list-ports
        """
    )

    parser.add_argument('port', nargs='?', help='serial port (COM1, /dev/ttyUSB0)')
    parser.add_argument('file', nargs='?', help='file will be sended')
    parser.add_argument('-b', '--baudrate', type=int, default=115200, 
                       help='baudrate (default: 115200)')
    parser.add_argument('-t', '--timeout', type=int, default=3,
                       help='timeout(s) (default: 3s)')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='debug message')
    parser.add_argument('--list-ports', action='store_true',
                       help='list all serial port available')

    args = parser.parse_args()

    if not check_dependencies():
        sys.exit(1)

    if args.list_ports:
        list_serial_ports()
        sys.exit(0)

    if not args.port or not args.file:
        parser.print_help()
        print(f"\nerr: port and file must be specified")
        sys.exit(1)

    if not os.path.exists(args.file):
        print(f"err: file '{args.file}' not exist")
        sys.exit(1)

    sender = YModemSender(
        port=args.port,
        baudrate=args.baudrate,
        timeout=args.timeout,
        verbose=args.verbose
    )

    try:
        if sender.open_serial():
            count_try_link = 0
            while count_try_link < 20:
                success = sender.send_file(args.file, count_try_link)
                if (success == False and sender.identify_flag == 0):
                    sender.send_reboot()
                    time.sleep(0.25)
                    sender.close_serial()
                    time.sleep(1.3)
                    sender.open_serial()
                    count_try_link=count_try_link+1;
                    continue
                else:
                    break
            sys.exit(0 if success else 1)
        else:
            sys.exit(1)
    except KeyboardInterrupt:
        print("\ntransmission interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"err: {e}")
        sys.exit(1)
    finally:
        sender.close_serial()

if __name__ == "__main__":
    main()
