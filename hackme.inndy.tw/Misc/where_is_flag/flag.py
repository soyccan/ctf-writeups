import re
import requests
import json

keyin = input()
pat = "FLAG{[^}]*}"
#pat = "FLAG{[^()\[\]{}@]+}"     # correct answer
flag_group = re.findall(pat, keyin)

#print(flag_group)
url = "https://hackme.inndy.tw/scoreboard/"

#print(flag_group[0])           # correct answer


for flag in flag_group:
    print(flag)
    s = requests.Session()
    get_params = {
        "declare" : "my_name",
        "name" : "Wildfoot"
    }
    s.get(url, params = get_params, timeout = 5)  #login success
    print(s.cookies)
    
    post_data = {
        'name' : 'Wildfoot',
        'flag' : flag
    }

    r = s.post(url + "?capture=the_flag", data = post_data, timeout = 5)
    print(r.text)
    #print(s.cookies)
    #print(post_data)
    #print(json.dumps(post_data))

