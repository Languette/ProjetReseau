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

##Configuration de VM3


eth1:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - enable_ipv4: false
    - ipv6proto: static
    - enable_ipv6: true
    - ipv6_autoconf: no
    - ipv6ipaddr: fc00:1234:2::36
    - ipv6netmask: 64

eth2:
  network.managed:
    - enabled: True
    - type: eth
    - enable_ipv4: true
    - enable_ipv6: false
    - proto: none
    - ipaddr: 172.16.2.186
    - netmask: 28

## Configuration de la route vers LAN2 via VM2
routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN1-6
        ipaddr6: fc00:1234:1::/64
        gateway: fc00:1234:2::26
    - name: eth2
    - routes:
      - name: LAN1
        ipaddr: 172.16.2.128/28
        gateway: 172.16.2.183
      - name: LAN2
        ipaddr: 172.16.2.160/28
        gateway: 172.16.2.183
      - name: LAN3
        ipaddr: 172.16.2.144/28
        gateway: 172.16.2.183




