<?php
// PHP is the best language for hacker
// Find the flag !!
highlight_file(__FILE__);
$_ = $_GET['🍣'];

if( strpos($_, '"') || strpos($_, "'") ) 
    die('Bad Hacker :(');

eval('die("' . substr($_, 0, 16) . '");');

