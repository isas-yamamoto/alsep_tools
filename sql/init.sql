DROP TABLE tbl_pse CASCADE;

CREATE TABLE tbl_pse (
    file_id integer NOT NULL,
    pos integer NOT NULL,
    length smallint,
    frame_count smallint,
    ap_station smallint,
    ground_station smallint,
    time_original timestamp without time zone,
    "time" timestamp without time zone,
    time_diff bigint,
    sp_z smallint[],
    lp_x smallint[],
    lp_y smallint[],
    lp_z smallint[],
    tidal_x smallint,
    tidal_y smallint,
    tidal_z smallint,
    inst_temp smallint,
    process_flag smallint,
    error_flag smallint,
    time_flag smallint
);

DROP TABLE tbl_lsg CASCADE;
CREATE TABLE tbl_lsg (
    file_id integer NOT NULL,
    pos integer NOT NULL,
    length smallint,
    frame_count smallint,
    ap_station smallint,
    ground_station smallint,
    time_original timestamp without time zone,
    "time" timestamp without time zone,
    time_diff bigint,
    lsg smallint[],
    lsg_tide smallint,
    lsg_free smallint,
    lsg_temp smallint,
    process_flag smallint,
    error_flag smallint,
    time_flag smallint
);

DROP TABLE tbl_lspe CASCADE;
CREATE TABLE tbl_lspe (
    file_id integer NOT NULL,
    pos integer NOT NULL,
    length smallint,
    ap_station smallint,
    ground_station smallint,
    time_original timestamp without time zone,
    "time" timestamp without time zone,
    time_diff bigint,
    gp1 smallint[],
    gp2 smallint[],
    gp3 smallint[],
    gp4 smallint[],
    status smallint[],
    process_flag smallint,
    error_flag smallint,
    time_flag smallint
);
