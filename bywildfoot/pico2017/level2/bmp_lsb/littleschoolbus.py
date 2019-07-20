from PIL import Image
#import sys, os
#IMAGE_DIR = os.path.dirname(__file__)
#sys.path.append(IMAGE_DIR)
img = Image.open("./suhidden.png")
width, height = img.size

def lsb(pixel_onlycolor):
    if pixel_onlycolor & 1:
        return 255
    return 0
 
for y in range(height):
    for x in range(width):
        rgba = img.getpixel((x,y))
        rgba = (lsb(rgba[0]), lsb(rgba[1]), lsb(rgba[2]))
        img.putpixel((x,y), rgba)

img.show()
img.save("new.bmp")

