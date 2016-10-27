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

struct Rect {
  int id;
  double minX;
  double minY;
  double maxX;
  double maxY;
  double dist;
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

double minMaxDist(struct Node node, struct Point p){
  double minMaxDist;
  /* TODO */
  return minMaxDist;
}

double objectDist(struct Point poi, double minX, double maxX, double minY, double maxY){
  /* TODO: compute the distance between point poi and the rect*/
  int dist=0;

  return dist;
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

int lengthOfList(struct Node* branchList){//?????????????????????????????????????????????????????????????????????????????????????????????
  /* Return the length of a branchList */
  int length = sizeof(branchList)/sizeof(struct Node);//aquiring the length of the branchList
  return length;
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

  int length = lengthOfList(branchList);
  for(i=0;i<length;i++){    // parameter: branchList[i] is a Node struct, p is a Point struct
    branchList[i].mindist = minDist(branchList[i], p);
  }
}

void genChildrenObject(sqlite3 *db, struct Node node, int* children){
  /* Return the id's of the children of a leaf node */

  int rc;
  sqlite3_stmt *stmt;
  //int* children; ////////

  char *sql_stmt = "SELECT rowid FROM rtree_index_rowid WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  //print_result(stmt);
  int i = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    printf("%s", sqlite3_column_text(stmt, 0));
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    children[i] = atoi(result); /////////////////////////// is it okay??????
    i+=1;
    printf("\n");
  }
}

void getRect(sqlite3 *db, int rectId){
  /* Query for the coordinates given the id of a rectangle */

  int rc;
  sqlite3_stmt *stmt;
  double rect[4];////////////////////////////???????????????????????????

  char *sql_stmt = "SELECT start_X, end_X, start_Y, end_Y FROM rtree_index WHERE id=?";
  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  sqlite3_bind_int(stmt, 1, rectId);
  
  int i = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    //printf("%s", sqlite3_column_text(stmt, 0));
    int col;
    for(col=0; col<sqlite3_column_count(stmt); col++) {
      rect[col] = atof((char *)sqlite3_column_text(stmt, col));
    }

    i+=1;
    //printf("\n");
  }
  //return rect;
}

int cmpfunc (const void * a, const void * b){
  const struct Node * aa = a;
  const struct Node * bb = b;

  if (aa->mindist > bb->mindist){
    return 1;
  }else if (aa->mindist == bb->mindist){
    return 0;
  }else{
    return -1;
  }
}

void sortBranchList(struct Node* branchList){
  int length = sizeof(branchList)/sizeof(struct Node);//aquiring the length of the branchList
  qsort(branchList, length, sizeof(struct Node), cmpfunc); 
}

int pruneBranchList(sqlite3 *db, struct Node node, struct Point poi, struct Rect nearest, struct Node* branchList){
  /* TODO */
  int last = 0;

  return last;
}

int LeafCount(sqlite3 *db, struct Node node){
  /* Return the # of children of a leaf node. 
    If it is not a leaf node, return 0 instead. */

  int count;
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT count(rowid) FROM rtree_index_rowid WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  //print_result(stmt);
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    printf("Leaf Node's Children Count: %s", sqlite3_column_text(stmt, 0));
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    count = atoi(result); /////////////////////////// is it okay??????
    printf("\n");
  }

  return count;
}


void nearestNeighborSearch(sqlite3 *db, struct Node node, struct Point poi, struct Rect nearest){//should parameter be pointers????????????????????
  //struct Point poi;
  struct Node newNode;
  struct Node branchList[200];
  int dist;
  int last;
  int i,j;

  int leafCount;
  leafCount = LeafCount(db, node);
  if (leafCount>0)
  {
    int children[leafCount]; //int children[lrafCount] -> double children[leafCount]
    genChildrenObject(db, node, children);
    
    struct Rect nearest;
    nearest.id = children[0];//init
    double rect[4];  //assign 4 elements to rect????
    getRect(db, children[i]); // where i comes from with no inital value??????? and never increase????
    for(j=0; j<4; j++) {
      rect[i] = (double)children[j];
    }
    nearest.dist = objectDist(poi, rect[0], rect[1], rect[2], rect[3]);
    for (i = 0; i < leafCount; ++i)
    {
      //rect[4] = getRect(db, children[i]);//////////////////////////
      getRect(db, children[i]);
      rect[i] = (double)children[i];
      dist = objectDist(poi, rect[0], rect[1], rect[2], rect[3]);
      if (dist<nearest.dist)
      {
        nearest.id = children[i];
        nearest.minX = rect[0];
        nearest.maxX = rect[1];
        nearest.minY = rect[2];
        nearest.maxY = rect[3];
        nearest.dist = dist;
      }
    }
  }else{
    genBranchList(db, poi, node, branchList);
    sortBranchList(branchList);
    //Perform Downward Pruning ???????????????????????????????????????????????
    last = pruneBranchList(db, node, poi, nearest, branchList); //this will require dynamically change the branchlist how??????????????

    for (i = 0; i < last; ++i)
    {
      newNode = branchList[i];

      //Recursively visit chile nodes
      nearestNeighborSearch(db, newNode, poi, nearest);

      //Perform Upward Pruning???????????????????????????????????????????????
      last = pruneBranchList(db, node, poi, nearest, branchList);
    }

  }
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
  struct Rect nearest;

  poi.x = atoi(x1);
  poi.y = atoi(y1);
  poi = *(struct Point *)malloc(sizeof(struct Point));
  node = *(struct Node *)malloc(sizeof(struct Node));
  nearest = *(struct Rect *)malloc(sizeof(struct Rect));/// zhe shi gan ma???????????????????????

  nearestNeighborSearch(db, node, poi, nearest);

  sqlite3_close(db);
}
