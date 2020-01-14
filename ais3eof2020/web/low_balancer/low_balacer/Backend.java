import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.List;
import java.io.ByteArrayInputStream;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.Headers;

public class Backend {

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(9487), 0);
        server.createContext("/content", new ContentHandler());
        server.createContext("/api", new APIHandler());
        server.setExecutor(null);
        server.start();
    }

    private static String getBody(HttpExchange t) throws IOException {
        StringBuilder sb = new StringBuilder();
        InputStream ios = t.getRequestBody();
        int i;
        while ((i = ios.read()) != -1) {
            sb.append((char) i);
        }
        return sb.toString();
    }

    private static String reverse(String s) throws Exception {
        StringBuilder rs = new StringBuilder();
        for(int i = s.length() - 1; i >= 0; i--) {
            rs.append(s.charAt(i));
        }
        return rs.toString();
    }

    static class ContentHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange t) throws IOException {
            try {
                String name = getName(t);
                String response = getContent(name);
                List<String> xff = t.getRequestHeaders().get("X-Forwarded-For");
                if(xff != null) {
                    String ip = xff.get(0);
                    System.out.println("[LOG] Backend - source ip: " + ip + "hi");
                    if(ip == "127.0.0.1" || ip == "54.87.54.87") {
                        System.out.println("Hello, Admin!");
                        BufferedReader reader = new BufferedReader(new FileReader("flag.txt"));
                        String flag = reader.readLine();
                        reader.close();
                        response += flag;
                    }
                }
                setHeaders(t);
                writeResponse(t, response);
            } catch (Exception e) {
                System.out.println(e);
                t.sendResponseHeaders(500, 0);
            } finally {
                t.close();
            }
        }

        private String getName(HttpExchange t) {
            String path = t.getRequestURI().getPath();
            return path.replaceFirst("^/content/", "");
        }

        private String getContent(String name) {
            try {
                return reverse(name);
            } catch (Exception e) {
                System.out.println(e);
                return "Error!";
            }
        }
    }

    // TODO
    static class APIHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange t) throws IOException {
            try {
                String body = getBody(t);
                if(!body.isEmpty()) {
                    byte[] b = body.getBytes();
                    ByteArrayInputStream bs = new  ByteArrayInputStream(b);
                    DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
                    dbFactory.setNamespaceAware(true);
                    DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
                    Document doc = dBuilder.parse(bs);
                    NodeList nodeList = doc.getElementsByTagName("txt");
                    Node node = nodeList.item(0);
                    String response = reverse(node.getTextContent());
                    setHeaders(t);
                    writeResponse(t, response);
                } else {
                    setHeaders(t);
                    writeResponse(t, "empty body");
                }
            } catch (Exception e) {
                System.out.println(e);
                t.sendResponseHeaders(500, 0);
            } finally {
                t.close();
            }
        }
    }

    private static void writeResponse(HttpExchange t, String response) throws IOException {
        t.sendResponseHeaders(200, response.length());
        OutputStream os = t.getResponseBody();
        os.write(response.getBytes());
        os.close();
    }

    private static void setHeaders(HttpExchange t) {
        t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
        t.getResponseHeaders().set("X-DNS-Prefetch", "off");
        t.getResponseHeaders().set("X-Frame-Options", "SAMEORIGIN");
        t.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
        if (canCloseConn(t)) {
            t.getResponseHeaders().set("Connection", "close");
        }
        t.getResponseHeaders().set("Content-Type", "text/plain");
    }

    private static boolean canCloseConn(HttpExchange t) {
        List<String> connectionHeaders = t.getRequestHeaders().get("Connection");
        if (connectionHeaders == null || (connectionHeaders.size() == 1 && connectionHeaders.get(0).toLowerCase() == "close")) {
            return true;
        }
        return false;
    }

}
