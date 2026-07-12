CREATE TABLE arduino_sensor
(
    id          integer
        primary key,
    humidity    float,
    light       integer,
    pass_value  bigint,
    posted_at   timestamp not null,
    temperature float
);