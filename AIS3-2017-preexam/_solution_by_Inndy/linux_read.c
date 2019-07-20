#include <unistd.h>

#define SYS_sendfile 40

int main()
{
	int fd = open("misc4.md", 0);
	size_t a = 0;
	syscall(SYS_sendfile, 1, fd, 0, 55);
}
