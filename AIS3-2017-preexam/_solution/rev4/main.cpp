#include <cstdio>
#include <algorithm>
using namespace std;

long long int matrix[1024][1024];

int main()
{
	FILE *fp = fopen("data", "rb");
	fread(matrix, 8, 1024 * 1024, fp);

	for(int i = 1 ; i < 1024 ; ++i)
		matrix[0][i] += matrix[0][i-1];
	
	for(int i = 1 ; i < 1024 ; ++i)
		matrix[i][0] += matrix[i-1][0];

	for(int i = 1 ; i < 1024 ; ++i)
		for(int j = 1 ; j < 1024 ; ++j)
			matrix[i][j] += min(matrix[i-1][j], matrix[i][j-1]);

	printf("%lld\n", matrix[1023][1023]);
	return 0;
}

