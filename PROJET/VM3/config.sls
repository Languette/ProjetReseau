# Configuration eth1
# RAPPEL: eth0 est à vagrant, ne pas y toucher

## Désactivation de network-manager
NetworkManager:
  service:
    - dead
    - enable: False

## Suppression de la passerelle par défaut
ip route del default:
  cmd:
    - run

## Configuration de VM3
eth1:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.163
    - netmask: 28
eth2:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.183
    - netmask: 28

## Configuration de la route vers LAN1 via VM2
routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN1
        ipaddr: 172.16.2.128/28
        gateway: 172.16.2.162
      - name: LAN3
        ipaddr: 172.16.2.144/28
        gateway: 172.16.2.162

## enable ipv4 forwarding
net.ipv4.ip_forward:
  sysctl:
    - present
    - value: 1


