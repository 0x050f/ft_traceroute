#include "ft_traceroute.h"

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

	ft_memset(&udp_pseudo_hdr, 0, sizeof(t_udp_pseudo_hdr));
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
	if (!packet)
		return (NULL);
	ft_memset(packet, 0, sizeof(t_udp_packet));
	fill_ip_header(traceroute, &packet->iphdr, ttl);
	packet->udphdr.uh_sport = htons(ft_rand());
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
