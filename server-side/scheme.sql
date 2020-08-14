create table if not exists climate 
(
    id          integer primary key autoincrement,
    temperature real,
    humidity    real,
    room        text,
    day         integer,
    month       integer,
    year        integer,
    hour        integer,
    minute      integer
);

