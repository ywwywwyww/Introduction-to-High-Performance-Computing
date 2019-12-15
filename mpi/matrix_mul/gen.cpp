#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<ctime>
using namespace std;
int main()
{
	FILE *f=fopen("mul.in","wb");
	int n=8192;
    fwrite(&n,sizeof(n),1,f);
	for(int i=1;i<=n;i++)
	{
		for(int j=1;j<=n;j++)
        {
            int x=rand()%10;
            fwrite(&x,sizeof(x),1,f);
        }
        char ch = '\n';
        fwrite(&ch,sizeof(ch),1,f);
	}
	for(int i=1;i<=n;i++)
	{
		for(int j=1;j<=n;j++)
        {
            int x=rand()%10;
            fwrite(&x,sizeof(x),1,f);
        }
        char ch = '\n';
        fwrite(&ch,sizeof(ch),1,f);
	}
	return 0;
}
