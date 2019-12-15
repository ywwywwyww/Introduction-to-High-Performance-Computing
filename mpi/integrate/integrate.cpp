#include<cstdio>
#include<cstring>
#include<mpi.h>
#include<cmath>
using namespace std;
typedef double db;
db f(db x)
{
	return sin(x);
}
int main()
{
	int comm_sz;
	int my_rank;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	db a, b;
	int n;
	if(my_rank==0)
	{
		scanf("%lf%lf%d",&a,&b,&n);
		MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	db local_n = n / comm_sz;
	db h = (b-a) / n;
	db local_a = a + my_rank*local_n*h;
	db local_b = local_a + local_n*h;
	db local_h = (local_b - local_a) / local_n;
	db approx = (f(local_a) + f(local_b)) / 2.0;
	for(int i=1;i<=local_n-1;i++)
	{
		db xi = local_a + i * local_h;
		approx += f(xi);
	}
	approx *= local_h;
	db sum=0;
	if(my_rank!=0)
	{
		MPI_Reduce(&approx, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Reduce(&approx, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
		printf("the answer is %.10f\n", sum);
	}
	MPI_Finalize();
	return 0;
}
