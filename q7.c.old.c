#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

/* Double function that returns the square of a number */
void my_function(sqlite3_context* ctx, int nargs, sqlite3_value** values){
  double x = sqlite3_value_double(values[0]);
  double y = x*x;
  sqlite3_result_double(ctx, y);
}

//https://webdocs.cs.ualberta.ca/~denilson/teaching/cmput391/sample_functions.c
void print_result(sqlite3_stmt *stmt){
	int rc;

	while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int col;
		for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
			printf("%s|", sqlite3_column_text(stmt, col));
		}
		printf("%s", sqlite3_column_text(stmt, col));
		printf("\n");
	}
}

Struct Node *extractNodes(char *nodeString, Struct Node *branchList){
  const char s[2] = " ";
  char *token;
  char* result[300];

  int i = 0;
  token = strtok(nodeString, s);
  while(token != NULL){

    if (token[0] == "{"){
      memmove(token, token+1, strlen(token));
    }
    printf("%s\n", token);

    result[i] = token;
    token = strtok(NULL, s);
    i+=1;
  }

  int j=0;
  int n=0;
  for(j=0;j<i;j+=5){
    branchList[n].node_index = result[j];//may need to cast--> atoi()
    branchList[n].node_index = result[j+1];//may need to cast--> double()
    branchList[n].node_index = result[j+2];
    branchList[n].node_index = result[j+3];
    branchList[n].node_index = result[j+4];
    n+=1;
  }
}

void genBranchList(sqlite3 *db, Struct Point p, Struct Node node, Struct Node* branchList){
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT rtreenode(2,data) FROM rtree_index_node WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  //print_result(stmt);
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    printf("%s", sqlite3_column_text(stmt, 0));
    char *result = sqlite3_column_text(stmt, 0); // parse from the query result
    extractNodes(result, branchList);// extract the result into the branchList
    printf("\n");
  }

  int i=0;
  for(i=0;i<200;i++){
    if (branchList[i]==NULL){ // may raise error, because I don't know how to determine if the cell is blank!!!!!!!!!!!!!!!!!!!!!!!
      break;
    }
    // parameter: branchList[i] is a Node struct, p is a Point struct
    branchList[i].dist = minDist(branchList[i], p);
  }
}

double cmpfunc (const void * a, const void * b){
  return ( *(double*)a.dist - *(double*)b.dist);
}

void sortBranchList(sqlite3 *db, Struct Node* branchList){
  int length = sizeof(branchList)/sizeof(Struct Node);//aquiring the length of the branchList
  qsort(branchList, length, sizeof(Struct Node), cmpfunc); 
}

int main(int argc, char **argv){
  sqlite3 *db;
  sqlite3_stmt *stmt;
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

  // deal with the parameters
  char *x1 = argv[2];
  char *y1 = argv[3];
  printf("Querying for the point (%s, %s)\n", x1,y1);


  /******** C program for NN searching algorithm ******************/
  
  
  sqlite3_close(db);
}
