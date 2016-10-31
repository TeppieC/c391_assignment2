#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <math.h> // since we include math.h to implement sqrt(), please make sure to include -lm when compile

/***
  gcc -g q7.c sqlite3.c -lpthread -ldl -lm -DSQLITE_ENABLE_RTREE=1 
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
    // if the point within range of Y dimension
    if(p.y < node.maxY && p.y > node.minY) {
      if(p.x > node.maxX) {
        mindist = square(p.x - node.maxX);
      }
      else if(p.x < node.minX) {
        mindist = square(node.minX - p.x);
      }
    } 

    // else if the point within range of X dimension
    else if(p.x > node.minX && p.x < node.maxX) {
      if(p.y > node.maxY) {
        mindist = square(p.y - node.maxY);
      }
      else if(p.y < node.minY) {
        mindist = square(node.minY - p.y);
      }
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

/* calculate the actual distacne from the point to the given object
  here we use the distance between the point and the left-top of the object as object distance */
double objectDist(struct Point poi, double minX, double maxY){
  double dist=0;
  dist = sqrt(square(poi.x-minX) + square(poi.y-maxY));
  return dist;
}

/* extract nodes information */
int extractNodes(char *nodeString, struct Node *branchList){
  const char s[2] = " ";
  char *token;
  // use two dimension array "result" to store node information, including index and coordinates
  char** result = (char**) malloc(MAXIMAL*sizeof(char*));

  int i = 0;
  // break nodeString into a series of tuples using the delimiter " "(space)
  token = strtok(nodeString, s);
  while(token != NULL){

    // if it is the beginning of a tuple
    if (token[0] == '{'){
      // extract the node information from the tuple
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
  // return the number of nodes
  return n;
}

// generate active branch list
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
    // calculate minimal distance from the point to every node in the branch list 
    branchList[i].mindist = minDist(branchList[i], p);
  }
  // return the number of current branch list
  return length;
}

/* generate the children objrct which is the object pointed by leaf nodes */
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
    // convert the inittial part(children id) of the result into long int
    children[i] = strtol(result, &ptr, 10); 
    i+=1;
  }
  // return the number of childrem
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
    int col;
    for(col=0; col<sqlite3_column_count(stmt); col++) {
      rect[col] = atof((char *)sqlite3_column_text(stmt, col));
    }
  }
}


// self-defined compare function use for qsort
int cmpfunc (const void * a, const void * b){
  const struct Node * aa = a;
  const struct Node * bb = b;
  //sort based on the minimal distance
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

// implement downward pruning
int DownwardPruning (struct Node node, struct Point poi, struct Rect nearest, struct Node* branchList, int length) {
  /* prune the branchlist by the first rule
  return: the number of available branches left */

  int i, last, j;
  last = length;
  double min_minmaxdist;
  // initialize minimal minmax distance
  min_minmaxdist = branchList[0].minmaxdist;

  /* if the minimal distance of a mbr m less than the minmax distance of another mbr m',
    keep m in the branch list otherwise m should be discarded (strategy 2) */
  for(i=1; i<length; i++) {
    if(branchList[i].mindist <= min_minmaxdist) {
      if(branchList[i].minmaxdist < min_minmaxdist) {
        min_minmaxdist = branchList[i].minmaxdist;
      }
    }
    else {
      last = i;
      break;
    }
  }

  /* since the branch list is sorted by minimal distance, we use last to keep track of the satisfied
    mbr, once the mbr should be discared, we don't have to consider the mbrs behind it. */
  return last;
}

//implement upward puring
int UpwardPruning (struct Node node, struct Point poi, struct Rect nearest, struct Node* branchList, int length) {
  /* prune the branchlist by the third rule
    return: the number of available branches left */
  int i, last, j;
  last = length;

  /* if the minimal distance between the point and the mbr is greater than the actual distance 
    from the point to the given objetc, then this mbr is discarded. (strategy 3) */
  for(i=0; i<length; i++) {
    if(branchList[i].mindist > nearest.dist) {
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

// recursive produce
void nearestNeighborSearch(sqlite3 *db, struct Node node, struct Point poi, struct Rect* nearest){
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
    if((*nearest).id==0){
      (*nearest).id = children[0];
      getRect(db, children[0], rect);
      (*nearest).dist = objectDist(poi, rect[0], rect[3]);
    }
    /* Iterative through all children: swap if there is a closer children to the point */
    for (i = 0; i < numLeaves; ++i)
    {
      getRect(db, children[i], rect);
      dist = objectDist(poi, rect[0], rect[3]);
      if (dist<(*nearest).dist)
      {
        (*nearest).id = children[i]; 
        (*nearest).minX = rect[0];
        (*nearest).maxX = rect[1];
        (*nearest).minY = rect[2];
        (*nearest).maxY = rect[3];
        (*nearest).dist = dist;
      }
    }
    
  }else{
    //printf("The non-leaf node is: %d\n", node.node_index);
    int length = genBranchList(db, poi, node, branchList);
    //printf("generated ABL, length: %d\n", length);

    sortBranchList(branchList, length);
    //printf("sorted ABL based on mindist\n");

    //Perform Downward Pruning 
    last = DownwardPruning(node, poi, *nearest, branchList, length); //this will require dynamically change the branchlist how??????????????
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
      //printf("new node is: %d\n", newNode.node_index);

      //Recursively visit chile 
      //printf("entering a new recursion\n");
      nearestNeighborSearch(db, newNode, poi, nearest);
      //printf("finished one recursion\n");

      last = UpwardPruning(node, poi, *nearest, branchList, length);
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
  if( argc!=4 ){
    fprintf(stderr, "Usage: %s <database file> <x1> <y1>\n", argv[0]);
  	return(1);
	}
  if(atoi(argv[2]) > 1000 || atoi(argv[3]) > 1000) {
    fprintf(stderr, "the coordinates of the point should within 1000.\n");
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
  printf("Querying for the point (%s, %s)\n", x,y);


  /******** C program for NN searching algorithm ******************/
  struct Point poi;
  struct Node node;
  struct Rect nearest;

  poi.x = atof(x);
  poi.y = atof(y);
  nearest.id = 0;

  struct Node testNode1;
  testNode1.node_index = 1;
  testNode1.minX = 2.74727;
  testNode1.maxX = 997.596;
  testNode1.minY = 4.84267;
  testNode1.maxY = 994.657;
  testNode1.mindist = minDist(testNode1, poi);
  testNode1.minmaxdist = minMaxDist(testNode1, poi);

  nearestNeighborSearch(db, testNode1, poi, &nearest);
  printf("found nearest: id:%ld minX:%f maxX:%f minY:%f maxY:%f dist:%f\n", nearest.id, nearest.minX, nearest.maxX, nearest.minY, nearest.maxY, nearest.dist);

  sqlite3_close(db);
}
