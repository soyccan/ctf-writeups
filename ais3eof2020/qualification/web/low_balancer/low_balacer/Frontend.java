import java.io.*;
import java.util.List;
import java.net.URLEncoder;
import java.nio.file.Files;
import java.net.InetSocketAddress;
import java.net.URI;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.Headers;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.ResponseHandler;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import com.google.gson.Gson;
import java.util.Base64;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Random;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Frontend {

    private static Gson gson = new Gson();

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new IndexHandler());
        server.createContext("/connect", new ConnectHandler());
        server.createContext("/static", new FileHandler());
        server.setExecutor(null);
        server.start();
    }

    static class ConnectHandler implements HttpHandler {
        private class Req {
            public String[] names;
        }

        @Override
        public void handle(HttpExchange t) throws IOException {
            try {
                String body = getBody(t);
                Req req = gson.fromJson(body, Req.class);
                if (req.names.length > 5) {
                    t.sendResponseHeaders(400, 0);
                    return;
                }

                String ip = getIP(t);
                String[] resp = getContent(t, req.names, ip);
                String response = gson.toJson(resp);
                t.getResponseHeaders().set("Content-Type", "application/json");
                t.sendResponseHeaders(200, response.length());
                OutputStream os = t.getResponseBody();
                os.write(response.getBytes());
                os.close();
            } catch (Exception e) {
                System.out.println(e);
                t.sendResponseHeaders(500, 0);
            } finally {
                t.close();
            }
        }

        public static String decodeValue(String value) {
            try {
                return URLDecoder.decode(value, StandardCharsets.UTF_8.toString());
            } catch (UnsupportedEncodingException ex) {
                throw new RuntimeException(ex.getCause());
            }
        }

        private String getBody(HttpExchange t) throws IOException {
            StringBuilder sb = new StringBuilder();
            InputStream ios = t.getRequestBody();
            int i;
            while ((i = ios.read()) != -1) {
                sb.append((char) i);
            }
            return sb.toString();
        }

        private String getIP(HttpExchange t) {
            Headers reqHeaders = t.getRequestHeaders();
            List<String>is_proxy = reqHeaders.get("X-Forwarded-For");

            try {
                String tmpip;
                if(is_proxy != null) {
                    String pattern = "^((0|1\\d?\\d?|2[0-4]?\\d?|25[0-5]?|[3-9]\\d?)\\.){3}(0|1\\d?\\d?|2[0-4]?\\d?|25[0-5]?|[3-9]\\d?)$";
                    tmpip = "87.87.87.87";
                    if(is_proxy.get(0).matches(pattern)) {
                        tmpip = is_proxy.get(0);
                    }
                } else {
                    InetAddress ip;
                    ip = InetAddress.getLocalHost();
                    tmpip = ip.getHostAddress();
                }

                if(tmpip.contains("127.0.0.1") || tmpip.contains("0.0.0.0"))
                    tmpip = "87.87.87.87";
                return tmpip;

            } catch (Exception e) {
                System.out.println(e);
                return "Error";
            }
        }

        private String[] getContent(HttpExchange t, String[] names, String ip) throws Exception {
            CloseableHttpClient httpclient = HttpClients.createDefault();
            List<String>original_url = t.getRequestHeaders().get("X-Original-URL");
            ResponseHandler<String> responseHandler = new ResponseHandler<String>() {
                @Override
                public String handleResponse(final HttpResponse response) throws IOException {
                    return EntityUtils.toString(response.getEntity());
                }
            };
            try {
                String[] resp = new String[names.length];
                Random rand = new Random();
                String ourl = "/";
                if(original_url != null) ourl = decodeValue(original_url.get(0));
                for (int i = 0; i < names.length; i++) {
                    String name = names[i].trim();

                    // fire requests to backend server
                    URI url;
                    if(rand.nextInt(2) == 1) { 
                        url = new URI("http://localhost:9487/content/" + URLEncoder.encode(name, "UTF-8"));
                    } else {
                        url = new URI("http://localhost:9488/content/" + URLEncoder.encode(name, "UTF-8"));
                    }
                    HttpGet httpget = new HttpGet(url);
                    httpget.setHeader("Referer", "Frontend-Server");
                    httpget.setHeader("X-Forwarded-For", ip);
                    httpget.setHeader("X-Original-URL", ourl);
                    String response = httpclient.execute(httpget, responseHandler);
                    resp[i] = response;
                }
                return resp;
            } finally {
                httpclient.close();
            }
        }
    }

    static class IndexHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange t) throws IOException {
            try {
                String path = t.getRequestURI().getPath();
                if(path.equals("/robots.txt")) {
                    File file = new File("static/robots.txt");
                    t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
                    t.getResponseHeaders().set("Content-Type", "text/plain");
                    t.sendResponseHeaders(200, file.length());
                    OutputStream os = t.getResponseBody();
                    Files.copy(file.toPath(), os);
                    os.close();
                } else {
                    File file = new File("index.html");
                    t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
                    t.getResponseHeaders().set("Content-Type", "text/html");
                    t.sendResponseHeaders(200, file.length());
                    OutputStream os = t.getResponseBody();
                    Files.copy(file.toPath(), os);
                    os.close();
                }
            } catch (Exception e) {
                System.out.println(e);
                t.sendResponseHeaders(500, 0);
            } finally {
                t.close();
            }
        }
    }

    static class FileHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            try {

                String path = t.getRequestURI().getPath().replaceFirst("^/static", "").replaceAll("^[ \t]+|[ \t]+$", "");
                if(path.equals("/")) {
                    t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
                    t.getResponseHeaders().set("Connection", "Close");
                    t.sendResponseHeaders(403, 0);
                } else if(path.equals("/README.md")) {
                    String filePath = "static" + path;
                    File file = new File(filePath);
                    t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
                    t.getResponseHeaders().set("Content-Type", "plain/text");
                    t.sendResponseHeaders(200, file.length());
                    OutputStream os = t.getResponseBody();
                    Files.copy(file.toPath(), os);
                    os.close();
                } else {
                    String filePath = "static/meow.jpeg";
                    File file = new File(filePath);
                    t.getResponseHeaders().set("Server", "KB-LowBalancer-v1.0");
                    t.getResponseHeaders().set("Content-Type", "image/jpeg");
                    t.sendResponseHeaders(200, file.length());
                    OutputStream os = t.getResponseBody();
                    Files.copy(file.toPath(), os);
                    os.close();
                }

            } catch (Exception e) {
                System.out.println(e);
                t.sendResponseHeaders(500, 0);
            } finally {
                t.close();
            }
        }

    }


}
