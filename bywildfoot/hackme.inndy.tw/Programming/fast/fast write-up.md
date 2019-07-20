fast write-up
=============


(compute)
a.out(c++) -stdio- <---socat---> python socket <-----> hackme.inndy.tw:5500


listen using socat:

	$ socat tcp-listen:12398 exec:./a.out