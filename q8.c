#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <math.h> // since we include math.h to implement sqrt(), please make sure to include -lm when compile

/***
  gcc -g q8.c sqlite3.c -lpthread -ldl -lm -DSQLITE_ENABLE_RTREE=1 
***/

//http://stackoverflow.com/questions/3437404/min-and-max-in-c
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MAXIMAL 512

struct Node {
  int node_index;
  double mindist;
  double minmaxdist;
  double minX;
  double minY;
  double maxX;
  double maxY;
};

struct Rect {
  long id;
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

// self-defined square function
double square (double a) {
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
    // if the point is within range of Y dimension
    if(p.y < node.maxY && p.y > node.minY) {
      if(p.x > node.maxX) {
        mindist = square(p.x - node.maxX);
      }
      else if(p.x < node.minX) {
        mindist = square(node.minX - p.x);
      }
    } 

    // if the point is within range of Y dimension
    else if(p.x > node.minX && p.x < node.maxX) {
      if(p.y > node.maxY) {
        mindist = square(p.y - node.maxY);
      }
      else if(p.y < node.minY) {
        mindist = square(node.minY - p.y);
      }
      //mindist = MIN((p.y-node.maxY)*(p.y-node.maxY), (p.y-node.minY)*(p.y-node.minY));
    }

    else {
      if(p.x > node.maxX) {
        mindist  = MIN(square(p.y-node.maxY)+square(p.x-node.maxX), square(p.y-node.minY)+square(p.x-node.maxX));
      }
      else if(p.x < node.minX) {
        mindist = MIN(square(p.y-node.maxY)+square(p.x-node.minX), square(p.y-node.minY)+square(p.x-node.minX));
      }
    }
  }
  return mindist;
}

//calculate the minmax distance from a point to mbr
double minMaxDist(struct Node node, struct Point p) {
  double minMaxDist;
  if(p.x > node.maxX) {
    if(p.y > node.minY && p.y < node.maxY) {
      minMaxDist = MAX(square(p.y-node.maxY)+square(p.x-node.maxX), square(p.y-node.minY)+square(p.x-node.maxX));
    }
    else {
      if(p.y > node.maxY) {
        minMaxDist = MIN(square(p.y-node.minY)+square(p.x-node.maxX), square(p.y-node.maxY)+square(p.x-node.minX));
      }
      else if(p.y < node.minY) {
        minMaxDist = MIN(square(p.y-node.maxY)+square(p.x-node.maxX), square(p.y-node.minY)+square(p.x-node.minX));
      }
    }
  }

  else if(p.x < node.minX) {
    if(p.y > node.minY && p.y < node.maxY) {
      minMaxDist = MAX(square(p.y-node.maxY)+square(p.x-node.minX), square(p.y-node.minY)+square(p.x-node.minX));
    }
    else {
      if(p.y > node.maxY) {
        minMaxDist = MIN(square(p.y-node.minY)+square(p.x-node.minX), square(p.y-node.maxY)+square(p.x-node.maxX));
      }
      else if(p.y < node.minY) {
        minMaxDist = MIN(square(p.y-node.maxY)+square(p.x-node.minX), square(p.y-node.minY)+square(p.x-node.maxX));
      }
    }
  }

  else {
    if(p.y > node.maxY) {
      minMaxDist = MAX(square(p.y-node.maxY)+square(p.x-node.minX), square(p.y-node.maxY)+square(p.x-node.maxX));
    }
    else {
      minMaxDist = MAX(square(p.y-node.minY)+square(p.x-node.maxX), square(p.y-node.minY)+square(p.x-node.minX));
    }
  }
  return minMaxDist;
}

double objectDist(struct Point poi, double minX, double maxY){
  /* calculate the actual distacne from the point to the given object
    here we use the distance between the point and the left-top of the object as object distance */
  double dist=0;
  dist = sqrt(square(poi.x-minX) + square(poi.y-maxY));
  return dist;
}

/* extract nodes information (same as q7) */
int extractNodes(char *nodeString, struct Node *branchList){
  const char s[2] = " ";
  char *token;
  char** result = (char**) malloc(300*sizeof(char*));

  int i = 0;
  token = strtok(nodeString, s);
  while(token != NULL){

    if (token[0] == '{'){
      memmove(token, token+1, strlen(token));
    }

    result[i] = token;
    token = strtok(NULL, s);
    i+=1;
  }

  int j=0;
  int n=0;
  for(j=0;j<i;j+=5){
    branchList[n].node_index = atoi(result[j]);
    branchList[n].minX = atof(result[j+1]);
    branchList[n].maxX = atof(result[j+2]);
    branchList[n].minY = atof(result[j+3]);
    /* deal with the last string, to remove the last '}' character */
    char *end = result[j+4];
    end[strlen(end)-1]=0;
    branchList[n].maxY = atof(end);
    n+=1;
  }
  free(result);
  return n;
}

