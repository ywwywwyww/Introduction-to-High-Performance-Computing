#include<cstdio>
#include<cstring>
#include<mpi.h>
#include<cstdlib>
#include<cmath>
#include<ctime>
using namespace std;
int mat_to_buf(int **mat, int *buf, int x1, int x2, int y1, int y2)
{
	int t=0;
	for(int i=x1;i<=x2;i++)
		for(int j=y1;j<=y2;j++)
			buf[t++]=mat[i][j];
	return t;
}
int buf_to_mat(int **mat, int *buf, int x1, int x2, int y1, int y2)
{
	int t=0;
	for(int i=x1;i<=x2;i++)
		for(int j=y1;j<=y2;j++)
			mat[i][j]=buf[t++];
	return t;
}
void get_input(int &n, int **&a, int **&b)
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
		a = new int*[n];
		b = new int*[n];
		for(int i=0;i<n;i++)
		{
			a[i] = new int[n];
			b[i] = new int[n];
		}
		for(int i=0;i<n;i++)
            fread(a[i],sizeof(int),n,f);
		for(int i=0;i<n;i++)
            fread(b[i],sizeof(int),n,f);
		printf("initialization finished\n");
	}
}
void calc(int *local_a, int *local_b, int *local_c, int k)
{
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	for(int i=0;i<k;i++)
		for(int j=0;j<k;j++)
			for(int l=0;l<k;l++)
				local_c[i*k+l]+=local_a[i*k+j]*local_b[j*k+l];
}
int id(int x, int y, int sqrtp)
{
    return x*sqrtp+y;
}
int main()
{
	MPI_Init(NULL, NULL);
	int my_rank, comm_sz;
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    double begin_time;
    if(my_rank==0)
        begin_time=MPI_Wtime();
	int n;
	int **a, **b;
	get_input(n, a, b);
	int sqrtp = sqrt(comm_sz);
	while(sqrtp*sqrtp<comm_sz)
		sqrtp++;
	while(sqrtp*sqrtp>comm_sz)
		sqrtp--;
	int k = n/sqrtp;
	int *local_c = new int[k*k], *local_a = new int[k*k], *local_b = new int[k*k];
    double cur=MPI_Wtime();
	for(int i=0;i<sqrtp;i++)
	{
        if(i==0)
    		if(my_rank==0)
    		{
    			printf("step #%d:\n", i);
    			for(int j=0;j<comm_sz;j++)
    			{
    				printf("sendding message to #%d\n", j);
                    if(j==0)
                    {
                        mat_to_buf(a, local_a, 0, k-1, 0, k-1);
                        mat_to_buf(b, local_b, 0, k-1, 0, k-1);
                    }
                    else
                    {
    				    int x=j/sqrtp;
    				    int y=(j+i+j/sqrtp)%sqrtp;
    				    int *buf = new int[k*k];
    				    mat_to_buf(a, buf, x*k, (x+1)*k-1, y*k, (y+1)*k-1);
    					MPI_Send(buf, k*k, MPI_INT, j, 0, MPI_COMM_WORLD);
    				    y=j%sqrtp;
    				    x=(j/sqrtp+i+j)%sqrtp;
    				    mat_to_buf(b, buf, x*k, (x+1)*k-1, y*k, (y+1)*k-1);
    					MPI_Send(buf, k*k, MPI_INT, j, 0, MPI_COMM_WORLD);
                    }
    			}
    		}
    		else
    		{
    			MPI_Recv(local_a, k*k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Recv(local_b, k*k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		}
        else
        {
            if(my_rank==0)
                printf("step #%d:\n", i);
            int x=my_rank/sqrtp;
            int y=my_rank%sqrtp;
            MPI_Sendrecv_replace(local_a, k*k, MPI_INT, id(x, (y+sqrtp-1)%sqrtp, sqrtp), 0, id(x, (y+1)%sqrtp, sqrtp), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(local_b, k*k, MPI_INT, id((x+sqrtp-1)%sqrtp, y, sqrtp), 0, id((x+1)%sqrtp, y, sqrtp), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if(my_rank==0)
        {
            printf("data transfer time : %.5f\n", MPI_Wtime()-cur);
            cur = MPI_Wtime();
        }
        calc(local_a, local_b, local_c, k);
        if(my_rank==0)
        {
            printf("calc mat mul time : %.5f\n", MPI_Wtime()-cur);
            cur=MPI_Wtime();
        }
	}
	int **c = new int*[n];
	for(int i=0;i<n;i++)
		c[i] = new int[n];
	if(my_rank==0)
	{
		for(int i=0;i<k;i++)
			for(int j=0;j<k;j++)
				c[i][j] = local_c[i*k+j];
		for(int i=1;i<comm_sz;i++)
		{
			int *buf = new int[k*k];
			MPI_Recv(buf, k*k, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
        double end_time = MPI_Wtime();
        printf("total time cost : %.5f\n", end_time-begin_time);
	}
	else
		MPI_Send(local_c, k*k, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}
