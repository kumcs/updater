DROP TABLE IF EXISTS telephonelookup.pkgtestitem;
CREATE TABLE pkgtestitem (a INTEGER REFERENCES pkgtest(a), b INTEGER);

REVOKE ALL ON TABLE pkgtestitem FROM PUBLIC;
GRANT  ALL ON TABLE pkgtestitem TO   xtrole;
