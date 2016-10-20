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
  if( argc!=4 ){
    fprintf(stderr, "Usage: %s <database file> <x1> <y1>\n", argv[0]);
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
                                  WHERE p.nodeno = n.nodeno AND \
                                        p.parentnode NOT IN (select nodeno from innerNodes) \
                                  GROUP BY p.parentnode";
  int loops = 0;
  while(loops<2){

    rc = sqlite3_exec(db, sql_insert_coords, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
    }else{
       fprintf(stdout, "table updated successful\n");
    }
    loops+=1;

    /**** next time ***********/
    // check # of rows inserted
    //char *sql_stmt_check = "SELECT count(distinct n.nodeno)-count(distinct i.nodeno) \
                            FROM innerNodes i, rtree_index_node n";
    /*
    rc = sqlite3_prepare_v2(db, sql_stmt_check, -1, &stmt, 0);
    int change = -1;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%s\n", sqlite3_column_text(stmt, 0));
        change = atoi((char *)sqlite3_column_text(stmt, 0));
    }
    if (change==0){
      printf("iteriation over.\n");
      break;
    }*/
  }

  // deal with the parameters
  char *x1 = argv[2];
  char *y1 = argv[3];
  printf("Querying for the point (%s, %s)\n", x1,y1);


  /******** C program for NN searching algorithm ******************/
  

  sqlite3_close(db);
}