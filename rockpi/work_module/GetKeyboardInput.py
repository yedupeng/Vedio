from loguru import logger
import serial


def get_keyboard_input():
    points_str = input()
    logger.info('Keyboard Inputs:   {}'.format(points_str))
    points = list(map(eval, points_str.split(' ')))  
    return points

def transmit_keyboard_msg():
    keyboard_input = get_keyboard_input()
    points_split = []
    for i in keyboard_input:
        temp_h = i >> 8
        points_split.append(temp_h)
        points_split.append(i - (temp_h << 8))
    return tuple(points_split)


if __name__ == '__main__':
    msg = transmit_keyboard_msg()