/* generate branch list (same as q7) */
int genBranchList(sqlite3 *db, struct Point p, struct Node node, struct Node* branchList){
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT rtreenode(2,data) FROM rtree_index_node WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  int length = 0;
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    length = extractNodes(result, branchList);// extract the result into the branchList
  }

  int i=0;
  for(i=0;i<length;i++){    // parameter: branchList[i] is a Node struct, p is a Point struct
    branchList[i].mindist = minDist(branchList[i], p);
    branchList[i].minmaxdist = minMaxDist(branchList[i], p);
  }

  return length;
}

int genChildrenObject(sqlite3 *db, struct Node node, long* children){
  /* Return the id's of the children of a leaf node */
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT rowid FROM rtree_index_rowid WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  int i = 0; 
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    char* result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    char *ptr;
    children[i] = strtol(result, &ptr, 10); 
    i+=1; 
  }
  return i;
}

void getRect(sqlite3 *db, long rectId, double* rect){
  /* Query for the coordinates given the id of a rectangle */

  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT start_X, end_X, start_Y, end_Y FROM rtree_index WHERE id=?";
  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  sqlite3_bind_int64(stmt, 1, rectId);
  
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    //printf("%s", sqlite3_column_text(stmt, 0));
    int col;
    for(col=0; col<sqlite3_column_count(stmt); col++) {
      rect[col] = atof((char *)sqlite3_column_text(stmt, col));
    }
  }
}

/* self-defined compare function use for qsort base on minimal distance from the point to mbr */
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

void sortBranchList(struct Node* branchList, int length){
  /* sorting based on mindist */
  qsort(branchList, length, sizeof(struct Node), cmpfunc); 
}

/* self-defined compare function use for qsort based on the object distance from 
  the point to the given object */
int cmpfunc2 (const void * a, const void * b){
  const struct Rect * aa = a;
  const struct Rect * bb = b;

  if (aa->dist > bb->dist){
    return 1;
  }else if (aa->dist == bb->dist){
    return 0;
  }else{
    return -1;
  }
}

void sortNearest(struct Rect* nearests, int k){
  /* sorting based on mindist */
  qsort(nearests, k, sizeof(struct Rect), cmpfunc2); 
}

/* self-defined compare function use for qsort base on minmax distance from the point to mbr */
int cmpfunc3 (const void * a, const void * b){
  const struct Node * aa = a;
  const struct Node * bb = b;

  if (aa->minmaxdist > bb->minmaxdist){
    return 1;
  }else if (aa->minmaxdist == bb->minmaxdist){
    return 0;
  }else{
    return -1;
  }
}

void sortBranchListMinMax(struct Node* branchlist, int length){
  /* sorting based on mindist */
  qsort(branchlist, length, sizeof(struct Node), cmpfunc3); 
}

// implement downward puring
int DownwardPruning (struct Node node, struct Point poi, struct Rect* nearests, int k, struct Node* branchList, int length) {
  int i, last, j;
  last = length;

  /* To make sure there is enough branches left
      We need to ...*/
  sortBranchListMinMax(branchList, length);
  double min_minmaxdist;
  if(length>k){
    // if the node has more than k branches
    min_minmaxdist = branchList[k-1].minmaxdist;
  }else{
    // if the node has less than k branches
    min_minmaxdist = branchList[length-1].minmaxdist;
  }
  // everytime need to sort the branch list again
  sortBranchList(branchList, length);

  for(i=0; i<length; i++) {
    if(branchList[i].mindist > min_minmaxdist){
      last = i;
      break;
    }
  }
  return last;
}

int DownwardPruningSecond(struct Node node, int leafCount, struct Point poi, struct Rect* nearest, struct Node* branchlist, int length){
  /* prune the branchlist by the second rule
  return: the number of available branches left */  
  int i;
  int last = length;
  if (leafCount>0){// if the node is a leaf node, do the second prunning
    for (i = 0; i < length; ++i){
      if (objectDist(poi, branchlist[i].minX, branchlist[i].maxY)>minMaxDist(node, poi)){
        last = i;
        break;
      }
    }
  }
  return last;
}

//implemnent upward puring
int UpwardPruning (struct Node node, struct Point poi, struct Rect* nearests, int k, struct Node* branchList, int length) {
  /* prune the branchlist by the third rule
    return: the number of available branches left */

  int i, last, j;
  last = length;
  for(i=0; i<length; i++) {
    // prunning based on the distance furtherest object in nearest
    if(branchList[i].mindist > nearests[k-1].dist) {
      last = i;
      break;
    }
  }
  return last;
}


int leafCount(sqlite3 *db, struct Node node){
  /* Return the # of children of a leaf node. 
    If it is not a leaf node, return 0 instead. */

  int count;
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT count(rowid) FROM rtree_index_rowid WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    count = atoi(result); 
    //printf("\n");
  }

  return count;
}

