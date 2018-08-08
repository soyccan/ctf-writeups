#include <stdio.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;
  
int main()  
{  
  struct sockaddr_in server;  
  int sock;  
  char buf[512];  
  int n;  
  
  /* 製作 socket */  
  sock = socket(AF_INET, SOCK_STREAM, 0);  
  
  /* 準備連線端指定用的 struct 資料 */  
  server.sin_family = AF_INET;  
  server.sin_port = htons(12001);  
  
  /* 127.0.0.1 是 localhost 本機位址 */  
  inet_pton(AF_INET, "35.201.132.60", &server.sin_addr.s_addr);  
  
  /* 與 server 端連線 */  
  connect(sock, (struct sockaddr *)&server, sizeof(server));  
  
  /* 從伺服器接受資料 */  
  memset(buf, 0, sizeof(buf));  
  n = read(sock, buf, sizeof(buf));  
  
  //printf("\t[Info] Receive %d bytes: %s\n", n, buf);  
  
  for(int i = 0;i < sizeof(buf);i++){
      cout << buf[i];
  }

  memset(buf, 0, sizeof(buf));  
  n = read(sock, buf, sizeof(buf));  
  for(int i = 0;i < sizeof(buf);i++){
      cout << buf[i];
  }

  srand(0);
  for(int i = 0;i < 16;i++){
    string str = to_string(rand() % 200);
    str = str + "\n";
    char cha[str.length()];
    for(int j = 0;j < str.length();j++){
        cha[j] = str[j];
    }
    send(sock , cha , str.length() , 0 );
    cout << str;
  }

  usleep(50000);

  memset(buf, 0, sizeof(buf));  
  n = read(sock, buf, sizeof(buf));  
  for(int i = 0;i < sizeof(buf);i++){
      cout << buf[i];
  }
  usleep(50000);
  memset(buf, 0, sizeof(buf));  
  n = read(sock, buf, sizeof(buf));  
  for(int i = 0;i < sizeof(buf);i++){
      cout << buf[i];
  }

  /* 結束 socket */  
  close(sock);  
  return 0;  
}  
