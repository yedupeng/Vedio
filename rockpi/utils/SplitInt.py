def get_gigh_low_data(data):
    if data == 0:
        return (0, 0)
    temp_h = data >> 8
    temp_l = data - (temp_h << 8)
    return (temp_h, temp_l)