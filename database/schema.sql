PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS roads (
    road_id          TEXT    PRIMARY KEY,
    road_name        TEXT    NOT NULL,
    vehicle_count    INTEGER NOT NULL DEFAULT 0 CHECK (vehicle_count >= 0),
    traffic_density  INTEGER NOT NULL DEFAULT 0 CHECK (traffic_density BETWEEN 0 AND 100),
    waiting_time     INTEGER NOT NULL DEFAULT 0 CHECK (waiting_time >= 0),
    emergency_status INTEGER NOT NULL DEFAULT 0,
    current_priority INTEGER NOT NULL DEFAULT 0,
    signal_status    TEXT    NOT NULL DEFAULT 'RED'
                     CHECK (signal_status IN ('RED','YELLOW','GREEN')),
    active_status    INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS traffic_history (
    record_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    road_id          TEXT    NOT NULL,
    vehicle_count    INTEGER NOT NULL,
    traffic_density  INTEGER NOT NULL,
    waiting_time     INTEGER NOT NULL,
    timestamp        TEXT    NOT NULL,
    FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS signal_history (
    record_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    road_id          TEXT    NOT NULL,
    previous_signal  TEXT    NOT NULL,
    new_signal       TEXT    NOT NULL,
    timestamp        TEXT    NOT NULL,
    FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS emergency_events (
    event_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    road_id          TEXT    NOT NULL,
    emergency_type   TEXT    NOT NULL DEFAULT 'Ambulance',
    start_time       TEXT    NOT NULL,
    end_time         TEXT    NOT NULL DEFAULT '',
    status           TEXT    NOT NULL DEFAULT 'ACTIVE'
                     CHECK (status IN ('ACTIVE','COMPLETED')),
    FOREIGN KEY (road_id) REFERENCES roads(road_id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_traffic_road ON traffic_history (road_id);
CREATE INDEX IF NOT EXISTS idx_signal_road ON signal_history (road_id);
CREATE INDEX IF NOT EXISTS idx_emergency_road ON emergency_events (road_id);
CREATE INDEX IF NOT EXISTS idx_emergency_live ON emergency_events (status);