DROP SCHEMA IF EXISTS package;
CREATE SCHEMA package;
REVOKE ALL ON SCHEMA package FROM PUBLIC;
GRANT  ALL ON SCHEMA package TO xtrole;
