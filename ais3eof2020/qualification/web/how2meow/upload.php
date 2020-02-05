<?php header("content-security-policy: default-src 'self'; img-src *;"); ?>
<!DOCTYPE html>
<head>
<title>how2meow</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="assets/bulma.min.css">
<script defer src="assets/all.js"></script>
</head>

<body>
<section class="section">
<div class="container is-primary">

<h2 class="title is-2 is-spaced bd-anchor-title">Meow Uploader</h2>
<h2 class="subtitle">If you upload a valid meow file, I'll give u a secret meow image!</h2>
<form action="upload.php" method="post" enctype="multipart/form-data">
    Select meow to upload:<br>
    <input type="file" name="meow" class="button is-light">
    <input type="submit" class="button is-success" value="Upload" name="submit">
</form>

<br><br>
<a href="?src">Meow src</a><br>
<hr>
<br>
<?php

if(isset($_GET['src']))
    die(highlight_file(__FILE__));

if(isset($_POST['submit'])) {

    if(filesize($_FILE['meow']['tmp_name']) > 10000)
        die("Bad meow! Your size is toooo big!");

    $target_dir = "upload/";
    $sandbox_dir = "/tmp/".bin2hex(random_bytes(32))."/";
    $name = $target_dir.bin2hex(random_bytes(32));
    $filetype = strtolower(pathinfo($_FILES["meow"]["name"],PATHINFO_EXTENSION));

    if($filetype !== "meow") die($filetype);

    try {

        @exec("mkdir " . $sandbox_dir);
        @exec("unzip ".$_FILES['meow']['tmp_name']." -d " . $sandbox_dir);

        if(substr(file_get_contents($sandbox_dir . "meow"), 0, 7) === "edu-ctf") {
            @move_uploaded_file($_FILES["meow"]["tmp_name"], "/var/www/html/".$name);
            echo "Nice! This is your file: " . $name . "<br>";
            echo "And the secret meow image: " . file_get_contents("/secret_meow") . "<br>";
        } else {
            die("Bad meow!");
        }

    } finally {
        @exec("rm -rf " . $sandbox_dir);
    }
}

?>

</div>
</section>
