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
strtolower($filename[0]) == "p" ?
    die("Bad 🍊!")
:
    die(htmlspecialchars(file_get_contents($filename)));


