sync with: https://hackmd.io/CwQwZg7ARgjCEFoBMYDMwGiRgnABglUzzwFMBjVKJCpAEyA=

# AIS3 2017 Pre-Exam Write Up
by soytw

## web1
FLAG: AIS3{As_Simple_As_Usual}
the website will automatically redirect to index.html when visited with browser
but the actual page is index.php
so cURL is good friend

## web2
FLAG: AIS3{Hey!Why_can_you_login_without_the_password???}
md5("240610708") => 0e462097431906509019562988736854
use row: array("username" => "sena", "password" => "0e959146861158620914280512624073")
compare with "==" => true (string is converted to float and 0 == 0)

## web3
FLAG: AIS3{Cute_Snoopy_is_back!!?!?!!?}
[NOTE] http://bruce30262.logdown.com/posts/289801-ais3-2015-pre-exam-complete-writeup
local file inclusion
https://quiz.ais3.org:23545/?p=php://filter/convert.base64-encode/resource=index
get a BASE64-encrypted string
decode it and get the source code
flag is in the comments

## pwn2
FLAG: ais3{Just_a_simpl3_overflow}
type 20+4 null characters
since the string in the struct is only 20 in length
the password will be overwritten to 0
[THINKING] Why does typing more than 24 characters cause the output leaving 'M' only?

## rev1
FLAG: AIS3{h0w d1d y0u s3e it}
run the program with the output redirected to a file

## crypto1
FLAG: AIS3{A XOR B XOR A EQUALS B}
"AIS3" to int => X
964600246 ^ X => Y
Y == var1 ^ var2 ^ var3 ^ var4
xor each result with Y and convert it from int to 4 characters

## crypto3
FLAG: AIS3{SHA1111l111111_is_broken}
download SHA1-collided two PDF files from https://shattered.io/
sent them: curl https://quiz.ais3.org:32670/ -X POST --data-urlencode username@shattered-1.pdf --data-urlencode password@shattered-2.pdf
[NOTE] https://httpbin.org/

## misc1
FLAG: ais3{hello, world!}

## misc2
FLAG: AIS3{pika}
1. read the response header get an string in "HereItIs" field
1. decode that with BASE64 get "S74G32.php"
1. load the page and set the backgroud to yellow
1. gotcha



# After Attending the Lecture
## misc3
FLAG: ais3{TIMEFORDEBUGGING!!}
gdb commands:
```
b  main
r > flag

set $rip = callme1
p strcpy(input, "HELLOWORLD")
b *0x400f4e
c

set {int}($rbp - 4) = 1
b *0x400f9a
c

set $rip = callme2
p strcpy(input, "YOUAREANEXPERT")
b *0x401195
c

set $rip = callme3
p strcpy(input, "LASTONEGOGOG0")
p memcpy(encrypted, 0x4012b7, 8)
c
```