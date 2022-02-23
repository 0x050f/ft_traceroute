#include "ft_traceroute.h"

void send_packet(t_traceroute *traceroute, int dstport)
{
	t_udp_packet	packet;

	ft_bzero(&packet, sizeof(t_udp_packet));
	// packet.src_addr
	packet.dst_addr = traceroute->ip_addr;
	packet.protocol = 0x11; // UDP
	packet.udp_len = sizeof(struct udphdr) + sizeof(packet.data);
	// packet.udphdr.uh_sport
	packet.udphdr.uh_dport = dstport;
	packet.udphdr.uh_ulen = sizeof(struct udphdr) + sizeof(packet.data);
	// packet.udphdr.uh_sum = checksum
	(void)packet;
	(void)traceroute;
	if (sendto(traceroute->sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&traceroute->sockaddr, sizeof(struct sockaddr)) < 0)
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", traceroute->prg_name);
	else
		printf("packet sent\n");
}

void recv_packet(t_traceroute *traceroute)
{
	t_udp_packet	packet;

	(void)packet;
	(void)traceroute;
//	recvfrom(traceroute->sockfd, );
// recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
	printf("recv packet\n");
}

int			init_traceroute(t_traceroute *traceroute)
{
	int ret;

	ret = inet_pton(AF_INET, traceroute->address, &traceroute->ip_addr);
	if (ret <= 0)
	{
		dprintf(STDERR_FILENO, "%s: inet_pton: Error\n", traceroute->prg_name);
		return (1);
	}
	traceroute->sockaddr.sin_addr.s_addr = traceroute->ip_addr;
	traceroute->sockaddr.sin_family = AF_INET;
	traceroute->sockaddr.sin_port = 0;
	traceroute->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (traceroute->sockfd < 0)
	{
		dprintf(STDERR_FILENO, "%s: socket: Operation not permitted\n", traceroute->prg_name);
		return (1);
	}
	int on = 1;
	setsockopt(traceroute->sockfd, IPPROTO_IP, IP_HDRINCL, (const char *)&on, sizeof(on));
	return (0);
}

int			main(int ac, char **av)
{
	int				ret;
	t_traceroute	traceroute;

	if ((ret = check_args(ac, av, &traceroute)))
		return (ret);
	if (traceroute.options.h)
		return (0);
	if (init_traceroute(&traceroute))
		return (1);
	int port = DST_PORT_MIN;
	int finished = 0;
	for (int i = 0; i < MAX_TTL_VALUE && !finished; i++)
	{
		// change ttl
		setsockopt(traceroute.sockfd, SOL_IP, IP_TTL, &i, sizeof(int));
		// use select instead ?
		int j = 0;
		while (++j <= NB_PROBES)
		{
			send_packet(&traceroute, port++);
			(port > DST_PORT_MAX) && (port = DST_PORT_MIN);
		}
		while (--j)
			recv_packet(&traceroute);
	}
	return (0);
}
