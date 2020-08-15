echo "--------------------------Make--------------------------"
make
echo "--------------------------Remove------------------------"
sudo rmmod hg5_MP3
echo "--------------------------Install--------------------------"
sudo insmod hg5_MP3.ko
echo "--------------------------Mknod---------------------------"
rm node
sudo mknod node c $1 0
sudo chmod 777 node
