#Setting IP Addr

##Laptop (TM-X Mockup)
sudo ip addr add 192.168.99.1/24 dev eno1
sudo ip link set eno1 up

##Raspberrypi
sudo ip addr add 192.168.99.2/24 dev eth0
sudo ip link set eth0 up



