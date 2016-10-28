#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

//http://stackoverflow.com/questions/3437404/min-and-max-in-c
//#define MIN(a,b) ((a) > (b) ? a : b)
//#define MAX(a,b) ((a) < (b) ? a : b) 
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct Node {
  int node_index;
  int count; // do we actually need this?
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
    // if the point is within range 
    if(p.y < node.maxY && p.y > node.minY) {
      if(p.x > node.maxX) {
        mindist = square(p.x - node.maxX);
      }
      else if(p.x < node.minX) {
        mindist = square(node.minX - p.x);
      }
    } 
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


double objectDist(struct Point poi, double minX, double maxX, double minY, double maxY){
  /* TODO: compute the distance between point poi and the rect*/
  double dist=0;

  return dist;
}

int extractNodes(char *nodeString, struct Node *branchList){
  const char s[2] = " ";
  char *token;
  char** result = (char**) malloc(300*sizeof(char*));

  int i = 0;
  //printf("heheh\n");
  token = strtok(nodeString, s);
  //printf("ha\n");
  while(token != NULL){

    if (token[0] == '{'){
      memmove(token, token+1, strlen(token));
    }
    //printf("%s\n", token);

    result[i] = token;
    token = strtok(NULL, s);
    i+=1;
  }
  //printf("hey\n");

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

int genBranchList(sqlite3 *db, struct Point p, struct Node node, struct Node* branchList){
  int rc;
  sqlite3_stmt *stmt;

  char *sql_stmt = "SELECT rtreenode(2,data) FROM rtree_index_node WHERE nodeno=?";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  sqlite3_bind_int(stmt, 1, node.node_index);

  int length = 0;
  //print_result(stmt);
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    printf("%s", sqlite3_column_text(stmt, 0));
    char *result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    length = extractNodes(result, branchList);// extract the result into the branchList
    printf("\n");
  }

  int i=0;
  //int length = lengthOfList(branchList);
  for(i=0;i<length;i++){    // parameter: branchList[i] is a Node struct, p is a Point struct
    branchList[i].mindist = minDist(branchList[i], p);
  }

  return length;
}

int genChildrenObject(sqlite3 *db, struct Node node, long* children){
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
    printf("%s\n", sqlite3_column_text(stmt, 0));
    char* result = (char *)sqlite3_column_text(stmt, 0); // parse from the query result
    //printf("%ld\n", strtol(result, &ptr, 10));
    char *ptr;
    children[i] = strtol(result, &ptr, 10); 
    //printf("%s\n", result);
    //printf("%ld\n", children[i]);
    i+=1;
  }
  return i;
}

