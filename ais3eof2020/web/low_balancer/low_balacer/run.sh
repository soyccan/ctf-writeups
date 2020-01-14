#!/bin/sh
java  -Djava.net.preferIPv4Stack=true Backend &
java  -Djava.net.preferIPv4Stack=true Backend2 &
java  -Djava.net.preferIPv4Stack=true -cp ".:deps/*"  Frontend
