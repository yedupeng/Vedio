from loguru import logger
from utils.SplitInt import get_gigh_low_data


def get_keyboard_input():
    points_str = input()
    logger.info('Keyboard Inputs:   {}'.format(points_str))
    points = list(map(eval, points_str.split(' ')))  
    return points

def transmit_keyboard_msg():
    keyboard_input = get_keyboard_input()
    points_split = []
    for data in keyboard_input:
        points_split.extend(get_gigh_low_data(data))
    return tuple(points_split)

def listen_keyboard():
    pass
