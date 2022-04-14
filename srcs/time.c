#include "ft_traceroute.h"

double		get_diff_ms(struct timeval *start, struct timeval *end)
{
	return ((double)((end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec) / 1000);
}

long		ft_floor(double f)
{
	if(f >= 0.0) {
		return ((int)f);
	} else {
		return ((int)f - 1);
	}
}

struct timeval	get_diff_timeval(float start, float end)
{
	struct timeval time;

	time.tv_sec = ft_floor(start - end);
	time.tv_usec = ((float)time.tv_sec - (start - end) * 1000000);
	return (time);
}
