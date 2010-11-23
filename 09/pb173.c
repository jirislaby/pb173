#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

static unsigned long PAGE_SIZE;

int main(int argc, char **argv)
{
	char *map1, *map2;
	int fd;

	PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
	if ((long)PAGE_SIZE < 0)
		err(1, "sysconf(_SC_PAGE_SIZE)");

	if (argc < 2)
		errx(2, "usage: %s <dev_node>", argv[0]);

	fd = open(argv[1], O_RDWR);
	if (fd < 0)
		err(3, "cannot open '%s'", argv[1]);

	map1 = mmap(NULL, PAGE_SIZE * 2, PROT_READ, MAP_SHARED, fd, 0);
	if (map1 == MAP_FAILED)
		err(4, "mmap1");

	map2 = mmap(NULL, PAGE_SIZE * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
			PAGE_SIZE * 2);
	if (map2 == MAP_FAILED)
		err(5, "mmap2");

	puts("map1:");
	puts(map1);
	puts(map1 + PAGE_SIZE);
	puts("map2:");
	puts(map2);
	puts(map2 + PAGE_SIZE);
	strcpy(map2, "Hello1");
	strcpy(map2 + PAGE_SIZE, "Hello2");
	puts("modified map2:");
	puts(map2);
	puts(map2 + PAGE_SIZE);

	munmap(map2, PAGE_SIZE * 2);
	munmap(map1, PAGE_SIZE * 2);
	close(fd);

	return 0;
}
