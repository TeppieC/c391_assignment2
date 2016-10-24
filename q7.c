#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

#define MIN(a,b) ((a) > (b) ? a : b)
#define MAX(a,b) ((a) < (b) ? a : b) 

struct Node {
  int node_index;
  int count;
  double mindist;
  double minmaxdist;
  double minX;
  double minY;
  double maxX;
  double maxY;
};

struct Point {
  double x;
  double y;
};


double sqrt(double a) {
  return a * a;
}


// calculate the minimal distance from the point to the mbr
double minDist (struct Node node, struct Point p) {
  double mindist;
  // if the point is inside the mbr 
  if(p.x >= node.minX && p.x <= node.maxX && p.y >= node.minY && p.y <= node.maxY) {
    mindist = 0;
  }
  // if the point is outside mbr
  else {
    // if the point is within range 
    if(p.y < node.maxY && p.y > node.minY) {
      if(p.x > node.maxX) {
        mindist = sqrt(p.x - node.maxX);
      }
      else if(p.x < node.minX) {
        mindist = sqrt(node.minX - p.x);
      }
    } 
    if(p.x > node.minX && p.x < node.maxX) {
      if(p.y > node.maxY) {
        mindist = sqrt(p.y - node.maxY);
      }
      else if(p.y < node.minY) {
        mindist = sqrt(node.minY - p.y);
      }
      //mindist = MIN((p.y-node.maxY)*(p.y-node.maxY), (p.y-node.minY)*(p.y-node.minY));
    }
    else {
      if(p.x > node.maxX) {
        mindist  = MIN(sqrt(p.y-node.maxY)+sqrt(p.x-node.maxX), sqrt(p.y-node.minY)+sqrt(p.x-node.minX));
      }
      if(p.x < node.minX) {
        mindist = MIN(sqrt(p.y-node.maxY)+sqrt(p.x-node.minX), sqrt(p.y-node.minY)+sqrt(p.x-node.minX));
      }
    }
  }
  return mindist;
}

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

//struct Node *extractNodes(char *nodeString, struct Node *branchList){
void extractNodes(char *nodeString, struct Node *branchList){
  const char s[2] = " ";
  char *token;
  char* result[300];

  int i = 0;
  token = strtok(nodeString, s);
  while(token != NULL){

    if (token[0] == '{'){
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
    branchList[n].node_index = *result[j];//may need to cast--> atoi()
    branchList[n].node_index = *result[j+1];//may need to cast--> double()
    branchList[n].node_index = *result[j+2];
    branchList[n].node_index = *result[j+3];
    branchList[n].node_index = *result[j+4];
    n+=1;
  }
}

void genBranchList(sqlite3 *db, struct Point p, struct Node node, struct Node* branchList){
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT rtreenode(2,data) FROM rtree_index_node WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  //print_result(stmt);
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    printf("%s", sqlite3_column_text(stmt, 0));
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    extractNodes(result, branchList);// extract the result into the branchList
    printf("\n");
  }

  int i=0;
  for(i=0;i<200;i++){
    if (branchList[i].count == 0) { // may raise error, because I don't know how to determine if the cell is blank!!!!!!!!!!!!!!!!!!!!!!!
      break;
    }
    // parameter: branchList[i] is a Node struct, p is a Point struct
    branchList[i].mindist = minDist(branchList[i], p);
  }
}

int cmpfunc (const void * a, const void * b){
  return (((struct Node*)a)->mindist - ((struct Node*)b)->mindist);
}

void sortBranchList(sqlite3 *db, struct Node* branchList){
  int length = sizeof(branchList)/sizeof(struct Node);//aquiring the length of the branchList
  qsort(branchList, length, sizeof(struct Node), cmpfunc); 
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
  
  struct Point poi;
  struct Node node;
  struct Node branchList[200];
  poi.x = atoi(x1);
  poi.y = atoi(y1);
  poi = *(struct Point *)malloc(sizeof(struct Point));
  node = *(struct Node *)malloc(sizeof(struct Node));
  genBranchList(db, poi, node, branchList);

  // import from rtree

  sqlite3_close(db);
}
