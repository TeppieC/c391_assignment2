#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

struct mbr {
    int start_x;
    int end_y;
    int length;
};

int main(int argc, char **argv){
	sqlite3 *db; //the database
	sqlite3_stmt *stmt; //the update statement
  	char *zErrMsg = 0;

  	int rc;

    // check the arguments, should read in l
  	if( argc!=3 ){
    	fprintf(stderr, "Usage: %s <database file> <length>\n", argv[0]);
    	return(1);
  	}

  	// open the database
  	rc = sqlite3_open(argv[1], &db);
  	if( rc ){
    	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    	sqlite3_close(db);
    	return(1);
  	}

  	struct mbr mbr_list[100];

  	int length = atoi(argv[2]);

  	// generate the 100 bounding boxes
  	for (int i = 0; i < 100; ++i){
  		// generate a random bounding box
  		mbr_list[i].start_x = rand()%1000; // x of the top-left
  		mbr_list[i].end_y = rand()%1000; // y of the top-left
  		mbr_list[i].length = length;
  	}


 /****************************** rtree method *****************************************/
    // template of the query for all objects within the bounding box
    char *sql_stmt = "SELECT count(r.id)  \
                      FROM rtree_index r \
                      WHERE r.start_x>=? AND r.end_x<=? \
                      AND r.start_y>=?  AND r.end_y<=? ";

    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);

    if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    

    double rtree_total_time = 0;
    // iterate over all 100 bounding boxes
	for (int i = 0; i < 100; ++i){
		
		double box_total_time = 0;
		struct mbr mbrBox = mbr_list[i]; // choose a box

		if (i>0) {
			//clear the bindings, for the new box
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
		}

		// bind the values
    	sqlite3_bind_int(stmt, 1, mbrBox.start_x);
    	sqlite3_bind_int(stmt, 2, mbrBox.start_x+mbrBox.length);
    	sqlite3_bind_int(stmt, 3, mbrBox.end_y-mbrBox.length);
    	sqlite3_bind_int(stmt, 4, mbrBox.end_y);
    	printf(" (%d, %d, %d, %d)\n", mbrBox.start_x, mbrBox.start_x+mbrBox.length,mbrBox.end_y-mbrBox.length, mbrBox.end_y);

    	// execute for 20 times of runs
    	for (int j = 0; j < 20; ++j)
    	{
    		//http://stackoverflow.com/questions/3557221/how-do-i-measure-time-in-c
    		clock_t start = clock(); // start the clock

    		printf("The iteration number: %d\n", j);
    		while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
	          	printf("%s objects contained in this box", sqlite3_column_text(stmt, 0));
    		}
    		
			clock_t end = clock(); // end the clock
			box_total_time += (double)(end - start) / (CLOCKS_PER_SEC/1000); //increase the total time for this box
    	}
    	printf("finished executing box #:%d\n", i);

    	rtree_total_time +=box_total_time/20; // increase the total time for 100 boxes by this box's average time
	}
    printf("Parameter l: %d \n", length);
   	printf("Average runtime with r-tree: %f ms\n", rtree_total_time/100);

    sqlite3_finalize(stmt); //always finalize a statement
  

 /****************************** common indexes method *****************************************/
    sqlite3_close(db);
}

