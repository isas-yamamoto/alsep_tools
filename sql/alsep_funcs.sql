--
-- pse sp data retrieval functions
--
DROP FUNCTION IF EXISTS  alsep_sp(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_sp(time0 timestamp, time9 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_sp_query(time0 timestamp, time2 timestamp, dtframe int, ncarray int);
DROP TYPE IF EXISTS alsep_sp_type;

--
-- alsep_sp()
-- 	specify start time and number of points to retrieve pse short period data(sp_z) from tbl_pse table for every stations.
--

CREATE TYPE alsep_sp_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, frame_count	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, nc	smallint
	, sp_z	smallint
	, process_flag	smallint
	, error_flag	smallint
	, time_flag	smallint
);


CREATE OR REPLACE FUNCTION alsep_sp_query(
  tstamp0 timestamp	
  ,tstamp2 timestamp
  ,dtframe int
  ,ncarray int
) RETURNS text AS $$
DECLARE
  tstamp1 timestamp;	-- start before 1 frame timestamp
  tstamp3 timestamp;	-- end after 1 step timestamp
  dtstep float;	-- data time step in msec
  arr_str text;	-- dummy array definition string
  s_query text;	-- part of query strings
  s_claus text;	-- where clause of query strings
  e_query text;	-- part of query strings
  e_claus text;	-- order by clause for query strings
  d_claus text;	-- where clause of query strings
  stmt text;	-- query statement to be executed
BEGIN
  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- Set dummy array
  arr_str := 'generate_series(1, ' || ncarray || ')';

  -- one frame before start timestamp
  tstamp1 := tstamp0 - (dtframe::text || ' milliseconds')::interval;

  -- one frame after end timestamp
  tstamp3 := tstamp2 + (dtframe::text || ' milliseconds')::interval;

  -- set query string
  s_query := 'f.ap_station, f.ground_station,  f.file_id, f.pos, f.length, f.frame_count,  f.time_diff, f.time, '||arr_str||' nc, unnest(f.sp_z) sp_z,  f.process_flag, f.error_flag, f.time_flag FROM tbl_pse f';
  e_query := 'e.ap_station, e.ground_station,  e.file_id, e.pos, e.length, e.frame_count,  e.time_diff, (e.time + (' || dtstep::text || '* (e.nc-1)||'' milliseconds'')::interval)::timestamp without time zone as time, e.nc, e.sp_z,  e.process_flag, e.error_flag, e.time_flag ';
  --                                                                                      e.x.          (e.time + (      18.875          * (e.nc-1)|| ' milliseconds' )::interval)::timestamp 


  s_claus := '''' || tstamp1 || ''' <= f.time AND f.time <= ''' || tstamp3 || '''';
  e_claus := 'ORDER BY  e.ap_station, time';
  d_claus := '''' || tstamp0 || ''' <= d.time AND d.time < ''' || tstamp2 || '''';

  stmt := 'SELECT * FROM (SELECT ' || e_query || ' FROM (SELECT ' || s_query || ' WHERE ' || s_claus || ') AS e ' || e_claus || ') AS d WHERE ' || d_claus ;
  RAISE NOTICE 'stmt= %', stmt;

  RETURN stmt;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_sp('1972-12-02T01:30:55.999', 500)"
--   psql -c "SELECT * FROM alsep_sp('1972-12-02T01:30:55.999')"
--   psql -c "SELECT * FROM alsep_sp()"
--
CREATE OR REPLACE FUNCTION alsep_sp(
  tstamp0 timestamp DEFAULT '1972-12-02T01:30:55.999' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_sp_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_sp_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 32;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_sp_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_sp('1977-09-30 16:30:22.103', '1977-09-30 16:30:32.367', 500)"
--   psql -c "SELECT * FROM alsep_sp('1977-09-30 16:30:23.311', '1977-09-30 16:30:32.367', 500)"
--   psql -c "SELECT * FROM alsep_sp('1977-09-30 16:30:24.518', '1977-09-30 16:30:32.971', 500)"
--
CREATE OR REPLACE FUNCTION alsep_sp(
  tstamp0 timestamp -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_sp_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_sp_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 32;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_sp_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;
--
-- pse lp data retrieval functions
--
DROP FUNCTION IF EXISTS  alsep_lp(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lp(time0 timestamp, time9 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lp_query(time0 timestamp, time2 timestamp, dtframe int, ncarray int);
DROP TYPE IF EXISTS alsep_lp_type;

--
-- alsep_lp()
-- 	specify start time and number of points to retrieve pse long period data(lp_x, lp_y, lp_z) from tbl_pse table for every stations.
--

CREATE TYPE alsep_lp_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, frame_count	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, nc	smallint
	, lp_x  smallint
	, lp_y  smallint
	, lp_z  smallint
	, process_flag	smallint
	, error_flag	smallint
	, time_flag	smallint
);

CREATE OR REPLACE FUNCTION alsep_lp_query(
  tstamp0 timestamp	
  ,tstamp2 timestamp
  ,dtframe int
  ,ncarray int
) RETURNS text AS $$
DECLARE
  tstamp1 timestamp;	-- start before 1 frame timestamp
  tstamp3 timestamp;	-- end after 1 step timestamp
  dtstep float;	-- data time step in msec
  arr_str text;	-- dummy array definition string
  s_query text;	-- part of query strings
  s_claus text;	-- where clause of query strings
  e_query text;	-- part of query strings
  e_claus text;	-- order by clause for query strings
  d_claus text;	-- where clause of query strings
  stmt text;	-- query statement to be executed
BEGIN
  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- Set dummy array
  arr_str := 'generate_series(1, ' || ncarray || ')';

  -- one frame before start timestamp
  tstamp1 := tstamp0 - (dtframe::text || ' milliseconds')::interval;

  -- one frame after end timestamp
  tstamp3 := tstamp2 + (dtframe::text || ' milliseconds')::interval;

  -- set query string
  s_query := 'f.ap_station, f.ground_station,  f.file_id, f.pos, f.length, f.frame_count,  f.time_diff, f.time, '||arr_str||' nc, unnest(f.lp_x) lp_x, unnest(f.lp_y) lp_y, unnest(f.lp_z) lp_z,  f.process_flag, f.error_flag, f.time_flag FROM tbl_pse f';
  e_query := 'e.ap_station, e.ground_station,  e.file_id, e.pos, e.length, e.frame_count,  e.time_diff, (e.time + (' || dtstep::text || '* (e.nc-1)||'' milliseconds'')::interval)::timestamp without time zone as time, e.nc, e.lp_x, e.lp_y, e.lp_z,  e.process_flag, e.error_flag, e.time_flag ';

  s_claus := '''' || tstamp1 || ''' <= f.time AND f.time <= ''' || tstamp3 || '''';
  e_claus := 'ORDER BY  e.ap_station, time';
  d_claus := '''' || tstamp0 || ''' <= d.time AND d.time < ''' || tstamp2 || '''';

  stmt := 'SELECT * FROM (SELECT ' || e_query || ' FROM (SELECT ' || s_query || ' WHERE ' || s_claus || ') AS e ' || e_claus || ') AS d WHERE ' || d_claus ;
  RAISE NOTICE 'stmt= %', stmt;

  RETURN stmt;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_lp('1972-11-20T01:30:55.999', 500)"
--   psql -c "SELECT * FROM alsep_lp('1972-11-20T01:30:55.999')"
--   psql -c "SELECT * FROM alsep_lp()"
--
CREATE OR REPLACE FUNCTION alsep_lp(
  tstamp0 timestamp DEFAULT '1972-12-02T01:30:55.999' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_lp_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lp_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 4;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_lp_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql;


--
-- example:
--   psql -c "SELECT * FROM alsep_lp('1977-09-30 16:29:16.897', '1977-09-30 16:30:32.367', 500)"
--   psql -c "SELECT * FROM alsep_lp('1977-09-30 16:29:17.501', '1977-09-30 16:30:32.367', 500)"
--   psql -c "SELECT * FROM alsep_lp('1977-09-30 16:29:18.709', '1977-09-30 16:30:32.367', 500)"
--
CREATE OR REPLACE FUNCTION alsep_lp(
  tstamp0 timestamp -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_lp_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lp_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 4;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_lp_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;
--
-- pse tidal data retrieval functions
--

-- 	for alsep_tidal_query()
DROP FUNCTION IF EXISTS  alsep_tidal_query(time0 timestamp, time2 timestamp, pattern int);
-- 	for alsep_tidalxy(),  alsep_tidalzt()
DROP FUNCTION IF EXISTS  alsep_tidalxy(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_tidalxy(time0 timestamp, time9 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_tidalzt(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_tidalzt(time0 timestamp, time9 timestamp, npts int);
--	custom types for alsep_tidalxy, alsep_tidalzt
DROP TYPE IF EXISTS alsep_tidalxy_type;
DROP TYPE IF EXISTS alsep_tidalzt_type;

--
-- alsep_tidalxy(),  alsep_tidalzt()
-- 	specify start time and number of points to retrieve each 2 rows data((tidal_x, tidal_y) or (tidal_z, inst_temp)) from tbl_pse table for every stations.
--
CREATE TYPE alsep_tidalxy_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, frame_count	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, tidal_x smallint
	, tidal_y smallint
	, process_flag smallint
	, error_flag smallint
	, time_flag smallint
);
CREATE TYPE alsep_tidalzt_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, frame_count	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, tidal_z smallint
	, inst_temp smallint
	, process_flag smallint
	, error_flag smallint
	, time_flag smallint
);

--
-- example:
--
--	alsep_tidal_query('1972-11-20T01:30:55.000', '1972-11-20T01:35:00.000', 0);
--
CREATE OR REPLACE FUNCTION alsep_tidal_query(
  tstamp0 timestamp	-- start timestamp
  ,tstamp2 timestamp	-- end timestamp
  ,pattern int		-- frame pattern( modurous 2 of frameid choosing xy or zt)
) RETURNS text AS $$
DECLARE
  tidal_c1 text;	-- tidal data column name(1)
  tidal_c2 text;	-- tidal data column name(2)
  s_query text;	-- part of query strings
  s_claus text;	-- where clause of query strings
  condition text; -- condition for pattern
  stmt text;	-- query statement to be executed
BEGIN
  IF pattern =  0 THEN
    tidal_c1 := 'tidal_x';
    tidal_c2 := 'tidal_y';
    condition := 'frame_count%2 = 0';
  ELSE
    tidal_c1 := 'tidal_z';
    tidal_c2 := 'inst_temp';
    condition := 'frame_count%2 = 1';
  END IF;

  -- set query string
  s_query := 'ap_station, ground_station, file_id, pos, length, frame_count,  time_diff, time, ' || tidal_c1 || ', ' || tidal_c2 || ', process_flag, error_flag, time_flag FROM tbl_pse f';
  s_claus := '(''' || tstamp0 || ''' <= time AND time <= ''' || tstamp2 || ''')';
  s_claus := s_claus || ' AND ' || condition || ' ORDER BY  ap_station, time';

  stmt := 'SELECT ' || s_query || ' WHERE ' || s_claus;
  RAISE NOTICE 'stmt= %', stmt;

  RETURN stmt;
END;
$$ LANGUAGE plpgsql;


--
-- example:
--   psql -c "SELECT * FROM alsep_tidalxy('1972-11-20T01:30:55.999', 500)"
--   psql -c "SELECT * FROM alsep_tidalxy('1972-11-20T01:30:55.999')"
--   psql -c "SELECT * FROM alsep_tidalxy()"
--
CREATE OR REPLACE FUNCTION alsep_tidalxy(
  tstamp0 timestamp DEFAULT '1972-11-20T01:30:55.999' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_tidalxy_type AS $$
DECLARE
  nfstep int;	-- number of skipping frames in a step
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd bigint;	-- time period
  stmt text;
  rec alsep_tidalxy_type;	-- record returd by query
BEGIN
  -- number of skipping frames between a step
  nfstep := 2;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float * nfstep;

  -- time period (interval) [msec]
  tprd := dtstep * npts::bigint;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_tidal_query(tstamp0, tstamp2, 0);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql;


--
-- example:
--   psql -c "SELECT * FROM alsep_tidalzt('1972-11-20T01:30:55.999', 500)"
--   psql -c "SELECT * FROM alsep_tidalzt('1972-11-20T01:30:55.999)"
--   psql -c "SELECT * FROM alsep_tidalzt()"
--
CREATE OR REPLACE FUNCTION alsep_tidalzt(
  tstamp0 timestamp DEFAULT '1972-11-20T01:30:55.999' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_tidalzt_type AS $$
DECLARE
  nfstep int;	-- number of skipping frames in a step
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd bigint;	-- time period
  stmt text;
  rec alsep_tidalzt_type;	-- record returd by query
BEGIN
  -- number of skipping frames between a step
  nfstep := 2;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float * nfstep;

  -- time period (interval) [msec]
  tprd := dtstep * npts::bigint;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_tidal_query(tstamp0, tstamp2, 1);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql;


--
-- example:
--   psql -c "SELECT * FROM alsep_tidalxy('1977-09-26 19:12:48.338', '1977-09-26 19:19:09.91', 500) WHERE error_flag = 0;"
--   psql -c "SELECT * FROM alsep_tidalxy('1977-09-26 19:13:55.958', '1977-09-26 19:19:07.495', 500) WHERE error_flag = 0;"
--
CREATE OR REPLACE FUNCTION alsep_tidalxy(
  tstamp0 timestamp  -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_tidalxy_type AS $$
DECLARE
  nfstep int;	-- number of skipping frames in a step
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd bigint;	-- time period
  stmt text;
  rec alsep_tidalxy_type;	-- record returd by query
BEGIN
  -- number of skipping frames between a step
  nfstep := 2;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float * nfstep;

  -- time period (interval) [msec]
  tprd := dtstep * npts::bigint;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_tidal_query(tstamp0, tstamp2, 0);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql;


--
-- example:
--   psql -c "SELECT * FROM alsep_tidalzt('1977-09-26 19:12:47.734', '1977-09-26 19:19:11.721', 500) WHERE error_flag = 0;"
--   psql -c "SELECT * FROM alsep_tidalzt('1977-09-26 19:12:52.564', '1977-09-26 19:19:09.306', 500) WHERE error_flag = 0;"
--
CREATE OR REPLACE FUNCTION alsep_tidalzt(
  tstamp0 timestamp  -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_tidalzt_type AS $$
DECLARE
  nfstep int;	-- number of skipping frames in a step
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd bigint;	-- time period
  stmt text;
  rec alsep_tidalzt_type;	-- record returd by query
BEGIN
  -- number of skipping frames between a step
  nfstep := 2;

  -- frame time step in msec
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float * nfstep;

  -- time period (interval) [msec]
  tprd := dtstep * npts::bigint;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_tidal_query(tstamp0, tstamp2, 1);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql;
--
-- lsg data retrieval functions
--
DROP FUNCTION IF EXISTS  alsep_lsg(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lsg(time0 timestamp, time9 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lsg_query(time0 timestamp, time2 timestamp, dtframe int, ncarray int);
DROP TYPE IF EXISTS alsep_lsg_type;

--
-- alsep_lsg()
-- 	specify start time and number of points to retrieve lsg data from tbl_lsg table for every stations.
--

CREATE TYPE alsep_lsg_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, frame_count	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, nc	smallint
	, lsg	smallint
	, lsg_tide	smallint
	, lsg_free	smallint
	, lsg_temp	smallint
	, process_flag	smallint
	, error_flag	smallint
	, time_flag	smallint
);


CREATE OR REPLACE FUNCTION alsep_lsg_query(
  tstamp0 timestamp	
  ,tstamp2 timestamp
  ,dtframe int
  ,ncarray int
) RETURNS text AS $$
DECLARE
  tstamp1 timestamp;	-- start before 1 frame timestamp
  tstamp3 timestamp;	-- end after 1 step timestamp
  dtstep float;	-- data time step in msec
  arr_str text;	-- dummy array definition string
  s_query text;	-- part of query strings
  s_claus text;	-- where clause of query strings
  e_query text;	-- part of query strings
  e_claus text;	-- order by clause for query strings
  d_claus text;	-- where clause of query strings
  stmt text;	-- query statement to be executed
BEGIN
  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- Set dummy array
  arr_str := 'generate_series(1, ' || ncarray || ')';

  -- one frame before start timestamp
  tstamp1 := tstamp0 - (dtframe::text || ' milliseconds')::interval;

  -- one frame after end timestamp
  tstamp3 := tstamp2 + (dtframe::text || ' milliseconds')::interval;

  -- set query string
  s_query := 'f.ap_station, f.ground_station,  f.file_id, f.pos, f.length, f.frame_count,  f.time_diff, f.time, '||arr_str||' nc, unnest(f.lsg) lsg, f.lsg_tide, f.lsg_free, f.lsg_temp, f.process_flag, f.error_flag, f.time_flag FROM tbl_lsg f';
  e_query := 'e.ap_station, e.ground_station,  e.file_id, e.pos, e.length, e.frame_count,  e.time_diff, (e.time + (' || dtstep::text || '* (e.nc-1)||'' milliseconds'')::interval)::timestamp without time zone as time, e.nc, e.lsg, e.lsg_tide, e.lsg_free, e.lsg_temp, e.process_flag, e.error_flag, e.time_flag';
  --                                                                                      e.x.          (e.time + (      18.875          * (e.nc-1)|| ' milliseconds' )::interval)::timestamp 

  s_claus := '''' || tstamp1 || ''' <= f.time AND f.time <= ''' || tstamp3 || '''';
  e_claus := 'ORDER BY  e.ap_station, time';
  d_claus := '''' || tstamp0 || ''' <= d.time AND d.time < ''' || tstamp2 || '''';

  stmt := 'SELECT * FROM (SELECT ' || e_query || ' FROM (SELECT ' || s_query || ' WHERE ' || s_claus || ') AS e ' || e_claus || ') AS d WHERE ' || d_claus ;
  RAISE NOTICE 'stmt= %', stmt;

  RETURN stmt;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_lsg('1976-01-01 00:08:05.379', 500)"
--   psql -c "SELECT * FROM alsep_lsg('1976-01-01 00:08:05.379')"
--   psql -c "SELECT * FROM alsep_lsg();
--
CREATE OR REPLACE FUNCTION alsep_lsg(
  tstamp0 timestamp DEFAULT '1976-01-01 00:08:05.379' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_lsg_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lsg_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 31;

  -- data time step [msec]
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_lsg_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_lsg('1977-09-30 18:50:47.722', '1977-09-30 18:50:57.986', 500)"
--   psql -c "SELECT * FROM alsep_lsg('1977-09-30 18:50:48.326', '1977-09-30 18:50:57.986', 500)"
--   psql -c "SELECT * FROM alsep_lsg('1977-09-30 18:50:48.929', '1977-09-30 18:50:58.571', 500)"
--
CREATE OR REPLACE FUNCTION alsep_lsg(
  tstamp0 timestamp  -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_lsg_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lsg_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 31;

  -- data time step [msec]
  dtframe := 604;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_lsg_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;
--
-- lspe data retrieval functions
--
DROP FUNCTION IF EXISTS  alsep_lspe(time0 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lspe(time0 timestamp, time9 timestamp, npts int);
DROP FUNCTION IF EXISTS  alsep_lspe_query(time0 timestamp, time2 timestamp, dtframe int, ncarray int);
DROP TYPE IF EXISTS alsep_lspe_type;

--
-- alsep_lspe()
-- 	specify start time and number of points to retrieve lspe data from tbl_lspe table for every stations.
--

CREATE TYPE alsep_lspe_type AS (
	ap_station	smallint
	, ground_station	smallint
	, file_id	int
	, pos	int
	, length	smallint
	, time_diff	bigint
	, time	timestamp without time zone
	, nc	smallint
	, gp1	smallint
	, gp2	smallint
	, gp3	smallint
	, gp4	smallint
	, status	smallint
	, process_flag	smallint
	, error_flag	smallint
	, time_flag	smallint
);


CREATE OR REPLACE FUNCTION alsep_lspe_query(
  tstamp0 timestamp	
  ,tstamp2 timestamp
  ,dtframe int
  ,ncarray int
) RETURNS text AS $$
DECLARE
  tstamp1 timestamp;	-- start before 1 frame timestamp
  tstamp3 timestamp;	-- end after 1 step timestamp
  dtstep float;	-- data time step in msec
  arr_str text;	-- dummy array definition string
  s_query text;	-- part of query strings
  s_claus text;	-- where clause of query strings
  e_query text;	-- part of query strings
  e_claus text;	-- order by clause for query strings
  d_claus text;	-- where clause of query strings
  stmt text;	-- query statement to be executed
BEGIN
  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- Set dummy array
  arr_str := 'generate_series(1, ' || ncarray || ')';

  -- one frame before start timestamp
  tstamp1 := tstamp0 - (dtframe::text || ' milliseconds')::interval;

  -- one frame after end timestamp
  tstamp3 := tstamp2 + (dtframe::text || ' milliseconds')::interval;

  -- set query string
  s_query := 'f.ap_station, f.ground_station,  f.file_id, f.pos, f.length,  f.time_diff, f.time, '||arr_str||' nc, unnest(f.gp1) gp1 , unnest(f.gp2) gp2 , unnest(f.gp3) gp3 , unnest(f.gp4) gp4 , unnest(f.status) status, f.process_flag, f.error_flag, f.time_flag FROM tbl_lspe f';
  e_query := 'e.ap_station, e.ground_station,  e.file_id, e.pos, e.length,  e.time_diff, (e.time + (' || dtstep::text || '* (e.nc-1)||'' milliseconds'')::interval)::timestamp without time zone as time, e.nc, e.gp1, e.gp2, e.gp3, e.gp4, e.status, e.process_flag, e.error_flag, e.time_flag ';
  --                                                                                      e.x.          (e.time + (      18.875          * (e.nc-1)|| ' milliseconds' )::interval)::timestamp 

  s_claus := '''' || tstamp1 || ''' <= f.time AND f.time <= ''' || tstamp3 || '''';
  e_claus := 'ORDER BY  e.ap_station, time';
  d_claus := '''' || tstamp0 || ''' <= d.time AND d.time < ''' || tstamp2 || '''';

  stmt := 'SELECT * FROM (SELECT ' || e_query || ' FROM (SELECT ' || s_query || ' WHERE ' || s_claus || ') AS e ' || e_claus || ') AS d WHERE ' || d_claus ;
  RAISE NOTICE 'stmt= %', stmt;

  RETURN stmt;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_lspe('1976-01-01 00:00:03.039', 500)"
--   psql -c "SELECT * FROM alsep_lspe('1976-01-01 00:00:03.039')"
--   psql -c "SELECT * FROM alsep_lspe()"
--
CREATE OR REPLACE FUNCTION alsep_lspe(
  tstamp0 timestamp DEFAULT '1976-01-01 00:00:03.039' -- start timestamp
  ,npts int	    DEFAULT 500		  -- output number of points for each station
) RETURNS SETOF alsep_lspe_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lspe_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 20;

  -- data time step [msec]
  dtframe := 170;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  stmt := alsep_lspe_query(tstamp0, tstamp2, dtframe, ncarray);
  RAISE NOTICE 'stmt(CALLER)= %', stmt;
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;


--
-- example:
--   psql -c "SELECT * FROM alsep_lspe('1977-04-25 00:30:25.526', '1977-04-25 00:30:29.772', 500)"
--   psql -c "SELECT * FROM alsep_lspe('1977-04-25 00:30:25.696', '1977-04-25 00:30:29.772', 500)"
--   psql -c "SELECT * FROM alsep_lspe('1977-04-25 00:30:25.696', '1977-04-25 00:30:29.942', 500)"
--
CREATE OR REPLACE FUNCTION alsep_lspe(
  tstamp0 timestamp  -- start timestamp
  ,tstamp9 timestamp -- end timestamp
  ,npts int	     -- output number of points for each station
) RETURNS SETOF alsep_lspe_type AS $$
DECLARE
  ncarray int;	-- number of data in a frame array
  tstamp2 timestamp;	-- end timestamp
  dtframe int;	-- frame time step in msec
  dtstep float;	-- data time step in msec
  tprd float;	-- time period
  stmt text;	-- query statement to be executed
  rec alsep_lspe_type;	-- record returd by query
BEGIN
  -- number of items in a array(frame)
  ncarray := 20;

  -- data time step [msec]
  dtframe := 170;

  -- data time step [msec]
  dtstep := dtframe::float / ncarray;

  -- time period (interval) [msec]
  tprd := dtstep * npts;

  -- end timestamp
  tstamp2 := tstamp0 + (tprd::text || ' milliseconds')::interval;

  IF (tstamp2 > tstamp9) THEN
    tstamp2 := tstamp9;
  END IF;

  stmt := alsep_lspe_query(tstamp0, tstamp2, dtframe, ncarray);
  FOR rec IN EXECUTE stmt LOOP
    RETURN NEXT rec;
  END LOOP;

END;
$$ LANGUAGE plpgsql IMMUTABLE STRICT;
