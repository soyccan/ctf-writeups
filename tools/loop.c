#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int pid = -1;
int catched_usr1 = 0;

void onusr1(int signo) {
    if (pid > 0 && kill(pid, 0) >= 0) {
        // if proccess exists
        kill(pid, SIGTERM);
        wait(NULL);
        pid = -1;
    }
    catched_usr1 = 1;
}

void setblock(int signo) {
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, signo);
    sigprocmask(SIG_BLOCK, &s, NULL);
}

void setunblock(int signo) {
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, signo);
    sigprocmask(SIG_UNBLOCK, &s, NULL);
}

void waitfor(int signo) {
    sigset_t s;
    sigprocmask(SIG_BLOCK, NULL, &s);
    sigdelset(&s, signo);
    sigsuspend(&s);
}

int main() {
    setblock(SIGUSR1);
    signal(SIGUSR1, onusr1);
    printf("%d\n", getpid());
    while (1) {
        while (!catched_usr1)
            waitfor(SIGUSR1);
        catched_usr1 = 0;

        pid = fork();
        if (pid < 0)
            continue;
        else if (pid == 0)
            execlp("python", "python", "exp.py", NULL);
        else {
            setunblock(SIGUSR1);
            if (pid > 0)
                wait(NULL);
            setblock(SIGUSR1);
            pid = -1;
        }
    }
    return 0;
}
