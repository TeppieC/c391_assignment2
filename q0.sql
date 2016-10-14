CREATE TABLE poi
(
	id INTEGER,
	uid INTEGER,
	lat NUMERIC(9,2),
	lon NUMERIC(9,2),
	PRIMARY KEY (id)
);

CREATE TABLE poi_tag(
	id INTEGER,
	key TEXT,
	value TEXT,
	PRIMARY KEY (id, key, value),
	FOREIGN KEY (id) REFERENCES po(id)
);