/* use branch list to store current nearest k neighbors */
void fillTheNearests(struct Rect* nearests, int k, struct Point poi, long child_id, double minX, double maxX, double minY, double maxY){
  double dist = objectDist(poi, minX, maxY);
  int i;
  if (nearests[k-1].id==0)
  {
    // change the first object to the current child, then resort
    nearests[k-1].id = child_id;
    nearests[k-1].minX = minX;
    nearests[k-1].maxX = maxX;
    nearests[k-1].minY = minY;
    nearests[k-1].maxY = maxY;
    nearests[k-1].dist = dist;
  }else{
    if (nearests[k-1].dist > dist){
        //change the last furtherest object to the current child, then resort
        nearests[k-1].id = child_id;
        nearests[k-1].minX = minX;
        nearests[k-1].maxX = maxX;
        nearests[k-1].minY = minY;
        nearests[k-1].maxY = maxY;
        nearests[k-1].dist = dist;
    }
  }
  // resort the nearest array
  sortNearest(nearests, k);
}

// recursive produce
void nearestNeighborSearch(sqlite3 *db, struct Node node, struct Point poi, struct Rect* nearests, int k){//should parameter be pointers????????????????????
  struct Node newNode;
  struct Node branchList[MAXIMAL];
  double dist;
  int last;
  int i,j;

  int numLeaves;
  numLeaves = leafCount(db, node);
  // numLeaves > 0 means at leaf level - compute distance to actual object
  if (numLeaves>0)
  {
    //printf("now found a leaf node: %d\n", node.node_index);
    //printf("num of leaves/objects: %d\n", numLeaves);

    long children[numLeaves];
    int numChildren = genChildrenObject(db, node, children);
    double rect[4];
    /* Iterative through all children: swap if there is a closer children to the point */
    for (i = 0; i < numLeaves; ++i)
    {
      getRect(db, children[i], rect); //get the coordinates of this child
      fillTheNearests(nearests, k, poi, children[i], rect[0], rect[1], rect[2], rect[3]);
    }
    
  }else{
    //printf("The non-leaf node is: %d\n", node.node_index);

    int length = genBranchList(db, poi, node, branchList);

    sortBranchList(branchList, length);
 
    last = DownwardPruning(node, poi, nearests, k, branchList, length); //this will require dynamically change the branchlist how??????????????
    //printf("down pruned, now has %d possible branches\n", last);

    for (j = 0; j < last; ++j)
    {
      newNode.node_index = branchList[j].node_index;
      newNode.minX = branchList[j].minX;
      newNode.maxX = branchList[j].maxX;
      newNode.minY = branchList[j].minY;
      newNode.maxY = branchList[j].maxY;
      newNode.mindist = branchList[j].mindist;
      newNode.minmaxdist = branchList[j].minmaxdist;

      //Recursively visit children
      nearestNeighborSearch(db, newNode, poi, nearests, k);

      //Perform Upward Pruning
      last = UpwardPruning(node, poi, nearests, k, branchList, length);
      //printf("up pruned, now has %d possible branches\n", last);
    }
  }
}

int main(int argc, char **argv){
  sqlite3 *db;
  sqlite3_stmt *stmt;
  char *zErrMsg = 0;
  int rc;  
  int i;

  // check the arguments
  if( argc!=5 ){
    fprintf(stderr, "Usage: %s <database file> <x1> <y1> <k value>\n", argv[0]);
  	return(1);
	}
  if(atoi(argv[2]) > 1000 || atoi(argv[3]) > 1000) {
    fprintf(stderr, "The coordinates of the point should within 1000.\n");
    return(1);
  }
  if(atoi(argv[4]) > 41990) {
    fprintf(stderr, "Beyond the number of nodes, at most 41990.\n");
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
  char *x = argv[2];
  char *y = argv[3];
  char *k_str = argv[4];
  int k = atoi(k_str);
  printf("Querying for the point (%s, %s), with the number of k: %d\n", x,y,k);

  /******** C program for NN searching algorithm ******************/
  struct Point poi;
  struct Node node;
  struct Rect nearests[k];

  poi.x = atof(x);
  poi.y = atof(y);

  /* initialize the nearests array*/
  for (i = 0; i < k; ++i)
  {
    nearests[i].id = 0;
    nearests[i].minX = -1;
    nearests[i].maxX = -1;
    nearests[i].minY = -1;
    nearests[i].maxY = -1;
    nearests[i].dist = 1000*1000*1.44; // the maximum distance in this map
  }

  /* initialize the starting node with the root node */
  struct Node testNode1;
  testNode1.node_index = 1;
  testNode1.minX = 2.74727;
  testNode1.maxX = 997.596;
  testNode1.minY = 4.84267;
  testNode1.maxY = 994.657;
  testNode1.mindist = minDist(testNode1, poi);
  testNode1.minmaxdist = minMaxDist(testNode1, poi);

  nearestNeighborSearch(db, testNode1, poi, nearests, k);
  for (i = 0; i < k; ++i)
  {
    printf("found nearest: id:%ld minX:%f maxX:%f minY:%f maxY:%f dist:%f\n", nearests[i].id, nearests[i].minX, nearests[i].maxX, nearests[i].minY, nearests[i].maxY, nearests[i].dist);
  }

  sqlite3_close(db);
}
