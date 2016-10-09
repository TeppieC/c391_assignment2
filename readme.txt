Group collaborations guide:

1. 保持相同缩进 --> 1 tab with width 4
2. 测试用数据库命名为test.dbsw

References
1. http://stackoverflow.com/questions/26065872/how-to-import-a-tsv-file-with-sqlite3

Build guide:
1. To do the projection from lat/lon to cartesian coordinates, I wrote a python prgram to create a new table. The table "poi_cartesian" contains four columns "id, uid, lat, lon". The "id" and "uid" is the same format as the "poi" table. And the "lat" and "lon" are the projected cartesian coordinates ranged from 0-1000.
Please use poi_cartesian table for future questions.

2. The rtree table is a virtual table. Once created, 3 additional tables were created alongside. 

3. In the cartesian coordination system we created, the lat&lon attribute of every point labels the position of the top-left corner of the point. Therefore, the bounded box of the point(10m*10m) is in the coordinates of (lat-0.4758483412331294, lat, lon, lon+0.3665517966646282)