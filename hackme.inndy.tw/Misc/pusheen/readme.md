這題其實相當簡單，也不算是CTF
打開題目裡的pusheen分為有填色及沒填色的
當時我第一個直覺就是應該有一個是0一個是1
很幸運地猜測是對的

```
g++ solve1.cpp
./a.out < pusheen.txt > pusheen_decode
python3 < pusheen_decode
```
