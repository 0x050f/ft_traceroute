#include "ft_traceroute.h"

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
		char	host_ip[ADDR_SIZE];

		ft_memset(host_ip, 0, ADDR_SIZE);
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
				char	actual_ip[ADDR_SIZE];

				ft_strcpy(actual_ip, inet_ntoa(*((struct in_addr *)&found->iphdr.saddr)));
				if (ft_strcmp(host_ip, actual_ip))
				{
					ft_strcpy(host_ip, actual_ip);
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
