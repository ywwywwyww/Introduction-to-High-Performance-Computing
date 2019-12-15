#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<ctime>
using namespace std;
int main()
{
	freopen("mul.in","w",stdout);
	int n=1000;
	printf("%d\n",n);
	for(int i=1;i<=n;i++)
	{
		for(int j=1;j<=n;j++)
			printf("%d ",rand()%10);
		printf("\n");
	}
	for(int i=1;i<=n;i++)
	{
		for(int j=1;j<=n;j++)
			printf("%d ",rand()%10);
		printf("\n");
	}
	return 0;
}
