CREATE VIRTUAL TABLE rtree_index USING rtree(
   id,              -- Integer primary key
   start_lat, end_lat,      -- Minimum and maximum X coordinate
   start_lon, end_lon       -- Minimum and maximum Y coordinate
);

-- 10m of latitude in cartesian coordination system: 10/(111191*(48.2490-48.0600)/1000) = 0.4758483412331294
-- 10m of longitude in cartesian coordination system: 10/(74539*(11.7240-11.3580)/1000) = 0.3665517966646282
INSERT INTO rtree_index(id, start_lat, end_lat, start_lon, end_lon)
SELECT id, lat-0.4758483412331294, lat, lon, lon+0.3665517966646282 FROM poi_cartesian;