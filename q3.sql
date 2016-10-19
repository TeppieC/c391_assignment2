CREATE VIRTUAL TABLE rtree_index USING rtree(
   id,              -- Integer primary key
   minX, maxX,      -- Minimum and maximum X coordinate
   minY, maxY       -- Minimum and maximum Y coordinate
);

INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 1, 2, 5, 23, 25;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 2, 3, 7, 17, 20;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 3, 1, 4, 11, 13;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 4, 1, 4, 0, 3;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 5, 6, 9, 21, 24;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 6, 7, 9, 15, 20;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 7, 6, 13, 3, 8;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 8, 17, 10, 9, 22;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 9, 19, 24, 9, 12;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 10, 19, 23, 6, 8;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 11, 21, 26, 21, 25;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 12, 20,30,15,17;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 13, 25,28,12,16;
INSERT INTO rtree_index(id, minX, maxX, minY, maxY)
SELECT 14, 13,17,19,22;
