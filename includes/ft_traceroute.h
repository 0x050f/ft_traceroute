#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/udp.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <stdlib.h>
# include <stdio.h>
# include <sys/time.h>
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

# define DATA_SIZE		32

typedef struct		s_icmp_packet
{
	struct iphdr	iphdr;
	struct icmphdr	icmphdr;
	char			data[DATA_SIZE];
	struct timeval	time;
}					t_icmp_packet;

/* https://en.wikipedia.org/wiki/User_Datagram_Protocol */
typedef struct		s_udp_pseudo_hdr
{
	uint32_t		src_addr;
	uint32_t		dst_addr;
	uint8_t			zero;
	uint8_t			protocol;
	uint16_t		udp_len;
	struct udphdr	udphdr;
	char			data[DATA_SIZE];
}					t_udp_pseudo_hdr;

typedef struct		s_udp_packet
{
	struct iphdr	iphdr;
	struct udphdr	udphdr;
	char			data[DATA_SIZE];
}					t_udp_packet;

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
	uint32_t			src_addr;
	struct sockaddr_in	sockaddr;
	int					sockfd_udp;
	int					sockfd_icmp;
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
# define TIME_TO_WAIT		5

# define DST_PORT_MIN		33435
# define DST_PORT_MAX		33535

/* args.c */
int			check_args(int argc, char *argv[], t_traceroute *traceroute);

/* error.c */
void		show_help(void);
int			getaddrinfo_error(char *prg_name, int error, char *str);
int			args_error(char *prg_name, int error, char *str, int range1, int range2);

/* utils.c */
size_t		ft_strlen(const char *s);
char		*ft_strcpy(char *dst, const char *src);
int			ft_strcmp(const char *s1, const char *s2);
void		*ft_memcpy(void *dst, const void *src, size_t n);
void		*ft_memset(void *b, int c, size_t len);

#endif
