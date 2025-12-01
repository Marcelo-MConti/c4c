#!/bin/sh
#
# Cria network namespaces Linux usando o comando `ip`
# para que seja possível testar e depurar o modo
# netplay/PvP online/remoto usando apenas um host.

: ${BRIDGE:=br0}

: ${NS0:=p0}
: ${NS1:=p1}

: ${VETH0:=veth0}
: ${VETH1:=veth1}

: ${BRIDGE_ADDR:=10.10.0.1}
: ${ADDR0:=10.10.0.5}
: ${ADDR1:=10.10.0.6}

: ${SUBNET:=8}

sudo ip netns add $NS0
sudo ip netns add $NS1

sudo ip netns exec $NS0 ip link set lo up
sudo ip netns exec $NS1 ip link set lo up

sudo ip link add $BRIDGE type bridge
sudo ip link set $BRIDGE up
sudo ip addr add $BRIDGE_ADDR/$SUBNET dev $BRIDGE

sudo ip link add $VETH0 type veth peer name ceth0
sudo ip link set $VETH0 master $BRIDGE
sudo ip link set $VETH0 up
sudo ip link set ceth0 netns $NS0
sudo ip netns exec $NS0 ip link set ceth0 up

sudo ip link add $VETH1 type veth peer name ceth1
sudo ip link set $VETH1 master $BRIDGE
sudo ip link set $VETH1 up
sudo ip link set ceth1 netns $NS1
sudo ip netns exec $NS1 ip link set ceth1 up

sudo ip netns exec $NS0 ip addr add $NS0_ADDR/$SUBNET dev ceth0
sudo ip netns exec $NS1 ip addr add $NS1_ADDR/$SUBNET dev ceth1

sudo ip netns exec $NS0 ping -c 2 $NS1_ADDR || echo "Couldn't ping $NS1 from $NS0!" >&2
sudo ip netns exec $NS1 ping -c 2 $NS0_ADDR || echo "Couldn't ping $NS0 from $NS1!" >&2

cat <<EOF
All done!

Run:

 - `sudo ip netns exec $NS0 ./c4c` (use $NS1_ADDR as the peer address)
 - `sudo ip netns exec $NS1 ./c4c` (use $NS0_ADDR as the peer address)
EOF
