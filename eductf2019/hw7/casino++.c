#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>


int lottery[6] = {0}, guess[6] = {0};
char name[0x10] = {0};
int age, seed;

// objdump -j .bss -t casino++
// 0000000000602078 g       .bss   0000000000000000 __bss_start
// 0000000000602080 l    d  .bss   0000000000000000 .bss
// 0000000000602080 g     O .bss   0000000000000008 stdout@@GLIBC_2.2.5
// 0000000000602090 g     O .bss   0000000000000008 stdin@@GLIBC_2.2.5
// 00000000006020a0 g     O .bss   0000000000000008 stderr@@GLIBC_2.2.5
// 00000000006020a8 l     O .bss   0000000000000001 completed.7697
// 00000000006020b0 g     O .bss   0000000000000018 lottery
// 00000000006020d0 g     O .bss   0000000000000018 guess
// 00000000006020f0 g     O .bss   0000000000000010 name
// 0000000000602100 g     O .bss   0000000000000004 seed
// 0000000000602104 g     O .bss   0000000000000004 age
// 0000000000602108 g       .bss   0000000000000000 _end

void init(){
    setvbuf(stdout,0,2,0);
    setvbuf(stdin,0,2,0);
    setvbuf(stderr,0,2,0);
    seed = time(0);
}

int read_int(){
    char buf[0x10];
    __read_chk( 0 , buf , 0xf , 0x10 ); // read: 15bytes(incl:\n) + \x1b -> buf
    // TODO: hijack GOT: __read_chk -> read
    return atoi( buf );
}

void welcome(){
    puts( "+--------------------+" );
    puts( "|       Casino       |" );
    puts( "+--------------------+" );
    puts( "" );
}


void casino(){

    srand( seed ); // TODO: srand -> system
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
            idx = read_int() - 1;
            printf( "Chose the number %d: " , idx );
            guess[idx] = read_int();
        }

        for( int i = 0 ; i < 6 ; ++i ){
            if( guess[i] != lottery[i] ) break;
            if( i == 5 ){
                puts( "You win! Hacker don't need luck :P" );
                // TODO: hijack GOT: puts -> casino
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
    read( 0 , name , 0x100 );

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
