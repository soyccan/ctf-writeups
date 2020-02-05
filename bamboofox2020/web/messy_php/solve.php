<?php
// date --date='Dec 31 00:00:00 CST 2019' +%s
// 1577721600
// date --date='Dec 31 23:59:59 CST 2019' +%s
// 1577807999

$fllllllag='BAMBOOFOX{bbb}';
$KEY='bbb';

for ($tt=1577721600; $tt<=1577807999; $tt++) {
    srand(20191231 + 20200101 + $tt);
    $mystr = 'Happy';
    $mystr .= ' New';
    $mystr .= '  Year⁠!~~~';
    # Useless comment here
    $array1 = str_split($fllllllag, 1);
    # 2019-01-01
    # Alice: What is array1, array2, and array3 ????
    # 2019-12-31
    # Alice: Can someone explain to me?
    $array2 = str_split($mystr, 1);
    # Want to kill your colleague for shitty code?
    # Call 000000000 now
    $array3 = str_split($KEY, 1);
    $final = '';
    # More useless changelog here
    foreach( $array1 as $value ){
        # 2019-12-31
        # Bob: This should be ok to protect our secret
        # Alice: No
        # Bob: Yes, it is
        # Alice: No!
        # Bob: prove it to me?
        # Ann: don't chat in here, plz
        # Bob: fine
        $final .= @strval(ord($value) ^ rand()
            ^ $array2[rand() % count($array2)]
            ^ ($array3[rand() % count($array3)] * random_int(1,128))) . ' ';
    }
    echo $final;
}
