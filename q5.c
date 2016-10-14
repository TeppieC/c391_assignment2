#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

// return a number within 0-1000 as the top-left corner of the bounding box
int random_num(){
  	srand ( time(NULL) );
  	int random_number = rand()%1000;
  	return random_number;
}



