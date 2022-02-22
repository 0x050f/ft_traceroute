#include "ft_traceroute.h"

void		show_help(void)
{
	printf("Usage:\n  traceroute host\n");
}

int			main(int ac, char **av)
{
	int				ret;
	t_traceroute	traceroute;

	if ((ret = check_args(ac, av, &traceroute)))
		return (ret);
	if (traceroute.options.h)
		return (0);
}
