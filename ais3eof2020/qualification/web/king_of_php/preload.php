<?php

class InitialOperation {

    private $path = "/tmp/*";

    function __destruct() {
        exec("rm ".$this->path);
    }

}

