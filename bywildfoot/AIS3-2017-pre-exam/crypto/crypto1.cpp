#include <stdio.h>
#include <string.h>

int main()
{
	int i, *ptr;
	char flag[29] = "1112????????????????????????"; // Hint: The flag begins with AIS3
	
	for(i = 0, ptr = (int*)flag ; i < 17 ; ++i)
		printf("%d\n", ptr[i]);
	
	/*
	964600246
	1376627084
	1208859320
	1482862807
	1326295511
	1181531558
	2003814564
	*/
	
	return 0;
}

