#include "ft_traceroute.h"

void		log_packet(char *host_ip, t_icmp_packet *packet, float diff)
{
	char	actual_ip[ADDR_SIZE];

	if (!packet || diff > TIME_TO_WAIT * 1000.0)
		printf(" *");
	else
	{
		ft_strcpy(actual_ip, inet_ntoa(*((struct in_addr *)&packet->iphdr.saddr)));
		if (ft_strcmp(host_ip, actual_ip))
		{
			ft_strcpy(host_ip, actual_ip);
			printf(" %s", host_ip);
		}
		printf(" %.3f ms", diff);
	}
}

int		print_packets(t_udp_packet **udp_packets, t_icmp_packet **icmp_packets)
{
	int		finished = 0;
	char	host_ip[ADDR_SIZE];

	ft_memset(host_ip, 0, ADDR_SIZE);
	for (int j = 0; j < NB_PROBES; j++)
	{
		t_icmp_packet	*found = NULL;
		/* get packet from udp send (match placement/nb) */
		for (int k = 0; k < NB_PROBES && udp_packets[j]; k++)
		{
			if (icmp_packets[k] &&
udp_packets[j]->udphdr.uh_dport == ((t_udp_packet *)icmp_packets[k]->data)->udphdr.uh_dport)
				found = icmp_packets[k];
		}
		/* print packet recv */
		float diff = 0.0;
		if (found)
		{
			diff = get_diff_ms((struct timeval *)&udp_packets[j]->data, &found->time);
			if (found->icmphdr.type == ICMP_DEST_UNREACH)
				finished = 1;
		}
		log_packet(host_ip, found, diff);
	}
	return (finished);
}
