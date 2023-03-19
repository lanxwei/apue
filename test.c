#include <stdio.h>
#include <string.h>
int ccc(char *ptr);

int main(int argc,char **argv)
{
	int rv=-1;
	char *ptr;
	char buf[]="awdawd";
	ptr=buf;
	printf("%s\n",ptr);
	memset(ptr,0,sizeof(buf));
	printf("%s\n",ptr);
	if(*ptr == 0)
	{
		printf("123\n");
	}

}

int  ccc(char *ptr)
{
	int i=0;
	for(i=0;i<10;i++)
	{
		*(ptr+i)='a';
	}
	printf("ptr:%s\n",ptr);
}
