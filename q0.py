'''
Author: Zhaorui Chen (Oct 6th 2016)
'''


#!/usr/bin/python

import sqlite3

LATSCALE = 111191
LONSCALE = 74539

# one unit in cartesian coordinates in meters: unitLatmeter * unitLonMeter
unitLatMeter = LATSCALE*(48.24900-48.06000)/1000
unitLonMeter = LONSCALE*(11.72400-11.35800)/1000

def convertLatToCartesian(lat):
	return str(LATSCALE*(float(lat)-48.06000)/unitLatMeter)

def convertLonToCartesian(lon):
	return str(LONSCALE*(float(lon)-11.35800)/unitLonMeter)

if __name__ == "__main__":
	conn = sqlite3.connect('test.db')
	print "Opened database successfully"

	conn.execute('''CREATE TABLE poi_cartesian
		   (id INT PRIMARY KEY,
		   uid INT,
		   lat NUMERIC(9,2),
		   lon NUMERIC(9,2));''')
	print "Table created successfully"

	container = []
	cursor = conn.execute("SELECT id, uid, lat, lon  from poi")
	for row in cursor:
		container.append((row[0],row[1],convertLatToCartesian(row[2]), convertLonToCartesian(row[3])))
	print "data extracted successfully",len(container)


	for row in container:
		conn.execute("INSERT INTO poi_cartesian (id,uid,lat,lon) VALUES (%s, %s, %s, %s )"%(row[0], row[1], row[2], row[3]));

	conn.commit()

	conn.close()
	# test only
	#print convertToCartesian(48.24,11.359)