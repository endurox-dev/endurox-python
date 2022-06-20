CREATE TABLE pyaccounts
(
        accnum varchar2(50) NOT NULL,
        balance number(20,0) NOT NULL,
        CONSTRAINT pyaccounts_pk PRIMARY KEY (accnum)
);

