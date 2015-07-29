USE mysql;

-- delete the old users
DELETE FROM user WHERE user != 'root';
DELETE FROM user WHERE host != 'localhost';

-- add a new user with full permissions
--    only allowed from localhost, password required.
GRANT ALL ON *.* TO trox@localhost IDENTIFIED BY 'trox'
