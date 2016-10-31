References
1. http://stackoverflow.com/questions/26065872/how-to-import-a-tsv-file-with-sqlite3
2. two papers from eclass outlink

Build guide:
0. Note: To do the projection from lat/lon to cartesian coordinates, I wrote a python prgram to create a new table. The table "poi_cartesian" contains four columns "id, uid, lat, lon". The "id" and "uid" is the same format as the "poi" table. And the "lat" and "lon" are the projected cartesian coordinates ranged from 0-1000.
Please use the poi_cartesian table for future questions.

1. To start the program and populate the database: 
	a) sqlite3 test.db --> create a new database 
	b) .read q0.txt --> create tables and populate database with tsv file
	c) .quit --> quit sqlite
	d) python q0.py --> create a new table, with projected cartesian coordinates, the table is named "poi_cartesian"
		 Note: If you want to project using a different database, change the line26 in q0.py to the name of your own database.
	e) sqlite3 test.db
	f) .read q1.txt --> create rtree indexes

2. Be sure to enable rtree module when compiling.  
	For Q4:
	a) gcc -g -o q4 q4.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
	b) ./q4 test.db 0 0 1000 1000 hotel (example only)

	For Q5:
	a) gcc -g -o q5 q5.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
	b) ./q5 test.db 10 (example only)

3. For q7, q8, Compile with -lm flag:
	For Q7:
	a) gcc -g -o q7 q7.c sqlite3.c -lpthread -ldl -lm -DSQLITE_ENABLE_RTREE=1
	b) ./q7 test.db 1 1 (example only)

	For Q8:
	a) gcc -g -o q8 q8.c sqlite3.c -lpthread -ldl -lm -DSQLITE_ENABLE_RTREE=1
	b) ./q8 test.db 1 1 1 (example only)

Note:
0. The database we created for the question is called "test.db"

1. In the cartesian coordination system we created, the lat&lon attribute of every point labels the position of the top-left corner of the point. Therefore, the bounded box of the point(10m*10m) is in the coordinates of (lat-0.4758483412331294, lat, lon, lon+0.3665517966646282)

2. We used mindist matrix to sort the available branch list

3. Please note for q5, you have to first create indexes using sql statements. See q5 for details.
