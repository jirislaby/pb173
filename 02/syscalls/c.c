#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
	syscall(__NR_mkdir, "test");
	return 0;
}
