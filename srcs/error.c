#include "ft_traceroute.h"

void		show_help(void)
{
	char *options[NB_OPTIONS][2] =
	{
		{"-h", "print help and exit"},
		{"-f first_ttl", "Start from the first_ttl hop (instead from 1)"},
		{"-m max_ttl", "Set the max number of hops (max TTL to be reached). Default is 30"}
	};

	printf("Usage:\n  traceroute host\nOptions:\n");
	for (size_t i = 0; i < NB_OPTIONS; i++)
		printf("  %-18s %s\n", options[i][0], options[i][1]);
}

int			getaddrinfo_error(char *prg_name, int error, char *str)
{
	dprintf(STDERR_FILENO, "%s: %s: ", prg_name, str);
	if (error == EAI_AGAIN || error == EAI_FAIL)
		dprintf(STDERR_FILENO, "Temporary failure in name resolution\n");
	else if (error == EAI_NONAME)
		dprintf(STDERR_FILENO, "Name or service not known\n");
	else
		dprintf(STDERR_FILENO, "Error\n");
	return (2);
}

int			args_error(char *prg_name, int error, char *str, int range1, int range2)
{
	dprintf(STDERR_FILENO, "%s: ", prg_name);
	if (error == ERR_INV_OPT)
		dprintf(STDERR_FILENO, "invalid option -- '%s'\n", str);
	else if (error == ERR_INV_ARG || error == ERR_OOR_ARG)
	{
		dprintf(STDERR_FILENO, "invalid argument: '%s'", str);
		if (error == ERR_OOR_ARG)
			dprintf(STDERR_FILENO, ": out of range: %d <= value <= %d", range1, range2);
		dprintf(STDERR_FILENO, "\n");
	}
	else if (error == ERR_REQ_ARG)
		dprintf(STDERR_FILENO, "option requires an argument -- '%s'\n", str);
	return (2);
}
