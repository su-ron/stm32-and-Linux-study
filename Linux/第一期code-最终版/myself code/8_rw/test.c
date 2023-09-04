#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void main()
{
	int fd;
	char buf[64]={0};
	char buf2[64]="suronjian";
	int len;
	
	fd = open("/dev/test",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}

	len=read(fd,buf,64);
    buf[len]='\0';
	printf("read:%s\n len=%d\n",buf,len);

	strcpy(buf,"su");
	len=write(fd,buf2,strlen(buf2));
	printf("len=%d\n",len);
	
	printf("open ok \n ");
}
