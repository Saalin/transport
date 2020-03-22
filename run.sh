# /usr/bin/time -v ./transport-client-fast 156.17.4.30 40002 a.bin 9000000 
# /usr/bin/time -v ./transport 156.17.4.30 40002 b.bin 9000000

./transport-client-fast 156.17.4.30 40002 a.bin 1000000 
./transport 156.17.4.30 40002 b.bin 1000000

# 1169001
cmp a.bin b.bin