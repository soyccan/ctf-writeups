 <?php

    highlight_file(__FILE__);

    if ($x = @$_GET['x'])
        eval(substr($x, 0, 5));

