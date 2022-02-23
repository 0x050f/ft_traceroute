#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <arpa/inet.h>
# include <netdb.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>

# define ADDR_SIZE	64

# define NB_OPTIONS	1

# define DATA_SIZE	56

/* ipv4
https://en.wikipedia.org/wiki/User_Datagram_Protocol */
typedef struct		s_udp_packet
{
	int				src_addr; // ipv4 -			4 bytes
	int				dst_addr; // ipv4 -			4 bytes
	char			zero; // = 0 -				1 byte
	char			protocol; //				1 byte
	short			udp_len; //					2 bytes
	short			src_port; //				2 bytes
	short			dst_port; //				2 bytes
	short			len; //						2 bytes
	short			checksum; //				2 bytes
	char			data[DATA_SIZE];
}					t_udp_packet; /* 20 bytes + DATA_SIZE */

typedef struct		s_options
{
	int				h;
}					t_options;

typedef	struct	s_traceroute
{
	char			*prg_name;
	char			*hostname;
	char			address[ADDR_SIZE];
	uint32_t		ip_addr;
	int				sockfd;
	t_udp_packet	packet;
	t_options		options;
}				t_traceroute;

# define ERR_NB_DEST		1
# define ERR_INV_OPT		2
# define ERR_INV_ARG		3
# define ERR_OOR_ARG		4 /* OUT OF RANGE */
# define ERR_REQ_ARG		5

void		show_help(void);

/* args.c */
int			check_args(int argc, char *argv[], t_traceroute *traceroute);

/* error.c */
int			getaddrinfo_error(char *prg_name, int error, char *str);
int			args_error(char *prg_name, int error, char *str, int range1, int range2);

/* utils.c */
void		ft_bzero(void *s, size_t n);
size_t		ft_strlen(const char *s);

#endif
