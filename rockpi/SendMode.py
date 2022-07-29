from dataclasses import dataclass
from communite_module.Communications import SelfSerial
from loguru import logger


def sum_check(data_list):
    data_sum = 0
    for temp in data_list:
        data_sum = temp+data_sum
    return data_sum % 256

if __name__ == '__main__':
    self_serial = SelfSerial('COM7')
    logger.info('System Starting')
    while True:
        mode = int(input('Input Run Mode:'))
        msg = [0x0f, 0xf0, 0x20, 0x02]
        msg.append(mode)
        msg.append(sum_check(msg))
        self_serial.uart.write(bytearray(msg))
