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

# define NB_OPTIONS			2

typedef struct		s_options
{
	int				h;
	int				m;
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
	int					ttl_val;
	t_options			options;
}						t_traceroute;

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

/* send.c */
t_udp_packet	*send_packet(t_traceroute *traceroute, int dstport, int ttl);

/* recv.c */
t_icmp_packet	*recv_packet(t_traceroute *traceroute, struct timeval start_recv, t_udp_packet **udp_packets, int nb_probes);

/* print.c */
int		print_packets(t_udp_packet **udp_packets, t_icmp_packet **icmp_packets);

/* time.c */
double			get_diff_ms(struct timeval *start, struct timeval *end);
struct timeval	get_diff_timeval(float start, float end);

/* args.c */
int			check_args(int argc, char *argv[], t_traceroute *traceroute);

/* error.c */
void		show_help(void);
int			getaddrinfo_error(char *prg_name, int error, char *str);
int			args_error(char *prg_name, int error, char *str, int range1, int range2);
int			is_num(const char *str);
int			ft_atoi(const char *str);

/* utils.c */
size_t		ft_strlen(const char *s);
char		*ft_strcpy(char *dst, const char *src);
int			ft_strcmp(const char *s1, const char *s2);
void		*ft_memcpy(void *dst, const void *src, size_t n);
void		*ft_memset(void *b, int c, size_t len);

#endif
