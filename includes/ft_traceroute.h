#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/udp.h>
# include <stdlib.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>

/* ==>  udp.h
struct udphdr {
	u_short	uh_sport;		source port
	u_short	uh_dport;		destination port
	short	uh_ulen;		udp length
	u_short	uh_sum;			udp checksum
};
*/

# define DATA_SIZE		12

/* ipv4
https://en.wikipedia.org/wiki/User_Datagram_Protocol */
typedef struct		s_udp_packet
{
	uint32_t		src_addr;
	uint32_t		dst_addr;
	uint8_t			zero; // = 0
	uint8_t			protocol;
	uint16_t		udp_len;
	struct udphdr	udphdr;
	char			data[DATA_SIZE];
}					t_udp_packet; /* 20 bytes + DATA_SIZE */

typedef struct		s_options
{
	int				h;
}					t_options;

# define ADDR_SIZE		64

typedef	struct			s_traceroute
{
	char				*prg_name;
	char				*hostname;
	char				address[ADDR_SIZE];
	uint32_t			ip_addr;
	struct sockaddr_in	sockaddr;
	int					sockfd;
	t_options			options;
}						t_traceroute;

# define NB_OPTIONS			1

# define ERR_NB_DEST		1
# define ERR_INV_OPT		2
# define ERR_INV_ARG		3
# define ERR_OOR_ARG		4 /* OUT OF RANGE */
# define ERR_REQ_ARG		5

# define MAX_TTL_VALUE		30
# define NB_PROBES			3

# define DST_PORT_MIN		33435
# define DST_PORT_MAX		33535

/* args.c */
int			check_args(int argc, char *argv[], t_traceroute *traceroute);

/* error.c */
void		show_help(void);
int			getaddrinfo_error(char *prg_name, int error, char *str);
int			args_error(char *prg_name, int error, char *str, int range1, int range2);

/* utils.c */
void		ft_bzero(void *s, size_t n);
size_t		ft_strlen(const char *s);
char		*ft_strcpy(char *dst, const char *src);

#endif
