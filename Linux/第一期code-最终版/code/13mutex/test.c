/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
main()
{
	int fd;
	int len;
	char buf[64]={0};
	char buf2[64+1]="peng";
	
	
	fd = open("/dev/hellodev",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}
	printf("before sleep\n");
	sleep(15);
	printf("after sleep\n");
	
	 
	close(fd);
}
