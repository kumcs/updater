DROP TABLE IF EXISTS telephonelookup.pkgtest CASCADE;
CREATE TABLE pkgtest (a INTEGER PRIMARY KEY, b INTEGER);

REVOKE ALL ON TABLE pkgtest FROM PUBLIC;
GRANT  ALL ON TABLE pkgtest TO   xtrole;
