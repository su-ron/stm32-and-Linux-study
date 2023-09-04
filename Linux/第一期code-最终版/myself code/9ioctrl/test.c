#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include"beep.h"
void main()
{
	int fd;
	int len;
	
	fd = open("/dev/test",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}

    ioctl(fd,DEV_FIFO_CLEAN);

	ioctl(fd,DEV_FIFO_GETVALUE,&num);
	printf("num=%d\n",num);

	int num=77;
	ioctl(fd,DEV_FIFO_SETVALUE,&num);
	printf("num=%d\n",num);
	
	printf("open ok \n ");
}
