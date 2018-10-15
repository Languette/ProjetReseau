# Configuration eth1 et eth2
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

## Configuration de VM2
eth1:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.132
    - netmask: 28

eth2:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.162
    - netmask: 28

routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN3
        ipaddr: 172.16.2.128/28
        gateway: 172.16.2.131
    - name: eth2
    - routes:
      - name: LAN4
        ipaddr: 172.16.2.176/28
        gateway: 172.16.2.183

## But enable ipv4 forwarding
net.ipv4.ip_forward:
  sysctl:
    - present
    - value: 1





