#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void main()
{
	int fd;
	fd = open("/dev/test",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}
	printf("open ok \n ");
}