void getRect(sqlite3 *db, int rectId, double* rect){
  /* Query for the coordinates given the id of a rectangle */

  int rc;
  sqlite3_stmt *stmt;

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

  /*MBRs with the lowest mindist should be left(Could have more than 1 left)*/

  /**/

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

  int numLeaves;
  numLeaves = leafCount(db, node);
  if (numLeaves>0)
  {
    long children[numLeaves]; //int children[lrafCount] -> double children[leafCount] --> Fixed
    int numChildren = genChildrenObject(db, node, children);
    
    /* Init: start with the first children */
    struct Rect nearest;
    nearest.id = children[0];//init
    double rect[4];  //assign 4 elements to rect???? --> Fixed
    getRect(db, children[0], rect); // fill the rect array
    nearest.dist = objectDist(poi, rect[0], rect[1], rect[2], rect[3]);
    
    /* Iterative through all children: swap if there is a closer children to the point */
    for (i = 1; i < numLeaves; ++i)
    {
      getRect(db, children[i], rect);
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
    // TODO: print out the result
    printf("id:%d minX:%f maxX:%f minY:%f maxY:%f dist:%f\n", nearest.id, nearest.minX, nearest.maxX, nearest.minY, nearest.maxY, nearest.dist);

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
  int i;


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
  char *x = argv[2];
  char *y = argv[3];
  printf("Querying for the point (%s, %s)\n", x,y);


  /******** C program for NN searching algorithm ******************/
  struct Point poi;
  struct Node node;
  struct Rect nearest;

  //printf("%f\n", atof(x));
  poi.x = atof(x);
  poi.y = atof(y);
  poi = *(struct Point *)malloc(sizeof(struct Point));
  node = *(struct Node *)malloc(sizeof(struct Node));
  nearest = *(struct Rect *)malloc(sizeof(struct Rect));/// zhe shi gan ma???????????????????????

  /* leaf count: OK
  struct Node testNode;
  testNode.node_index = 1683;
  int count = leafCount(db, testNode);
  printf("%d\n", count);
  */

  /* min dist: not OK, may need to calibrate */
  struct Point testPoi;
  testPoi.x = 9.898; // 0 won't work --> case5&6 have problem
  testPoi.y = 446;
  printf("test poi is: %f, %f\n", testPoi.x, testPoi.y);

  printf("test for test1101\n");
  struct Node testNode1101;
  testNode1101.node_index = 1101;
  testNode1101.minX = 2.74727;
  testNode1101.maxX = 575.684;
  testNode1101.minY = 5.34161;
  testNode1101.maxY = 994.443;
  testNode1101.mindist = minDist(testNode1101, testPoi);
  //printf("%f\n", testNode1101.mindist);

  printf("test for test1472\n");
  struct Node testNode1472;
  testNode1472.node_index = 1472;
  testNode1472.minX = 2.74727;
  testNode1472.maxX = 474.458;
  testNode1472.minY = 681.342;
  testNode1472.maxY = 994.443;
  testNode1472.mindist = minDist(testNode1472, testPoi);

  printf("test for test890\n");
  struct Node testNode890;
  testNode890.node_index = 890;
  testNode890.minX = 602.46;
  testNode890.maxX = 654.178;
  testNode890.minY = 484.865;
  testNode890.maxY = 629.767;
  testNode890.mindist = minDist(testNode890, testPoi);
  //printf("%f\n", testNode1472.mindist);*/

  printf("test for test53, a leaf node\n");
  //53 620.96 627.419 411.67 502.449 A leaf node
  struct Node testNode53;
  testNode53.node_index = 53;
  testNode53.minX = 602.96;
  testNode53.maxX = 627.419;
  testNode53.minY = 411.67;
  testNode53.maxY = 502.449;
  testNode53.mindist = minDist(testNode53, testPoi);

/* extractNodes: OK*/
  //char nodeString[] = "{1101 2.74727 575.684 5.34161 994.443} {1100 526.545 997.596 4.84267 994.657}";//// cannot use char*???
  //char nodeString[] = "{146 2.75355 90.1406 359.665 480.358} {652 3.01694 437.572 524.205 709.73} {43 2.97623 312.291 5.576 386.872} {1099 90.3024 193.208 398.932 510.586} {219 95.5809 430.375 468.996 579.271} {802 116.597 149.479 374.843 400.528} {1085 149.113 260.103 367.148 509.154} {1057 250.55 320.57 369.577 509.633} {892 302.808 425.133 8.78182 234.717} {519 316.861 411.414 374.148 509.966} {274 385.343 442.121 7.02256 276.013} {812 402.164 526.629 227.145 265.49} {590 396.143 553.229 260.617 310.096} {783 405.72 487.73 372.001 510.997} {300 408.413 487.749 301.7 341.671} {959 434.862 560.883 5.34161 149.199} {633 436.161 575.684 608.962 985.94} {758 438.37 528.135 560.579 745.281} {382 430.2 525.865 502.837 561.902} {430 470.624 570.692 361.295 429.932} {406 483.626 526.757 302.32 391.654} {1125 148.166 154.519 398.732 432.215} {1180 408.84 484.389 331.887 391.472} {1245 284.246 409.954 235.74 373.258} {1357 440.719 559.541 148.646 233.403} {1472 2.74727 474.458 681.342 994.443} {1564 469.916 569.906 432.828 510.025}";
  //int length = extractNodes(nodeString, testBranchList);
/* genBranchList: OK*/ /*
  struct Node testBranchList[200];
  int length = genBranchList(db, testPoi, testNode1101, testBranchList);
  printf("length of the testBranchList: %d\n", length);
  sortBranchList(genBranchList);
  for (i = 0; i < length; ++i)
  {
    printf("%d\n", testBranchList[i].node_index);
    printf("%f\n", testBranchList[i].minX);
    printf("%f\n", testBranchList[i].maxX);
    printf("%f\n", testBranchList[i].minY);
    printf("%f\n", testBranchList[i].maxY);
  }*/

/* genChildrenObject: OK */
  long children[50]; // could we use other forms? just wondering
  int numChildrenObjects = genChildrenObject(db, testNode53, children); //max:39
  printf("%d\n", numChildrenObjects);
  for (i = 0; i < numChildrenObjects; ++i)
  {
    /* code */
    printf("%ld\n", children[i]);
  }

  //nearestNeighborSearch(db, node, poi, nearest);

  sqlite3_close(db);
}
