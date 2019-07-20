import requests
import re
from bs4 import BeautifulSoup

web_url = "https://hackme.inndy.tw/otp/?issue_otp=a"


My_list = [[True] * 256 for i in range(50)]

for i in range(1000):
    r = requests.get(web_url)
    soup = BeautifulSoup(r.text, "html.parser")
    text = str(soup)
    temp = re.findall("[^\n]*\n", text)

    for j in range(10):
        hex_text = temp[j]

        for k in range(1, 51):
            char_hex = "0x" + hex_text[k * 2 - 2: k * 2]
            char_int = int(char_hex, 16)
            My_list[k - 1][char_int] = False

flag = ""

for i in range(50):
    for j in range(256):
        if My_list[i][j]:
            flag += chr(j)
            
print(flag)


