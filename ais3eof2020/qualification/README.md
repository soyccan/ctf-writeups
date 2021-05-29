# Computer Security 2019 Fall Final / AIS3 2019 EOF Qualification
###### tags: `security` `write-up` `multimeter`

* [website](https://eductf.zoolab.org/challenges)
* [official chat room](https://tlk.io/edu-ctf-2019)
* Team Members
  * chiffon_cake
  * boook
  * soyccan
  * WildfootW


# Write-Ups
- official
    - web: https://hackmd.io/gqbp2S-tQeCbZ5Jju32YEg
    - crypto: https://github.com/OAlienO/MYCTF-Challenge#eof-ctf-quals-2020
- non-official: https://hackmd.io/@cXpZn6ltSku4Vwx_OL0bqA/SyyxioFgI

---
# Web

## King of PHP
```php
<?php
if (isset($_GET['info'])) die(phpinfo());
$filename = $_GET['f']??"index.php";
if (isset($_GET['c']) && strlen($_GET['c']) < 87)
{
    $f = "/tmp/" . uniqid(rand() , true);
    if (stripos($_GET['c'], "path")) exit();
    @file_put_contents($f, $_GET['c']);
    die($f);
}
strtolower($filename[0]) == "p" ? die("Bad 🍊!") : die(htmlspecialchars(file_get_contents($filename)));
```

### vulnerability
- local file inclusion: `https://eductf.zoolab.org:28443/?f=/etc/passwd` [name=wildfoot]
- include external resource: `https://eductf.zoolab.org:28443/?f=https://wildfoo.tw`
- phpinfo() information leak

### try: attack vector
#### PHP Phar deserialization
##### Online Resouces
* https://xz.aliyun.com/t/2958
- [generate chain - phpggc](https://github.com/ambionics/phpggc)

- 擋 `filename[0] == "p"` 的問題可以用 compress.zlib:// 繞過 [name=wildfoot]
    - compress.bzip2://phar:///home/sx/test.phar/test.txt
- 但 PHP 序列化只有 metadata，要有\_\_destruct() \_\_wakeup() 這類方法存在才能用 POP Chain，而 source 裡什麼都沒有 [name=soyccan]

#### OPcache
上傳 opcache 編譯過後的檔案 

#### upload tmp file + race condition
```
upload_tmp_dir	no value
```
未嘗試 因為不確定環境為何


### some notes
```
allow_url_fopen	On
PHP Version	7.4.3-dev
SCRIPT_FILENAME	/var/www/html/index.php
```

#### htmlspecialchars()
about
>htmlspecialchars is roughly the same as htmlentities. The difference: character encodings.
>
>Both encode control characters like <, >, & and so on used for opening tags etc. htmlentities also encode chars from other languages like umlauts, euro-symbols and such. If your websites are UTF, use htmlspecialchars(), otherwise use htmlentities().

``` javasciprt
document.characterSet
"UTF-8"
```


## echo
- [name=soyccan]
- 先從註解裡載 source：https://eductf.zoolab.org:49007/echo.zip
- 後端是 Node.js 的 express 框架，模版引擎是 EJS
- 第一個想到 SSTI
    - 但模版內容不可控，只有 text 參數可控
    - 參考資料：
        - https://www.anquanke.com/post/id/84336
        - https://portswigger.net/research/server-side-template-injection
- 接著想到 JS prototype pollution
    - ejs 受原型鏈汙染的例子：
        - https://www.anquanke.com/post/id/185377
        - http://j0k3r.top/2019/09/10/js_prototype_pollution/
        - 但一直沒有試出合併後原型鏈汙染的效果，也找不到哪個函式庫有問題，基本上版本都滿新的
    - https://www.leavesongs.com/PENETRATION/javascript-prototype-pollution-attack.html
- 再看到 body-parser
    - app.js有一行：`app.use(bodyParser.urlencoded({ extended: true }));`
    - 代表使用 qs 作為解析 POST body 的函式庫
    - qs 支援指定物件屬性：
        - text[\_\_proto\_\_][toString]=brabra
        - text 在被 ejs 引入時成功噴錯
        ![](https://i.imgur.com/hzVHwyZ.png)
        - 能控制 text 從 object 轉為 string 時的行為？
            - 我們只能傳字串，不知道怎麼讓 toString 成為我們要的 function


## BabyRMI (solved)
:::success
FLAG{java_registry_is_important_to_rmi_deserialization_vulnerability!}
:::
- Java RMI
    - 用 LocateRegistry.getRegistry 連上 remote
    - 用 registry.list() 察看有那些 registry
        - 有 Hello 和 FLAG
    - 看 interface 檔裡有 getSecret() 這個 method，執行就有 flag 了
- 參考：https://github.com/PoliCTF/sources2017/blob/master/pwn-lamermi/writeup.md
- exploit code: https://github.com/soyccan/hacknote/blob/master/ais3eof2020/web/babyrmi/src-public/com/kaibro/rmi/Exploit.java
```java
// Exploit.java
package com.kaibro.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Exploit {
    public static void main(String[] args) {//throws RemoteException, NotBoundException {
        String host = "140.113.203.209";
        int port = 11099;
        try {
            Registry registry = LocateRegistry.getRegistry(host, port);
            RMIInterface stub = (RMIInterface) registry.lookup("FLAG");
            String response = stub.getSecret();
            System.out.println("response: " + response);

            String[] ports = registry.list();
            for (String por: ports) {
                System.out.println(por);
            }
        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}
```
執行：
```sh
javac com/kaibro/rmi/Exploit.java com/kaibro/rmi/RMIInterface.java
java com.kaibro.rmi.Exploit
```

---
# Pwn
## impossible (solved)
:::success
FLAG{H0w_did_y0u_byp4ss_my_ch3cking?_I7s_imp0ss1b1e!}
:::

```c=
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>


void init(){
    setvbuf(stdout,0,2,0);
    setvbuf(stdin,0,2,0);
    setvbuf(stderr,0,2,0);
}


int main(){

    init();

    char buf[0x100];
    int len = 0;

    printf( "Size: " );
    scanf( "%d" , &len );
    
    if( len < 0 ){
        len = abs( len );
        puts( "Bad hacker :(" );
    }

    if( len > 0x100 ){
        len = 0x100;
        puts( "Too long!" );
    }

    puts( "It's safe now :)" );
    read( 0 , buf , len );

    
    return 0;
}
```
abs() 裡的 assembly code:
```asm
   0x40079b <main+83>     jns    main+114 <0x4007ba>
 
   0x40079d <main+85>     mov    eax, dword ptr [rbp - 0x104]
   0x4007a3 <main+91>     cdq    
 ► 0x4007a4 <main+92>     xor    eax, edx
   0x4007a6 <main+94>     sub    eax, edx
   0x4007a8 <main+96>     mov    dword ptr [rbp - 0x104], eax
   0x4007ae <main+102>    lea    rdi, [rip + 0xe9]
   0x4007b5 <main+109>    call   puts@plt <0x4005b0>
```
- 只要讓 len 是負的，可以通過 > 0x100 的檢查，而且在呼叫 read() 時會 len 會被當成 unsigned，也就是很大的整數，就拿到 buffer overflow
    - no PIE, no canary -> BOF to ROP chain
- 但負的 len 會經過一次 abs()，所以要讓 abs() 後還是負的
- 32-bit signed int 範圍：[-2147483648, 2147483647]
- 讓 len = -2147483648 = 0x80000000
    - abs(len) = 0xfffffffe = unsigned 的 2147483648 = signed 的 -1
    - -1 <= 0x100 通過檢查
    - read() 當成 unsigned 的 2147483648，buffer overflow
- 再做 ROP chain 開 shell

## nonono
長的像經典的 heap 題，有 new, show, remove, exit function
* new 會 `malloc` 一塊指定大小的記憶體，並填入值，其中 malloc 之前不會檢查 pointer 是不是空的
* show 會檢查 pointer 不是空的之後，puts 出值
* free 掉記憶體後，設 pointer 為 null

另外比較特別的有 take_flag function 會 print 出 `fake_flag`

new function:
![](https://i.imgur.com/I2R1YZt.png)

本來以為： fgets 會自動補上 null byte，可以玩 heap overlap
online resouce: [off-by-one NUL byte](https://googleprojectzero.blogspot.com/2014/08/the-poisoned-nul-byte-2014-edition.html)
> fgets 在 libc-2.27 的話，null byte 算在 size 內，不會 overflow [name=soyccan]

take_flag:
![](https://i.imgur.com/bot0Nlh.png)
會從 fake_flag 讀 256 bytes，然後 puts 出來


## EasyROP
題目給的是一個 docker 的環境
執行起來之後由 xinetd 執行 `run.sh` 把 stderr 導到 `/dev/null` 後執行 `pow.py`
`pow.py`執行完 proof of work 後執行 `EasyROP`

 
觀察 main function
main function:
![](https://i.imgur.com/LLuVeHe.png)
一開始先用 read 讀入最多 100 個字元（這邊沒有 `\x00` 截斷, 可能會影響到後續的 `strlen`, `strcpy`）
再用讀入的字串長度在 stack 上 alloca 一段空間
後面也一樣再讀入一串字串 又再 alloca 一段空間
最後做3個 `strcpy` (沒有長度檢查)

觀察 seccomp
```
LD_PRELOAD=./libseccomp.so.2 seccomp-tools dump ./docker/share/EasyROP
 line  CODE  JT   JF      K
=================================
 0000: 0x20 0x00 0x00 0x00000004  A = arch
 0001: 0x15 0x00 0x07 0x40000003  if (A != ARCH_I386) goto 0009
 0002: 0x20 0x00 0x00 0x00000000  A = sys_number
 0003: 0x15 0x04 0x00 0x00000001  if (A == exit) goto 0008
 0004: 0x15 0x03 0x00 0x00000003  if (A == read) goto 0008
 0005: 0x15 0x02 0x00 0x00000004  if (A == write) goto 0008
 0006: 0x15 0x01 0x00 0x00000005  if (A == open) goto 0008
 0007: 0x15 0x00 0x01 0x000000fc  if (A != exit_group) goto 0009
 0008: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0009: 0x06 0x00 0x00 0x00000000  return KILL
```

想法：
整個 binary 到處充滿奇怪的片斷，再加上題目名稱，判斷是要用 ROP 進行 open read write 把 flag 讀出來，但是戳了一天還是弄不出怎麼利用漏洞

note:
在 container 中動態 debug
```
apt install gdbserver, socat
socat TCP-LISTEN:7071,fork,reuseaddr EXEC:"gdbserver \:7070 ./EasyROP"
```
docker-compose.yml
```
    ports:
        - "4444:4444"
        - "7070:7070"
        - "7071:7071"
    expose:
        - "4444"
    cap_add:
        - SYS_PTRACE
    security_opt:
        - seccomp:unconfined
```

### new discovery
主要的漏洞是alloca時沒有考慮放null-byte的空間
alloca() 的長度會補齊到0x10的倍數
如果 alloca(0x10的倍數)
那兩個字串在stack上會剛好連續
重點在 strcpy，總共有3次
flow of main()：
1. read(buf) <- 讀 '\xAA'*0x40
2. newbuf = alloca(strlen(buf))  <- alloc(0x40)
3. read(newbuf)  <- 讀 '\xCC'*0x40
4. extrabuf = alloca(strlen(newbuf))  <- alloc(0x40)
此時 stack:
```
extrabuf: ???????? ???????? ???????? ????????
          ???????? ???????? ???????? ???????? 
          ???????? ???????? ???????? ???????? 
          ???????? ???????? ???????? ???????? 
newbuf:   CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          00000000 00000000 
buf[100]:                   AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA 00000000 00000000
          00000000 00000000 00000000 00000000
          00000000 00000000 00000000
```
5. strcpy(extrabuf, newbuf)
此時 stack: (strcpy會append null-byte)
```
extrabuf: CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
newbuf:   00CCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          00000000 00000000 
buf[100]:                   AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA 00000000 00000000
          00000000 00000000 00000000 00000000
          00000000 00000000 00000000
```
6. strcpy(newbuf, buf)
此時 stack，發現extrabuf和newbuf已經連起來變成0x80 bytes:
```
extrabuf: CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
newbuf:   AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          00000000 00000000 
buf[100]:                   AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA 00000000 00000000
          00000000 00000000 00000000 00000000
          00000000 00000000 00000000
```
7. strcpy(buf, extrabuf)
在大小100=0x64的buf寫入0x80 bytes + 1個'\0' => overflow
```
extrabuf: CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
newbuf:   AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA 
          00000000 00000000 
buf[100]:                   CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
          CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
          CCCCCCCC CCCCCCCC AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA
(overflow):                          AAAAAAAA
          AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
          AAAAAAAA AAAAAAAA 00
```

再來看buf後面有什麼
```
buf[100]:                     CCCCCCCC CCCCCCCC
            CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
            CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC 
            CCCCCCCC CCCCCCCC CCCCCCCC CCCCCCCC
            CCCCCCCC CCCCCCCC AAAAAAAA AAAAAAAA
            AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA
            AAAAAAAA AAAAAAAA AAAAAAAA
(overflow):                            < here 
               are 5 local variables ...      >
            < argc > < argv > <old_esp>
```
重點來了，overflow 恰好把 old_esp 的第0個 byte 蓋成 null-byte
所以原本 old_esp 應該要指向目前 stack frame 後面，蓋掉之後卻有機會跑到前面去
(前提是 ASLR 下它本來大於0x10，有時候不會出問題，害我納悶了好久...)
還原 esp 之後會接 ret，就可以由前面的 buf 和 newbuf 控 eip

### next step


## re-alloc
從名字判斷是 heap 題，比較特別的是使用 2.29 glibc
Makefile 特別強調 `no-pie`
`define MAX 2`, `define MAXSIZE 0x78`
* allocate
* reallocate
* rfree

---
# Reverse
## DuRaRaRa (solved)

- 行為: 一個 PE 包著另一個 PE (在 data 段)，`CreateProcessA()` 之後再用 `WriteProcessMemory()` 寫進 child 的 text 段
- 算好 offset 之後把 data 的資料挖出來，並加上 PE 的 header: 
    - `cat <(head -c 16 DuRaRaRa.exe) <(tail -c +8233 DuRaRaRa.exe) > oraora.exe`
    - 不用算長度，反正後面有多東西不影響 reverse
    - 再拿新的 PE 做 reverse

```c
// DaRaRaRa.exe 
  iVar1 = DAT_00403054;
  local_8 = DAT_00403004 ^ (uint)&stack0xfffffffc;
  GetModuleFileNameA((HMODULE)0x0,local_408,0x400);
  local_45c.hProcess = (HANDLE)0x0;
  local_45c.hThread = (HANDLE)0x0;
  local_45c.dwProcessId = 0;
  local_45c.dwThreadId = 0;
  memset(&local_44c,0,0x44);
  BVar2 = CreateProcessA(local_408,(LPSTR)0x0,(LPSECURITY_ATTRIBUTES)0x0,(LPSECURITY_ATTRIBUTES)0x0,
                         0,4,(LPVOID)0x0,(LPCSTR)0x0,(LPSTARTUPINFOA)&local_44c,
                         (LPPROCESS_INFORMATION)&local_45c);
  if (BVar2 != 0) {
    lpContext = (LPCONTEXT)VirtualAlloc((LPVOID)0x0,4,0x1000,4);
    lpContext->ContextFlags = 0x10007;
    BVar2 = GetThreadContext(local_45c.hThread,lpContext);
    if (BVar2 != 0) {
      ReadProcessMemory(local_45c.hProcess,(LPCVOID)(lpContext->Ebx + 8),local_460,4,(SIZE_T *)0x0);
      lpBaseAddress =
           VirtualAllocEx(local_45c.hProcess,*(LPVOID *)(iVar1 + 0x40304c),
                          *(SIZE_T *)(s_!This_program_cannot_be_run_in_D_00403065 + iVar1 + 3),
                          0x3000,0x40);
      WriteProcessMemory(local_45c.hProcess,lpBaseAddress,&DAT_00403018,
                         *(SIZE_T *)(s_!This_program_cannot_be_run_in_D_00403065 + iVar1 + 7),
                         (SIZE_T *)0x0);
      iVar5 = 0;
      if (*(short *)(iVar1 + 0x40301e) != 0) {
        iVar4 = 0;
        do {
          iVar3 = DAT_00403054 + iVar4;
          WriteProcessMemory(local_45c.hProcess,
                             (LPVOID)(*(int *)(iVar3 + 0x40311c) + (int)lpBaseAddress),
                             &DAT_00403018 + *(int *)(iVar3 + 0x403124),
                             *(SIZE_T *)(iVar3 + 0x403120),(SIZE_T *)0x0);
          iVar5 = iVar5 + 1;
          iVar4 = iVar4 + 0x28;
        } while (iVar5 < (int)(uint)*(ushort *)(iVar1 + 0x40301e));
      }
      WriteProcessMemory(local_45c.hProcess,(LPVOID)(lpContext->Ebx + 8),(LPCVOID)(iVar1 + 0x40304c)
                         ,4,(SIZE_T *)0x0);
      lpNumberOfBytesWritten = (SIZE_T *)0x0;
      nSize = 0x18;
      lpBuffer = GetModuleHandleW((LPCWSTR)0x0);
      WriteProcessMemory(local_45c.hProcess,lpBaseAddress,lpBuffer,nSize,lpNumberOfBytesWritten);
      lpContext->Eax = *(int *)(iVar1 + 0x403040) + (int)lpBaseAddress;
      SetThreadContext(local_45c.hThread,(CONTEXT *)lpContext);
      ResumeThread(local_45c.hThread);
    }
  }
  FUN_00401221();
  return;
}
```
data 段有長得像 PE 的東西：
![](https://i.imgur.com/R7RmgcP.png)


新的exe執行起來，會有一個視窗要你輸入flag，然後他會告訴你你輸入的是不是flag。reverse完之後發現他的行為就是把flag(總長16)拆成4段，每段4個byte，然後去算rolling hash，還要檢查每段的第0和第1個字元是不是指定的字元，通過這些檢查的就是flag。

因為一段的所有可能有大約 100^4 = 1e8 種可能，所以這次用C來寫解題腳本而不是python。

爆搜出來之後，還是有超過一種的可能，這時就用人眼來辨識了。
script: https://github.com/ktpss95112/Practices/blob/master/Computer%20Security%202019/final/rev/DuRaRaRa/solve.c
:::success
FLAG{D-Day:2020/01/14}
:::

## verify (solved)
:::success
FLAG{7h1s15justAbase64enc0d3!}
:::
- 會檢查 argv[1] 是不是 flag
    - argv[1] 長度必須是 24
    - 先把 argv[0] 和 argv[1] 做 xor
    - 推測：argv[0] = "/home/terrynini38514/..."
- 用到很多 xmm, ymm 等 register
- 看到這裡發現，它做的操作的條件判斷不會很複雜，說不定可以試試angr?
    - 用angr跑了大概不到1個小時求出flag
    - https://docs.angr.io/examples
- script: https://github.com/soyccan/hacknote/blob/master/ais3eof2020/rev/vwvwvw/exp.py
```python
import angr
import claripy

angr.l.setLevel('DEBUG')
p = angr.Project('./verify', load_options={"auto_load_libs": False})

args = claripy.BVS('args', 8*24)
initial_state = p.factory.entry_state(args=[p.filename, args], add_options={'BYPASS_UNSUPPORTED_SYSCALL'})

for i, b in enumerate(args.chop(8)):
    initial_state.add_constraints(b >= 0x21, b <= 0x7e)

pg = p.factory.simulation_manager(initial_state)
pg.explore(find=[0x4022cb], avoid=[])
print(pg)
print(pg.found[0].solver.eval(args, cast_to=bytes))

```

## H0W (solved)
murmur: 這題真的超讚的，很有趣又學到不少東西，第一次這麼仔細看C跟python的接口原來長這樣。

丟線上工具解pyc，解出如下的python code：
```python=
# Python bytecode 3.7 (3394)
# Embedded file name: H0W.py
# Size of source mod 2**32: 387 bytes
# Decompiled by https://python-decompiler.com
import sys, struct
from terrynini import *
if len(sys.argv) != 2:
    print('Usage: python3 H0W.py filename')
    exit(0)
nini3()
f = open(sys.argv[1], 'rb').read()
if len(f) % 4 != 0:
    f += (4 - len(f) % 4) * '\x00'
nini1()
nini4()
for i in range(0, len(f), 4):
    nini6(nini5(struct.unpack('<i', f[i:i + 4])[0]))

for i in list(map(ord, nini2())):
    nini6(i)

print('Complete')
```

發現有`nini1`~`nini6`這6個怪怪的函數，估計應該是在`terrynini.so`裡面。
用ghidra來decompile `terrynini.so`，發現有6個沒有名稱的函數，又剛剛好這6個函數裡面都有呼叫一個C library的函數，還不重複，所以可以得到每個nini函數和一個C library函數的對應關係。
```
$ gdb python3
gdb-peda$ # set breakpoint at time, gmtime, fopen, srand, rand, fwrite
gdb-peda$ r H0W.pyc input.txt
gdb-peda$ # use `continue` to observe the sequence of the functions
```
![](https://i.imgur.com/tA0058H.png)
得到了這6個C library函數的先後出現順序，再比對一下decompyle出來的python code，就可以得到每個nini函數是對應到terrynini.so裡面的哪個函數了，結果如下圖：
![](https://i.imgur.com/bKQ1rtX.png)

所以整個H0W.pyc的行為，就是srand(time)之後，把argv[1]的檔案每4個byte一組，每組根據當前的rand()來決定要使用「ichinokata」（日文：方法一）、「ninokata」（日文：方法二）、「sannokata」（日文：方法三）、「yonnokata」（日文：方法四）的哪個方法來加密。最後，把當前的年、月、星期、時、分、秒給寫進輸出檔案的最後面。**特別注意，寫進輸出檔案裡面的是星期而不是日期，還有月份跟星期是從0開始算的。**

因此，我們可以透過output.txt的檔案結尾來猜測srand()當下的時間（總共有4種可能，因為給的是星期而不是日期），然後把output.txt用那四種加密方法的逆操作來還原出原本的檔案。

解出4個檔案之後，`strings`找一下發現都沒有flag的字串在裡面，還心想是不是解爛了，想了10分鐘之後通靈到有可能檔案不是純文字，`file`看一下還真的有圖片檔，果然就有flag了~~~
![](https://i.imgur.com/J1FO66r.jpg)

人生第一次在CTF首殺某個感覺有點複雜的題目:heart:
script: https://github.com/ktpss95112/Practices/blob/master/Computer%20Security%202019/final/rev/H0W/solve.py
:::success
FLAG{H3Y_U_C4NT_CHiLL_H3R3}
:::

## PokemonGo (Solved)
Google搜尋了一下，找到了這篇：http://blog.terrynini.tw/en/2018-0CTF-Quals-g0g0g0/ ，作者還是出題者自己呢！看起來照著做就會過了的感覺，頓時安心了不少。

照著這篇blog所說的，先把log裡面所有看起來是library function的部份刪掉，log檔瞬間從快19000行變成只剩下490行。接著發現有很多重複的部份，猜測可能是有迴圈的結構，所以稍微整理一下，剩下大約130行左右的pseudo-code。

pseudo-code裡面驗證flag的部份大概長這樣：
```
flag[0] + flag[1] = 185;
flag[1] + flag[2] = 212;
...
flag[19] + flag[0] = 177;
```
發現只有19條線性獨立的方程式，卻有20個未知數，所以就暴力列舉囉。
![](https://i.imgur.com/6sqSU56.png)
肉眼確認答案的時候差點看不出來，這個flag也太鬧XD
script: 
:::success
FLAG{PikAPikApikaPikap1Ka}
:::

## YugiMuto (Solved)
murmur: 我覺得這題很難搞，沒想到最後居然只剩229分，感覺他應該有300~400分的難度R……

Google搜尋了「gameboy ctf」，發現 [這篇writeup](https://github.com/VoidHack/write-ups/tree/master/Square%20CTF%202017/reverse/gameboy) ，於是就載了 [BGB](http://bgb.bircd.org/) ，在windows裡面跑起來了。
遊戲跑起來之後很單純，就只有一個輸入文字的介面，輸入完之後選取最右下角的打勾，程式就會驗證我們輸入的東西是不是flag。

本來在bgb上面下斷點，跑起來之後他不理我，完全不知道要怎麼操作，網路上面也都查不到教學文，是要放棄這題了。後來又回來解是因為分數實在變太低，覺得應該沒有那麼難，所以才回來繼續解。

心路歷程是這樣的：
先打開bgb cheat searcher，這裡跟cheat engine的功能有點像，可以找到符合某個值的位址，也可以根據值的變化來搜尋。我猜測應該有兩個變數記錄我們的游標位址，一個x軸一個y軸，所以就移動游標之後更新搜尋的值，反正就搜到了，`C7B2`這裡存的是x座標。
然後對著`C7B2`點右鍵，按「set access breakpoint」，然後點一下遊戲視窗，再移動一下游標，發現assembly的介面停在某個指令，而且遊戲畫面的游標也沒有更新，然後我一行一行執行assembly，發現在某個指令執行完的瞬間，遊戲畫面上的游標更新了！然後我好像就抓到了bgb要怎麼使用了。
接著，發現某個感覺應該像是負責判斷我們按了什麼按鍵的區域，經過一些實驗，發現這5個jp對應到5種我們的按鍵訊號(上下左右和s)，如下圖所示：
![](https://i.imgur.com/u2rvbWw.png)

在select那邊下斷點之後，到遊戲裡面選取打勾，一步一步執行下去就會發現有某一串感覺像是在準備一大長串stack上的值，有可能是作為後續比對的用途，用ida pro來看流程圖，也在底下發現了一個迴圈比對的結構，所以在`0E61`的函數真的就是用來驗證我們的flag正不正確。
![](https://i.imgur.com/UV8tCmt.png)

經過了一點點的小實驗，發現了我們輸入的字的encoding會是像下圖那樣：
![](https://i.imgur.com/UgappgD.png)

所以把用來比對的那些資料轉換成文字，就可以得到flag了~~
:::success
FLAG{OHMYGODY0UAREGAM3B0Y}
:::

附上一張痛苦的過程截圖：
![](https://i.imgur.com/GcePRfw.png)


---
# Misc
## Ponzi Scheme (solved)
Computer an answer such that `SHA256(ZLx9aJWsb4N9xxq7bKb88g + answer)` has **22** leading zero bits in **600** seconds

使用 https://balsn.tw/proof-of-work/solver/js.html 來跑出解答後按照網頁的指示投資一下就可以拿到 flag 了。

<!-- 大概沒關係--[DEFCON 26 pow](https://github.com/kpcyrd/defcon26-pow) -->

---
# Crypto
## back to 1997
https://eprint.iacr.org/2013/319.pdf

## winner winner chicken dinner (solved)

這題是可以每次猜ㄧ個 01 之間的數字，如果猜對了就可以賺錢、猜錯了就會輸錢，而最後的目標也就是贏到足夠多的錢。

觀察到程式中 random 的方式，可以發現到每次猜數字的答會會是由一個叫 state 的數字決定的，且在每次猜數字前 state 都會經過一些迭代運算。因此只要能知道 state 的數值，也就可以推測出接下來每次猜數字的答案。
![](https://i.imgur.com/2EqsYMv.png)

考慮程式剛跑起來時變數 state 的值，總共有 64 個 bit，將每個 bit 拆開來看之後，可以是視作有 64 個變數 $a_0, a_1, ..., a_{63}$，每個變數可能是 0 或 1。接下來每一次迭代都可以看成是一種線性運算，也就是說迭代後的 state' 的最後一位會是由 $a_{b_0} \oplus a_{b_1} \oplus ... \oplus a_{b_k}$ 所決定出來的結果。因此可以透過先猜測 64 次並拿到相對應的結果來列出 64 元一次多項式，接著用高斯消去解出 $a_0, a_1, ..., a_{63}$，得到原始 state 的值，就可以控制接下來的每一次猜測了。

script: https://pastebin.com/neTFzi36

:::success
FLAG{w3_W4nT_fA_d4_CaI!!!fA_d4_CaI!!!fA_d4_CaI!!!}
:::

## Train (Solved)
這題的是RSA-CBC，CBC mode很有可能會出現像padding oracle的攻擊，這題的漏洞利用方式跟padding oracle也有異曲同工之妙。

觀察server.py與server回傳的東西，可以得到plain text有4個block，每個block有16 bytes；cipher text有5個block，每個block有128 bytes。
![](https://i.imgur.com/IizQVVU.png)

注意到在CBC mode下，如果我們只傳送了$c_1$和$c_2$，server還是可以解密，而且只會解出$m_2$。

我們要利用的是server.py裡面的use函數來作為一個oracle，洩漏出有關明文的資訊。可以利用的點在於第70行，如果plain裡面的`|`(後文用pipe來稱呼)只有一個的話，那麼這裡就會發生error，跳到except的部份去執行。
![](https://i.imgur.com/4YAOPfw.png)

例如我們想要洩漏$m_2$的最後一個byte，那麼我們就可以窮舉$c_2[-1]$（$c_1$的最後一個byte），然後只送$c_1+c_2$過去。大部分的情況，解出來的$m_2'[-1]$不會是pipe，所以整個$m_2'$只有一個pipe，server會輸出`Oops`；只有在$m_2'[-1]$是pipe的時候，第70行不會出錯，server會回傳`Wrong`，這個時候我們就可以推算出$m_2[-1]=124+c_2'[-1]-c_2[-1]$，其中124是pipe的ascii值。

利用同樣的原理，我們也可以leak出$c_3$的每一個byte，特別要注意的是$m_3$裡面沒有任何的pipe，所以我們需要自己構造一塊$c_4=RSA(c_3+pipe)$，這樣子解出來的plain才可以利用上面的手法來leak，否則pipe的數量會不夠，server永遠只會輸出`Oops`。

script: https://github.com/ktpss95112/Practices/blob/master/Computer%20Security%202019/final/crypto/Train/solve_v2.py
:::success
FLAG{cBCNEVERGetSoLD}
:::

---
# General
## Best Rank
![](https://i.imgur.com/zyeCWNv.png)

## Final Rank
![](https://i.imgur.com/i6cQD3B.png)
![](https://i.imgur.com/lkDdxxS.png)
![](https://i.imgur.com/kBLrNsr.png)
![](https://i.imgur.com/uYOZQ8j.png)
