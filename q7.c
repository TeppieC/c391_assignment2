#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

int main(int argc, char **argv){
	sqlite3 *db; //the database
  sqlite3_stmt *stmt; //the update statement
  char *zErrMsg = 0;

  int rc;

  // check the arguments
  if( argc!=2 ){
    fprintf(stderr, "Usage: %s <database file> \n", argv[0]);
  	return(1);
	}

  // open the database
  rc = sqlite3_open(argv[1], &db);
	if( rc ){
  	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  	sqlite3_close(db);
  	return(1);
  }

/************** generating coordinates for all nodes **********************************/

  char *sql_gen_coords = "CREATE TABLE innerNodes AS\
                              SELECT r.nodeno as nodeno, \
                                MIN(p.start_x) as start_x, \
                                MAX(p.end_x) as end_x, \
                                MIN(p.start_y) as start_y, \
                                MAX(p.end_y) as end_y \
                              FROM rtree_index p, rtree_index_rowid r \
                              WHERE p.id = r.rowid \
                              GROUP BY r.nodeno";

  rc = sqlite3_exec(db, sql_gen_coords, 0, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
  fprintf(stderr, "SQL error: %s\n", zErrMsg);
     sqlite3_free(zErrMsg);
  }else{
     fprintf(stdout, "table created successful\n");
  }

  char * sql_insert_coords = "INSERT INTO innerNodes(nodeno, start_x, end_x, start_y, end_y) \
                                SELECT  p.parentnode as nodeno, \
                                        MIN(n.start_x) as start_x, \
                                        MAX(n.end_x) as end_x, \
                                        MIN(n.start_y) as start_y, \
                                        MAX(n.end_y) as end_y \
                                  FROM rtree_index_parent p, innerNodes n \
                                  WHERE p.nodeno = n.nodeno \
                                  GROUP BY p.parentnode";
  while(1){

    sqlite3_close(db);
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }

    rc = sqlite3_exec(db, sql_insert_coords, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
    }else{
       fprintf(stdout, "table created successful\n");
    }
    printf("%d\n", sqlite3_total_changes(db));
    if (sqlite3_total_changes(db)==1){
      printf("iteriation over.\n");
      break;
    }
  }

/*
  // deal with the parameters
  char *x1 = argv[2];
  char *y1 = argv[3];
  char *x2 = argv[4];
  char *y2 = argv[5];
  char *c = argv[6];
  printf("Querying the bounding rectangle (%s,%s), (%s,%s), of class: %s\n", x1,y1,x2,y2,c);


  // query for all objects within the bounding box
  char *sql_stmt = "SELECT p.* \
                      FROM rtree_index r, poi_tag p \
                      WHERE r.start_x>=? AND r.end_x<=? \
                      AND r.start_y>=?  AND r.end_y<=? \
                      AND p.id = r.id \
                      AND p.key='class' AND p.value=?;";

    // Allocates storage
    //char *sql = (char*)malloc(300 * sizeof(char));
    // Prints "Hello world!" on hello_world
    //sprintf(sql, sql_stmt, x1, x2, y1, y2, c);
    //printf("%s\n", sql);

    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);

    if (rc != SQLITE_OK) {  
          fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db);
          return 1;
    }    

    // bind the value 
    sqlite3_bind_text(stmt, 1, x1, strlen(x1), 0);
    sqlite3_bind_text(stmt, 2, x2, strlen(x2), 0);
    sqlite3_bind_text(stmt, 3, y1, strlen(y1), 0);
    sqlite3_bind_text(stmt, 4, y2, strlen(y2), 0);
    sqlite3_bind_text(stmt, 5, c, strlen(c), 0);
    
    int count = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
          int col;
          count+=1;
          for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
            printf("%s|", sqlite3_column_text(stmt, col));
          }
          printf("%s", sqlite3_column_text(stmt, col));
          printf("\n");
    }
    printf("%d rows found\n", count);
    sqlite3_finalize(stmt); //always finalize a statement
  */
    //free(sql);
    sqlite3_close(db);
}