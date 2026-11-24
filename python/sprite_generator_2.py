import PIL
from PIL import Image

IMAGE_LOCATION = "./python/sprites.png"
sprites_img = Image.open(IMAGE_LOCATION)

out_path = "./python/sprites.c"

def get_sprite(inx: int):
    box = ((inx % 16) * 4, (inx // 16) * 4, (inx % 16) * 4 + 4, (inx // 16) * 4 + 4)
    return sprites_img.crop(box)

def get_sprite_coor(spr, x, y):
    pixel = spr.getpixel((x, y))
    if(pixel[0] == 0):
        return 0
    else:
        return 1

def get_sprite_bytes(spr):

    char_list1 = list("XXXXXXXX")
    char_list2 = list("XXXXXXXX")

    char_list1[7- (0)] = str(get_sprite_coor(spr, 0, 0))
    char_list1[7- (1)] = str(get_sprite_coor(spr, 1, 0))
    char_list1[7- (2)] = str(get_sprite_coor(spr, 2, 0))
    char_list1[7- (3)] = str(get_sprite_coor(spr, 3, 0))
    char_list1[7- (4)] = str(get_sprite_coor(spr, 0, 1))
    char_list1[7- (5)] = str(get_sprite_coor(spr, 1, 1))
    char_list1[7- (6)] = str(get_sprite_coor(spr, 2, 1))
    char_list1[7- (7)] = str(get_sprite_coor(spr, 3, 1))

    char_list2[7- (0)] = str(get_sprite_coor(spr, 0, 2))
    char_list2[7- (1)] = str(get_sprite_coor(spr, 1, 2))
    char_list2[7- (2)] = str(get_sprite_coor(spr, 2, 2))
    char_list2[7- (3)] = str(get_sprite_coor(spr, 3, 2))
    char_list2[7- (4)] = str(get_sprite_coor(spr, 0, 3))
    char_list2[7- (5)] = str(get_sprite_coor(spr, 1, 3))
    char_list2[7- (6)] = str(get_sprite_coor(spr, 2, 3))
    char_list2[7- (7)] = str(get_sprite_coor(spr, 3, 3))

    byte1_str = "".join(char_list1)
    byte2_str = "".join(char_list2)

    b1_len = (len(byte1_str) + 7) // 8

    b1 = int(byte1_str, 2).to_bytes(b1_len, byteorder="big")

    b2_len = (len(byte2_str) + 7) // 8

    b2 = int(byte2_str, 2).to_bytes(b2_len, byteorder="big")

    return b1, b2

lines = ""

for i in range(128):
    b0, b1 = get_sprite_bytes(get_sprite(i))
    lines += f"{{{hex(b0[0], )}, {hex(b1[0])}}},\n"

with open(out_path, "w") as f:
    f.write(lines)