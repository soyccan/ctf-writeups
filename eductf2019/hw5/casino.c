#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

// put shellcode at:
// 0x6020d0 - 0x6020e8: 24 bytes
// 0x6020f0 - 0x602108: 24 bytes
int lottery[6] = {0}; // 0x6020b0, not controllable
int guess[6] = {0}; // 0x6020d0, with 8 byte padding
char name[0x10] = {0}; // 0x6020f0
int seed; // 0x602100
int age; // 0x602104

void init(){
    setvbuf(stdout,0,2,0);
    setvbuf(stdin,0,2,0);
    setvbuf(stderr,0,2,0);
    seed = time(0);
}

int read_int(){
    char buf[0x10];
    // __read_chk: read() with buffer size check
    // ssize_t __read_chk(int fd, void * buf, size_t nbytes, size_t buflen);
    __read_chk( 0 , buf , 0xf , 0x10 );
    return atoi( buf );
}

void welcome(){
    puts( "+--------------------+" );
    puts( "|       Casino       |" );
    puts( "+--------------------+" );
    puts( "" );
}


void casino(){

    srand( seed );
    for( int i = 0 ; i < 6 ; ++i ) lottery[i] = rand() % 100;

    int try = 2, idx;

    while( try-- ){
        printf( "\n$$$$$$$ Lottery $$$$$$$\n " );

        for( int i = 0 ; i < 6 ; ++i ){
            printf( "Chose the number %d: " , i );
            guess[i] = read_int();
        }

        printf( "Change the number? [1:yes 0:no]: " );
        if( read_int() == 1 ){
            printf( "Which number [1 ~ 6]: " );
            idx = read_int() - 1; // NOTE: no check idx range
            printf( "Chose the number %d: " , idx );
            guess[idx] = read_int(); // NOTE: out-of-bound read
        }

        for( int i = 0 ; i < 6 ; ++i ){
            if( guess[i] != lottery[i] ) break;
            if( i == 5 ){
                puts( "You win! Hacker don't need luck :P" );
            }
        }
    }

    printf( "You lose.\nBye~\n " );
}


int main(){

    init();
    welcome();

    puts( "Show me your passport." );
    printf( "Your name: " );
    read( 0 , name , 0x100 ); // NOTE: overflow, can control seed

    printf( "Your age: " );
    age = read_int();

    if( age < 20 ){
        puts( "You can not enter the casino!" );
    }
    else{
        casino();
    }

    return 0;
}
