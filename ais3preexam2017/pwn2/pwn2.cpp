// ais3_pwn1.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <windows.h>  
#include <stdio.h>  
#include <stdlib.h>

struct user {
	char name[20];
	int pass;
} ais3_user;

void menu() {
	puts("=================================");
	puts(" 1. Capture The Flag ");
	puts(" 2. Exit ");
	puts("=================================");
	printf("Your choice :");
};

void readflag() {
	char buf[100];
	FILE *fp;
	fp = fopen("./flag.txt", "rb");
	if (fp) {
		fread(buf, 40, 1, fp);
		fclose(fp);
		for (int i = 0; i < 40; i++) {
			buf[i] = buf[i] ^ ais3_user.pass;
		}
		printf("Magic : %s\n", buf);
		Sleep(2);
		exit(0);
	}
};


int main()
{
	int password;
	char choice[12];
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	ais3_user.pass = (int)&password;
	puts("======== AIS3 Login sytem ========");
	printf(" Username : ");
	scanf("%s",ais3_user.name);
	printf(" Password : ");
	scanf("%d", &password);
	if (password == ais3_user.pass) {
		puts("Login Success !");
		while (1) {
			menu();
			fgets(choice, 4, stdin);
			switch (atoi(choice)) {
				case 1:
					readflag();
					break;
				case 2 :
					puts("Bye ~");
					exit(0);
					break;
				deafult:
					puts("Invaild choice !");
					break;

			}
		}
	}
	else {
		puts("Sorry ! Try your best !");
		exit(0);
	}

	return 0;
}

