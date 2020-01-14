import requests, json, pprint, collections

def fetch():
    COOKIES = 'AWSALB=FraqrsyJNWU+E0mnvUh0TGCef/d8sSuLzcrLmCfF7CvVR0PeVGChvMn8/wVfxuoYmmbpsI8nNYiPo41YRWLbum+Vx7eC79ZrP8mDq3cT7rlzLQanLPy2cK/L9TEx; session=0b3afb3b-ebee-45b1-9981-7b8a5a2192a6; __cfduid=d187cddb5e4a5d9bfd27bf71bc8199f261570248212'

    chal_info = []
    for chalid in range(100):
        r = requests.get(f'https://2019.balsnctf.com/api/v1/challenges/{chalid}', headers={'cookie': COOKIES})
        if r.status_code == 200:
            print(r.text)
            chal_info.append(r.json())

    open('chal.json', 'w').write(json.dumps(chal_info))

def show():
    data = json.load(open('chal.json'))
    chals = collections.defaultdict(list)
    for item in data:
        chals[item['data']['category']].append(item['data'])
    for cat in chals:
        print('#'*22)
        print('#{: ^20}#'.format(cat))
        print('#'*22)
        for chal in chals[cat]:
            print(chal['name'])
            print('------------------------')
            print(chal['description'])
            print('========================')
            print()

show()
