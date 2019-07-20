from PIL import Image
#import sys, os
#IMAGE_DIR = os.path.dirname(__file__)
#sys.path.append(IMAGE_DIR)
img = Image.open("./littleschoolbus.bmp")
width, height = img.size

Perhap_flag = ""

def lsb(pixel_onlycolor):
    global Perhap_flag
    if pixel_onlycolor & 1:
        Perhap_flag = Perhap_flag + "1"
        return 255

    Perhap_flag = Perhap_flag + "0"
    return 0
 
for x in range(width):
    rgba = img.getpixel((x, 198))
    lsb(rgba[0])
    lsb(rgba[1])
    lsb(rgba[2])

print(Perhap_flag)

