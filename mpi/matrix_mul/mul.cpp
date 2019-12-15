#include<cstdio>
#include<cstring>
#include<mpi.h>
#include<cstdlib>
#include<cmath>
#include<ctime>
using namespace std;
typedef double db;
int mat_to_buf(db **mat, db *buf, int x1, int x2, int y1, int y2)
{
	int t=0;
	for(int i=x1;i<=x2;i++)
		for(int j=y1;j<=y2;j++)
			buf[t++]=mat[i][j];
	return t;
}
int buf_to_mat(db **mat, db *buf, int x1, int x2, int y1, int y2)
{
	int t=0;
	for(int i=x1;i<=x2;i++)
		for(int j=y1;j<=y2;j++)
			mat[i][j]=buf[t++];
	return t;
}
void get_input(int &n, db **&a, db **&b)
{
	int my_rank, comm_sz;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    FILE *f;
	if(my_rank == 0)
	{
        f = fopen("mul.in","rb");
        fread(&n,sizeof(n),1,f);
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	for(int i=1;;i++)
	{
		if(i*i==comm_sz)
		{
			if(n%i)
			{
				if(my_rank == 0)
					printf("n must be divided by sqrt(thread_cnt)\n");
				MPI_Finalize();
				exit(0);
			}
			break;
		}
		else if(i*i>comm_sz)
		{
			if(my_rank == 0)
				printf("p must be a perfect square number\n");
			MPI_Finalize();
			exit(0);
		}
	}
	if(my_rank == 0)
	{
		srand(time(0));
		a = new db*[n];
		b = new db*[n];
		for(int i=0;i<n;i++)
		{
			a[i] = new db[n];
			b[i] = new db[n];
		}
		for(int i=0;i<n;i++)
            fread(a[i],sizeof(db),n,f);
		for(int i=0;i<n;i++)
            fread(b[i],sizeof(db),n,f);
		printf("initialization finished\n");
	}
}
int main()
{
	MPI_Init(NULL, NULL);
	int my_rank, comm_sz;
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	int n;
	db **a, **b;
	get_input(n, a, b);
	int sqrtp = sqrt(comm_sz);
	while(sqrtp*sqrtp<comm_sz)
		sqrtp++;
	while(sqrtp*sqrtp>comm_sz)
		sqrtp--;
	int k = n/sqrtp;
	db **local_c = new db*[k], **local_a = new db*[k], **local_b = new db*[k];
	for(int i=0;i<k;i++)
	{
		local_c[i] = new db[k];
		local_a[i] = new db[k];
		local_b[i] = new db[k];
		for(int j=0;j<k;j++)
			local_c[i][j]=local_a[i][j]=local_b[i][j]=0;
	}
	for(int i=0;i<sqrtp;i++)
	{
		if(my_rank==0)
		{
			printf("step #%d:\n", i);
			for(int j=0;j<comm_sz;j++)
			{
				printf("sendding message to #%d\n", j);
				int x=j/sqrtp;
				int y=(j+i+j/sqrtp)%sqrtp;
				db *buf = new db[k*k];
				mat_to_buf(a, buf, x*k, (x+1)*k-1, y*k, (y+1)*k-1);
				if(j==0)
					buf_to_mat(local_a, buf, 0, k-1, 0, k-1);
				else
					MPI_Send(buf, k*k, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
				y=j%sqrtp;
				x=(j/sqrtp+i+j)%sqrtp;
				mat_to_buf(b, buf, x*k, (x+1)*k-1, y*k, (y+1)*k-1);
				if(j==0)
					buf_to_mat(local_b, buf, 0, k-1, 0, k-1);
				else
					MPI_Send(buf, k*k, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
			}
		}
		else
		{
			db *buf = new db[k*k];
			MPI_Recv(buf, k*k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			buf_to_mat(local_a, buf, 0, k-1, 0, k-1);
			MPI_Recv(buf, k*k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			buf_to_mat(local_b, buf, 0, k-1, 0, k-1);
		}
        int start = clock();
		for(int i=0;i<k;i++)
			for(int j=0;j<k;j++)
				for(int l=0;l<k;l++)
					local_c[i][l]+=local_a[i][j]*local_b[j][l];
        int end = clock();
		if(my_rank==0)
			printf("step #%d finished , time=%.4fs\n", i, (end-start)/double(CLOCKS_PER_SEC));
	}
	db **c = new db*[n];
	for(int i=0;i<n;i++)
		c[i] = new db[n];
	if(my_rank==0)
	{
		for(int i=0;i<k;i++)
			for(int j=0;j<k;j++)
				c[i][j] = local_c[i][j];
		for(int i=1;i<comm_sz;i++)
		{
			db *buf = new db[k*k];
			MPI_Recv(buf, k*k, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			int x = i/sqrtp;
			int y = i%sqrtp;
			buf_to_mat(c, buf, x*k, (x+1)*k-1, y*k, (y+1)*k-1);
		}
//		freopen("mul.out","w",stdout);
//		for(int i=0;i<n;i++)
//		{
//			for(int j=0;j<n;j++)
//				printf("%.0f ", c[i][j]);
//			printf("\n");
//		}
	}
	else
	{
		db *buf = new db[k*k];
		mat_to_buf(local_c, buf, 0, k-1, 0, k-1);
		MPI_Send(buf, k*k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return 0;
}
