#include "ft_traceroute.h"

t_icmp_packet	*recv_packet(t_traceroute *traceroute, struct timeval start_recv, t_udp_packet **udp_packets, int nb_probes)
{
	t_icmp_packet		*packet;
	struct sockaddr_in	r_addr;
	unsigned int		addr_len;
	struct timeval		tv;

	packet = malloc(sizeof(t_icmp_packet));
	if (!packet)
		return (NULL);
	ft_memset(packet, 0, sizeof(t_icmp_packet));
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
