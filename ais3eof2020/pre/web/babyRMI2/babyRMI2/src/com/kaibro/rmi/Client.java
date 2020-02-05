package com.kaibro.rmi;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Client {

    public static void main(String[] args) {

        String host = "140.113.203.209";
        int port = 11099;
//         port = 56666;
        try {
            Registry registry = LocateRegistry.getRegistry(host, port);

            String[] ports = registry.list();
            for (String pp : ports) {
                System.out.println(pp);
            }

            RMIInterface stub = (RMIInterface) registry.lookup("Hello");
            String response = stub.getSecret();
            System.out.println("response: " + response);
        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}
