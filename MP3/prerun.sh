echo "--------------------------Make--------------------------"
make
echo "--------------------------Remove------------------------"
sudo rmmod mp3
echo "--------------------------Install--------------------------"
sudo insmod mp3.ko
echo "--------------------------Mknod---------------------------"
rm node
sudo mknod node c $1 0
sudo chmod 777 node
