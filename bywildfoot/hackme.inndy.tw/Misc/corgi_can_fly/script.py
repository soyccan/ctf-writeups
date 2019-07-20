from PIL import Image
import binascii

image = Image.open("corgi-can-fly.png")
width, height = image.size
binary_string = ""
for y in range(height):
    for x in range(width):
        r, g, b = image.getpixel((x, y))
        color = (r << 16) + (g << 8) + b
        last_bit = str(bin(color))[-1]
        binary_string += last_bit
#print(binary_string)


hex_string = hex(int(binary_string, 2))
hex_string = hex_string[2:]
print(hex_string)
print(binascii.unhexlify(hex_string))
print(bytes.fromhex(hex_string).decode('utf-8'))

