#!/bin/bash
javac -cp "deps/*:." \
    com/kaibro/rmi/Server.java \
    com/kaibro/rmi/RMIInterface.java \
    com/kaibro/rmi/Client.java
#     exp/RMIRegistryExploit.java \
