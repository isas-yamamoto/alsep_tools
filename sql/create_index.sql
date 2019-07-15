ALTER TABLE ONLY tbl_pse ADD CONSTRAINT tbl_pse_pkey PRIMARY KEY (file_id, pos);
CREATE INDEX idx_pse_file_id ON tbl_pse(file_id);
CREATE INDEX idx_pse_ap_station ON tbl_pse(ap_station);
CREATE INDEX idx_pse_ground_station ON tbl_pse(ground_station);
CREATE INDEX idx_pse_time ON tbl_pse(time);
CREATE INDEX idx_pse_process_flag ON tbl_pse(process_flag);
CREATE INDEX idx_pse_error_flag ON tbl_pse(error_flag);
CREATE INDEX idx_pse_time_flag ON tbl_pse(time_flag);
CREATE INDEX idx_pse_ap_station_time ON tbl_pse(ap_station,time);

ALTER TABLE ONLY tbl_lsg ADD CONSTRAINT tbl_lsg_pkey PRIMARY KEY (file_id, pos);
CREATE INDEX idx_lsg_file_id ON tbl_lsg(file_id);
CREATE INDEX idx_lsg_ap_station ON tbl_lsg(ap_station);
CREATE INDEX idx_lsg_ground_station ON tbl_lsg(ground_station);
CREATE INDEX idx_lsg_time ON tbl_lsg(time);
CREATE INDEX idx_lsg_process_flag ON tbl_lsg(process_flag);
CREATE INDEX idx_lsg_error_flag ON tbl_lsg(error_flag);
CREATE INDEX idx_lsg_time_flag ON tbl_lsg(time_flag);
CREATE INDEX idx_lsg_ap_station_time ON tbl_lsg(ap_station,time);

ALTER TABLE ONLY tbl_lspe ADD CONSTRAINT tbl_lspe_pkey PRIMARY KEY (file_id, pos);
CREATE INDEX idx_lspe_file_id ON tbl_lspe(file_id);
CREATE INDEX idx_lspe_ap_station ON tbl_lspe(ap_station);
CREATE INDEX idx_lspe_ground_station ON tbl_lspe(ground_station);
CREATE INDEX idx_lspe_time ON tbl_lspe(time);
CREATE INDEX idx_lspe_process_flag ON tbl_lspe(process_flag);
CREATE INDEX idx_lspe_error_flag ON tbl_lspe(error_flag);
CREATE INDEX idx_lspe_time_flag ON tbl_lspe(time_flag);
CREATE INDEX idx_lspe_ap_station_time ON tbl_lspe(ap_station,time);
