# ft_traceroute

## Compilation
```
make
```

## Usage
```
sudo ./ft_traceroute [options] <destination>
```
(ft_traceroute need sudo to bind socket)
```
Usage:
  traceroute host
Options:
  -h                 print help and exit
  -f first_ttl       Start from the first_ttl hop (instead from 1)
  -m max_ttl         Set the max number of hops (max TTL to be reached). Default is 30
```

## Packet
Traceroute use udp packet with a payload of 40 bytes by default, this project use this structure (defined in ft_traceroute.h):

```
typedef struct		s_udp_packet
{
	struct iphdr	iphdr;
	struct udphdr	udphdr;
	char			data[DATA_SIZE];
}					t_udp_packet;
```