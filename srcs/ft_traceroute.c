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
	return (0);
}
