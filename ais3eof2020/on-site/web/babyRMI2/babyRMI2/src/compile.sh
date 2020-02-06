#!/bin/bash
javac -cp "deps/*:." \
    com/kaibro/rmi/Server.java \
    com/kaibro/rmi/RMIInterface.java \
    com/kaibro/rmi/Client.java \
    ./RMIexploit.java
#     com/kaibro/rmi/TransformTest.java \
#     exp/RMIRegistryExploit.java \
