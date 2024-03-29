DROP FUNCTION IF EXISTS  alsep_sp(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_sp(timestamp, timestamp, int);
DROP FUNCTION IF EXISTS  alsep_sp_query(timestamp, timestamp, int, int);
DROP TYPE IF EXISTS alsep_sp_type;
DROP FUNCTION IF EXISTS  alsep_lp(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lp(timestamp, timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lp_query(timestamp, timestamp, int, int);
DROP TYPE IF EXISTS alsep_lp_type;
DROP FUNCTION IF EXISTS  alsep_tidal_query(timestamp, timestamp, pattern int);
DROP FUNCTION IF EXISTS  alsep_tidalxy(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_tidalxy(timestamp, timestamp, int);
DROP FUNCTION IF EXISTS  alsep_tidalzt(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_tidalzt(timestamp, timestamp, int);
DROP TYPE IF EXISTS alsep_tidalxy_type;
DROP TYPE IF EXISTS alsep_tidalzt_type;
DROP FUNCTION IF EXISTS  alsep_lsg(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lsg(timestamp, timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lsg_query(timestamp, timestamp, int, int);
DROP TYPE IF EXISTS alsep_lsg_type;
DROP FUNCTION IF EXISTS  alsep_lspe(timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lspe(timestamp, timestamp, int);
DROP FUNCTION IF EXISTS  alsep_lspe_query(timestamp, timestamp, int, int);
DROP TYPE IF EXISTS alsep_lspe_type;
