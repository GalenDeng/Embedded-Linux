#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*
* firstdrvtest on  打开 
* firstdrvtest off 关闭
*/


int main(int argc,char **argv)
{
	int fd;
	int val=0;
	fd =open("/dev/xyz",O_RDWR);
	if (fd < 0)
		printf("can't open!\n");
        if (argc != 2)
        {
                printf("usage :\n");
                printf("%s <on|off>\n",argv[0]);
                return 0;
        }

        if (strcmp(argv[1],"on") == 0)    // 如果第二个参数 == on 的时候
	{
		val = 1;
	}
	else
	{
		val = 0;
	}

	write(fd,&val,4);
	return 0;
}
