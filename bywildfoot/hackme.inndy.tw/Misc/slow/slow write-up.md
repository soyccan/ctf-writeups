slow write-up
===

\n
...Bad
F\n
....Bad
FL\n
.....Bad

* FLAG{
	* no lowercase 
	* no space
	* \w	數字、字母、底線	[a-zA-Z0-9_]


code:

	test = "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	for x in test:
