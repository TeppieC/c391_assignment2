CREATE VIRTUAL TABLE rtree_index USING rtree(
   id,              -- Integer primary key
   start_y, end_y,      -- Minimum and maximum y coordinate
   start_x, end_x       -- Minimum and maximum x coordinate
);

-- 10m of latitude in cartesian coordination system: 10/(111191*(48.2490-48.0600)/1000) = 0.4758483412331294
-- 10m of longitude in cartesian coordination system: 10/(74539*(11.7240-11.3580)/1000) = 0.3665517966646282
-- y==lat, x==lon
INSERT INTO rtree_index(id, start_y, end_y, start_x, end_x)
SELECT id, end_y-0.4758483412331294, end_y, start_x, start_x+0.3665517966646282 FROM poi_cartesian;