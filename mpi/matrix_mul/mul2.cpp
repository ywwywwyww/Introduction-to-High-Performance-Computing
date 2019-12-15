#include<cstdio>
#include<cstring>
using namespace std;
typedef double db;
const int N=1010;
db a[N][N],b[N][N],c[N][N];
int main()
{
	freopen("mul.in","r",stdin);
	freopen("mul2.out","w",stdout);
	int n;
	scanf("%d",&n);
	for(int i=1;i<=n;i++)
		for(int j=1;j<=n;j++)
			scanf("%lf",&a[i][j]);
	for(int i=1;i<=n;i++)
		for(int j=1;j<=n;j++)
			scanf("%lf",&b[i][j]);
	for(int i=1;i<=n;i++)
		for(int j=1;j<=n;j++)
			for(int k=1;k<=n;k++)
				c[i][k]+=a[i][j]*b[j][k];
	for(int i=1;i<=n;i++)
	{
		for(int j=1;j<=n;j++)
			printf("%.0f ",c[i][j]);
		printf("\n");
	}
	return 0;
}
