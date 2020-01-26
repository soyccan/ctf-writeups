import requests, urllib3, logging
from urllib.parse import quote, unquote
from base64 import b64decode as d64
from base64 import b64encode as e64

import padding_oracle_attack

def party(flag):
    COOKIES = {
        'PHPSESSID': '7k5avnarq3rp91orhfruha6sac',
        'rack.session': 'BAh7CEkiD3Nlc3Npb25faWQGOgZFVEkiRTllM2QzY2E4ZWNjOTRjMGQ0YmJj%0AMTY2ZTI2M2VmYzU1N2QzZDk3MjM3YTRhOTUxNzIwNGY1ZTU3MzUxMmY3YjUG%0AOwBGSSIKZmxhc2gGOwBGewBJIgx1c2VyX2lkBjsARmk4%0A--29774fa1138ec522fe9470e96ace9cdd1656c4b0',
        'session': 'b6ef01b6-6008-41d8-a383-2aefc8b3bd22',
    }
    r = requests.get(
        'https://edu-ctf.csie.org:10190/party.php',
        cookies={'FLAG': quote(e64(flag)), **COOKIES},
        verify=False,
    )

    if 'CHEATER' in r.text:
        return False
    elif 'strange' in r.text:
        return True
    else:
        return True

def main():
    urllib3.disable_warnings()
    logging.basicConfig(level='DEBUG')


    valid_flag = 'md%2FG%2FwgIF%2BFQ1s%2FCA6zZhGzFnTHfQv3AA%2BNdm0kqlNDQ%2FbcUkgqFsIPit%2B%2BGnoKQZ6tzvBHj08V3pdE2S6qWVV7Tng84JaAEJcNN1RLKneJE9JHRWXQ6df3iQ1fAgpu3'
    flag_len = 96
    block_count = 6
    block_size = 16

    cipher = bytes(d64(unquote(valid_flag)))

    assert len(cipher) == flag_len
    assert party(cipher)

    plain = padding_oracle_attack.crack(cipher, block_size, block_count, party)
    print(plain)


main()
