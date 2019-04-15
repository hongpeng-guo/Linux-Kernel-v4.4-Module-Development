rm profile1.data
nice ./work 1024 L 10000 & 
nice ./work 1024 R 50000 &
./monitor > profile2.data &
