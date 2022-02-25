#include "ft_traceroute.h"

double		get_diff_ms(struct timeval *start, struct timeval *end)
{
	return ((double)((end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec) / 1000);
}

/* RFC 1071 https://datatracker.ietf.org/doc/html/rfc1071 */
unsigned short checksum(void *addr, size_t count)
{
	unsigned short *ptr;
	unsigned long sum;

	ptr = addr;
	for (sum = 0; count > 1; count -= 2)
		sum += *ptr++;
	if (count > 0)
		sum += *(unsigned char *)ptr;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

unsigned short udp4_checksum(t_traceroute *traceroute, t_udp_packet *packet)
{
	t_udp_pseudo_hdr	udp_pseudo_hdr;

	ft_bzero(&udp_pseudo_hdr, sizeof(t_udp_pseudo_hdr));
	ft_memcpy(&udp_pseudo_hdr.udphdr, &packet->udphdr, sizeof(t_udp_packet) - sizeof(struct iphdr));
	udp_pseudo_hdr.src_addr = traceroute->src_addr;
	udp_pseudo_hdr.dst_addr = traceroute->ip_addr;
	udp_pseudo_hdr.zero = 0;
	udp_pseudo_hdr.protocol = IPPROTO_UDP;
	udp_pseudo_hdr.udp_len = packet->udphdr.uh_ulen;
	return (checksum(&udp_pseudo_hdr, sizeof(t_udp_pseudo_hdr)));
}

void	fill_ip_header(t_traceroute *traceroute, struct iphdr *iphdr, int ttl)
{
	iphdr->version = 4; // ipv4
	iphdr->ihl = sizeof(struct iphdr) / 4; // 5 = 20 / 32 bits
	iphdr->tos = 0;
	iphdr->tot_len = sizeof(t_udp_packet);
	iphdr->id = 0;
	iphdr->frag_off = 0;
	iphdr->ttl = ttl;
	iphdr->protocol = IPPROTO_UDP;
	iphdr->check = 0; // filled by kernel
	iphdr->saddr = INADDR_ANY;
	iphdr->daddr = traceroute->ip_addr;
}

void	send_packet(t_traceroute *traceroute, int dstport, int ttl)
{
	t_udp_packet		packet;

	ft_bzero(&packet, sizeof(t_udp_packet));
	fill_ip_header(traceroute, &packet.iphdr, ttl);
	// packet.udphdr.uh_sport = not needed ?
	packet.udphdr.uh_dport = htons(dstport);
	packet.udphdr.uh_ulen = htons(sizeof(t_udp_packet) - sizeof(struct iphdr)); // 0x2800 -> 0x0028
	ft_memset(packet.data, 0x42, DATA_SIZE);
	packet.udphdr.uh_sum = udp4_checksum(traceroute, &packet);
	if (sendto(traceroute->sockfd_udp, &packet, sizeof(packet), 0, (struct sockaddr *)&traceroute->sockaddr, sizeof(struct sockaddr)) < 0)
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", traceroute->prg_name);
	if (gettimeofday(&traceroute->last_time, NULL))
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
}

int		recv_packet(t_traceroute *traceroute, int first_probe)
{
	t_icmp_packet		packet;
	struct sockaddr_in	r_addr;
	unsigned int		addr_len;
	struct timeval		tv;

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (setsockopt(traceroute->sockfd_icmp, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
		dprintf(STDERR_FILENO, "%s: setsockopt: Error\n", traceroute->prg_name);
	addr_len = sizeof(struct sockaddr_in);
	if (recvfrom(traceroute->sockfd_icmp, &packet, sizeof(packet), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0)
		printf(" *");
	else
	{
		if (gettimeofday(&tv, NULL))
			dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
		if (!first_probe)
			printf(" %s", inet_ntoa(*((struct in_addr *)&packet.iphdr.saddr)));
		traceroute->src_addr = packet.iphdr.daddr;
		printf(" %.3f ms", get_diff_ms(&traceroute->last_time, &tv));
		if (packet.icmphdr.type == ICMP_DEST_UNREACH)
			return (1);
		return (2);
	}
	return (0);
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
	traceroute->src_addr = 16777343; // 127.0.0.1
	traceroute->sockfd_udp = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (traceroute->sockfd_udp < 0)
	{
		dprintf(STDERR_FILENO, "%s: socket: Operation not permitted\n", traceroute->prg_name);
		return (1);
	}
	traceroute->sockfd_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (traceroute->sockfd_icmp < 0)
	{
		dprintf(STDERR_FILENO, "%s: socket: Operation not permitted\n", traceroute->prg_name);
		return (1);
	}
	int on = 1;
	setsockopt(traceroute->sockfd_udp, IPPROTO_IP, IP_HDRINCL, (const char *)&on, sizeof(on));
	setsockopt(traceroute->sockfd_icmp, IPPROTO_IP, IP_HDRINCL, (const char *)&on, sizeof(on));
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
	int i;
	for (i = 1; i <= MAX_TTL_VALUE && finished != NB_PROBES; i++)
	{
		printf("%2d ", i);
		// use select instead ?
		int found = 0;
		int j = 0;
		while (++j <= NB_PROBES)
		{
			send_packet(&traceroute, port++, i);
			(port > DST_PORT_MAX) && (port = DST_PORT_MIN);
			found = recv_packet(&traceroute, found);
			if (found == 1)
				finished += 1;
		}
		printf("\n");
	}
	return (0);
}
