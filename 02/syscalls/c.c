#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
	printf("syscall retval=%ld\n", syscall(__NR_fork));
	return 0;
}
