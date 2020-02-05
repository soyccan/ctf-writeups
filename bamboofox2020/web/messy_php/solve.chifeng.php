<?php

// srand(20191231 + 20200101 + time());
srand(123);

// $url = 'http://34.82.101.212?KEY=0&is_this_flag？=123';
$url = 'http://localhost/source.php?KEY=123&is_this_flag？=123';
$data = array(
    '😂' => 'aaa', // for checking
    '🤣' => 'bbb', // for checking
    '​😂' => '0e123', // for md5
    '🤣​' => '0e215962017', // for md5
);

$mystr = 'Happy';
$mystr .= ' New';
$mystr .= '  Year⁠!~~~';
$array2 = str_split($mystr, 1);
$array3 = str_split("0", 1);

// use key 'http' even if you send the request to https://...
$options = array(
    'http' => array(
        'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
        'method'  => 'POST',
        'content' => http_build_query($data)
    )
);
$context  = stream_context_create($options);
$result = file_get_contents($url, false, $context);
if ($result === FALSE) { /* Handle error */ }

$numbers = explode(" ", explode("\n", $result)[4]);
$flag = "";
foreach(explode(" ", explode("\n", $result)[4]) as $x){
    if($x === "") break;
    var_dump(intval($x));
}

