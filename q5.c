#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

/***
  gcc -g q5.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
***/
struct mbr {
  double start_x;
  double end_y;
  int length;
};

int main(int argc, char **argv){
	sqlite3 *db; //the database
	sqlite3_stmt *stmt; //the update statement
	sqlite3_stmt *stmt2;
  char *zErrMsg = 0;
  int i,j;
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


  /**************** generating 100boxes ********************************************/
	struct mbr mbr_list[100];

	int length = atoi(argv[2]);

	// generate the 100 bounding boxes
	for (int i = 0; i < 100; ++i){
		// generate a random bounding box
		mbr_list[i].start_x = (double)(rand()%1000); // x of the top-left
		mbr_list[i].end_y = (double)(rand()%1000); // y of the top-left
		mbr_list[i].length = length;
	}


 /****************************** rtree method *****************************************/
  // template of the query for all objects within the bounding box
  char *sql_stmt = "SELECT count(r.id)  \
                    FROM rtree_index r \
                    WHERE r.start_x>=? AND r.end_x<=? \
                    AND r.start_y>=?  AND r.end_y<=? ";

  double rtree_total_time = 0;
    // iterate over all 100 bounding boxes
	for (i = 0; i < 20; ++i){

    //http://stackoverflow.com/questions/3557221/how-do-i-measure-time-in-c
    clock_t start = clock(); // start the clock
    	
    // execute for 100 boxes
    for (j = 0; j < 100; ++j)
    	{
      struct mbr mbrBox = mbr_list[j]; // choose a box
        
      if (j>0) {
        //clear the bindings, for the new box
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
      }

      rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
      if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
      }    

      // bind the values
      sqlite3_bind_double(stmt, 1, mbrBox.start_x);
      sqlite3_bind_double(stmt, 2, mbrBox.start_x+mbrBox.length);
      sqlite3_bind_double(stmt, 3, mbrBox.end_y-mbrBox.length);
      sqlite3_bind_double(stmt, 4, mbrBox.end_y);
      //printf(" (%d, %d, %d, %d)\n", mbrBox.start_x, mbrBox.start_x+mbrBox.length,mbrBox.end_y-mbrBox.length, mbrBox.end_y);
    	//printf("The iteration number: %d\n", j);
    	while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // execute the query
	      //printf("%s objects contained in this box", sqlite3_column_text(stmt, 0));
    	}
    		
    }
    //printf("finished executing box #:%d\n", i);
    clock_t end = clock(); // end the clock
    rtree_total_time += (double)(end - start) / (CLOCKS_PER_SEC/1000); //increase the total time for this box
	}

  sqlite3_finalize(stmt); //always finalize a statement



 /****************************** common indexes method *****************************************/
    // template of the query for all objects within the bounding box
  /* we didn't store the end_x and end_y for the poi/poi_cartesian tables. 
    -- 10m of latitude in cartesian coordination system: 10/(111191*(48.2490-48.0600)/1000) = 0.4758483412331294
    -- 10m of longitude in cartesian coordination system: 10/(74539*(11.7240-11.3580)/1000) = 0.3665517966646282
    Therefore, end_x is start_x+0.3665517966646282, start_y = end_y-0.4758483412331294
    */
  char *sql_stmt2 = "SELECT count(p.id)  \
                      FROM poi_cartesian p \
                      WHERE p.start_x>=? AND p.start_x+0.3665517966646282<=? \
                      AND p.end_y-0.4758483412331294>=? AND p.end_y<=? ";

  double index_total_time = 0;
  // iterate over all 100 bounding boxes
  for (i = 0; i < 20; ++i){

    //http://stackoverflow.com/questions/3557221/how-do-i-measure-time-in-c
    clock_t start = clock(); // start the clock
      
    // execute for 100 boxes
    for (j = 0; j < 100; ++j)
      {
      struct mbr mbrBox = mbr_list[j]; // choose a box
        
      if (j>0) {
        //clear the bindings, for the new box
        sqlite3_reset(stmt2);
        sqlite3_clear_bindings(stmt2);
      }

      rc = sqlite3_prepare_v2(db, sql_stmt2, -1, &stmt2, 0);
      if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
      }    

      // bind the values
      sqlite3_bind_double(stmt2, 1, mbrBox.start_x);
      sqlite3_bind_double(stmt2, 2, mbrBox.start_x+mbrBox.length);
      sqlite3_bind_double(stmt2, 3, mbrBox.end_y-mbrBox.length);
      sqlite3_bind_double(stmt2, 4, mbrBox.end_y);
      //printf(" (%d, %d, %d, %d)\n", mbrBox.start_x, mbrBox.start_x+mbrBox.length,mbrBox.end_y-mbrBox.length, mbrBox.end_y);
      //printf("The iteration number: %d\n", j);
      while((rc = sqlite3_step(stmt2)) == SQLITE_ROW) {
        // execute the query
        //printf("%s objects contained in this box", sqlite3_column_text(stmt, 0));
      }
        
    }
    //printf("finished executing box #:%d\n", i);
    clock_t end = clock(); // end the clock
    index_total_time += (double)(end - start) / (CLOCKS_PER_SEC/1000); //increase the total time for this box
  }

  sqlite3_finalize(stmt2); //always finalize a statement



    printf("Parameter l: %d \n", length);
   	printf("Average runtime with r-tree: %f ms\n", rtree_total_time/20);
   	printf("Average runtime without r-tree: %f ms\n", index_total_time/20);

    sqlite3_finalize(stmt2); //always finalize a statement

  sqlite3_close(db);
}

