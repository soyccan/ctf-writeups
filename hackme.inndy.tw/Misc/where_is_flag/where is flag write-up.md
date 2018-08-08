where is flag write-up
===

原本不知道FLAG的規則 只用了:

	pat = "FLAG{[^}]*}"

跑出一堆東西XD 因為想不到怎麼用 只好暴力丟flagXDDDDDD(不良示範XD):

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
			
最後發現正確的re是:

	FLAG{[^()\[\]{}@]+}