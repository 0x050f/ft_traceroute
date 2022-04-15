#include "ft_traceroute.h"

int			ft_traceroute(t_traceroute *traceroute)
{
	int port = DST_PORT_MIN + 1;
	int finished = 0;
	t_udp_packet		**udp_packets = NULL;
	t_icmp_packet		**icmp_packets = NULL;

	/* array of udp & icmp packets [NB_PROBES] */
	if (!(udp_packets = malloc(sizeof(t_udp_packet *) * NB_PROBES)) || 
	!(icmp_packets = malloc(sizeof(t_icmp_packet *) * NB_PROBES)))
	{
		dprintf(STDERR_FILENO, "%s: malloc: Error\n", traceroute->prg_name);
		free(udp_packets);
		return (1);
	}
	printf("traceroute to %s (%s) %d hops max, %ld byte packets\n", traceroute->hostname, traceroute->address, traceroute->ttl_val, sizeof(t_udp_packet));
	for (int i = traceroute->first_ttl; i <= traceroute->ttl_val && !finished; i++)
	{
		ft_memset(udp_packets, 0, sizeof(t_udp_packet *) * NB_PROBES);
		ft_memset(icmp_packets, 0, sizeof(t_icmp_packet *) * NB_PROBES);
		/* number of hops */
		printf("%2d ", i);
		/* attempt send */
		for (int j = 0; j < NB_PROBES; j++)
		{
			udp_packets[j] = send_packet(traceroute, port++, i);
			(port > DST_PORT_MAX) && (port = DST_PORT_MIN);
		}
		struct timeval start_recv;
		struct timeval time_recv;

		if (gettimeofday(&start_recv, NULL))
			dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
		/* attempt recv */
		for (int j = 0; j < NB_PROBES; j++)
		{
			icmp_packets[j] = recv_packet(traceroute, start_recv, udp_packets, NB_PROBES);
			if (gettimeofday(&time_recv, NULL))
				dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
			if (get_diff_ms(&start_recv, &time_recv) >= TIME_TO_WAIT * 1000.0)
				break ;
		}
		finished = print_packets(udp_packets, icmp_packets);
		printf("\n");
		/* free packets */
		for (int j = 0; j < NB_PROBES; j++)
		{
			free(icmp_packets[j]);
			free(udp_packets[j]);
		}
	}
	free(icmp_packets);
	free(udp_packets);
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
	struct timeval time;
	/* random for src port */
	if (gettimeofday(&time, NULL))
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", traceroute->prg_name);
	ft_srand(time.tv_usec);	
	/* test packet to get src_addr */
	t_udp_packet *packet = send_packet(traceroute, DST_PORT_MIN, 0);
	free(recv_packet(traceroute, time, &packet, 1));
	free(packet);
	if (!traceroute->options.f)
		traceroute->first_ttl = 1;
	if (!traceroute->options.m)
		traceroute->ttl_val = MAX_TTL_VALUE;
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
	if (ft_traceroute(&traceroute))
		return (1);
	return (0);
}
