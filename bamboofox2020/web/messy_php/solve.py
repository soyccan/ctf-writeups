import requests
import hashlib

target = 'http://34.82.101.212/'
target = 'http://localhost:1111'
msg = 'hihi'
r = requests.post(target,
    params={
        'KEY': 'fine',
        'is_this_flag？': 'yes yes yes',
    },
    data={
        '😂': 'foo',
        '🤣': 'bar',
        '\u200b😂': hashlib.md5(msg.encode()).hexdigest(),
        '🤣\u200b': msg})
print(r.text)
