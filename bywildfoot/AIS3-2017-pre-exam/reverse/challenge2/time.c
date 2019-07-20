#include <stdio.h>
#include <time.h>

int main ()
{
   time_t seconds;

   seconds = time(NULL);
   printf("Seconds since January 1, 1970 = %ld", seconds);
  
   return(0);
}

