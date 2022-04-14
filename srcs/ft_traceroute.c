#include "ft_traceroute.h"

double		get_diff_ms(struct timeval *start, struct timeval *end)
{
	return ((double)((end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec) / 1000);
}

long	ft_floor(double f)
{
	if(f >= 0.0) {
		return ((int)f);
	} else {
		return ((int)f - 1);
	}
}

struct timeval	get_diff_timeval(float start, float end)
{
	struct timeval time;

	time.tv_sec = ft_floor(start - end);
	time.tv_usec = ((float)time.tv_sec - (start - end) * 1000000);
	return (time);
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

t_udp_packet	*send_packet(t_traceroute *traceroute, int dstport, int ttl)
{
	struct timeval		time;
	t_udp_packet		*packet;

	packet = malloc(sizeof(t_udp_packet));
	ft_memset(packet, 0, sizeof(t_udp_packet));
	if (!packet)
		return (NULL);
	ft_bzero(packet, sizeof(t_udp_packet));
	fill_ip_header(traceroute, &packet->iphdr, ttl);
	// packet->udphdr.uh_sport = not needed ?
	packet->udphdr.uh_dport = htons(dstport);
	packet->udphdr.uh_ulen = htons(sizeof(t_udp_packet) - sizeof(struct iphdr)); // 0x2800 -> 0x0028
	if (gettimeofday(&time, NULL))
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
	ft_memcpy(packet->data, &time, sizeof(struct timeval));
	ft_memset(packet->data + sizeof(struct timeval), 0x42, DATA_SIZE - sizeof(struct timeval));
	packet->udphdr.uh_sum = udp4_checksum(traceroute, packet);
	if (sendto(traceroute->sockfd_udp, packet, sizeof(t_udp_packet), 0, (struct sockaddr *)&traceroute->sockaddr, sizeof(struct sockaddr)) < 0)
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", traceroute->prg_name);
	return (packet);
}

t_icmp_packet	*recv_packet(t_traceroute *traceroute, struct timeval start_recv, t_udp_packet **udp_packets, int nb_probes)
{
	t_icmp_packet		*packet;
	struct sockaddr_in	r_addr;
	unsigned int		addr_len;
	struct timeval		tv;

	packet = malloc(sizeof(t_icmp_packet));
	ft_memset(packet, 0, sizeof(t_icmp_packet));
	if (!packet)
		return (NULL);
	tv.tv_sec = TIME_TO_WAIT;
	tv.tv_usec = 0;
	if (setsockopt(traceroute->sockfd_icmp, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
		dprintf(STDERR_FILENO, "%s: setsockopt: Error\n", traceroute->prg_name);
	addr_len = sizeof(struct sockaddr_in);
	int found = 0;
	while (!found)
	{
		if (recvfrom(traceroute->sockfd_icmp, packet, sizeof(t_icmp_packet) - sizeof(struct timeval), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0)
		{
			free(packet);
			return (NULL);
		}
		int i = 0;
		for (i = 0; i < nb_probes; i++)
		{
			if (((t_udp_packet *)packet->data)->udphdr.check == udp_packets[i]->udphdr.check)
				break ;
		}
		struct timeval		packet_time;
		/* gettimeofday + copy time into packet received */
		if (gettimeofday(&packet_time, NULL))
			dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
		packet->time = packet_time;
		if (i != nb_probes || get_diff_ms(&start_recv, &packet_time) >= TIME_TO_WAIT * 1000.0)
			found = 1;
		else
			tv = get_diff_timeval(TIME_TO_WAIT * 1000.0, get_diff_ms(&start_recv, &packet_time));
	}
	return (packet);
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
	int i;
	int j;
	int finished = 0;
	for (i = 1; i <= MAX_TTL_VALUE && !finished; i++)
	{
		t_udp_packet		**udp_packets;
		t_icmp_packet		**icmp_packets;

		udp_packets = malloc(sizeof(t_udp_packet *) * NB_PROBES);
		icmp_packets = malloc(sizeof(t_icmp_packet *) * NB_PROBES);
		ft_memset(udp_packets, 0, sizeof(t_udp_packet *) * NB_PROBES);
		ft_memset(icmp_packets, 0, sizeof(t_udp_packet *) * NB_PROBES);
		printf("%2d ", i);
		for (j = 0; j < NB_PROBES; j++)
		{
			udp_packets[j] = send_packet(&traceroute, port++, i);
			(port > DST_PORT_MAX) && (port = DST_PORT_MIN);
		}
		struct timeval start_recv;
		struct timeval time_recv;

		if (gettimeofday(&start_recv, NULL))
			dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute.prg_name);
		for (j = 0; j < NB_PROBES; j++)
		{
			icmp_packets[j] = recv_packet(&traceroute, start_recv, udp_packets, NB_PROBES);
			if (gettimeofday(&time_recv, NULL))
				dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute.prg_name);
			if (get_diff_ms(&start_recv, &time_recv) >= TIME_TO_WAIT * 1000.0)
				break ;
		}
		char	*host_ip = NULL;
		for (j = 0; j < NB_PROBES; j++)
		{
			t_icmp_packet	*found = NULL;
			for (int k = 0; k < NB_PROBES; k++)
			{
				if (udp_packets[j] && icmp_packets[k] &&
udp_packets[j]->udphdr.uh_dport == ((t_udp_packet *)icmp_packets[k]->data)->udphdr.uh_dport)
					found = icmp_packets[k];
			}
			if (!found || get_diff_ms((struct timeval *)&udp_packets[j]->data, &found->time) > TIME_TO_WAIT * 1000.0)
				printf(" *");
			else
			{
				if (!host_ip || ft_strcmp(host_ip, inet_ntoa(*((struct in_addr *)&found->iphdr.saddr))))
				{
					host_ip = inet_ntoa(*((struct in_addr *)&found->iphdr.saddr));
					printf(" %s", host_ip);
				}
				printf(" %.3f ms", get_diff_ms((struct timeval *)&udp_packets[j]->data, &found->time));
			if (found->icmphdr.type == ICMP_DEST_UNREACH)
					finished = 1;
			}
		}
		printf("\n");
		for (j = 0; j < NB_PROBES; j++)
		{
			free(icmp_packets[j]);
			free(udp_packets[j]);
		}
		free(icmp_packets);
		free(udp_packets);
	}
	return (0);
}
