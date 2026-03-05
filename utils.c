/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 14:47:25 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 07:50:08 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long get_time_ms(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1)
		return (0);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void print_log(t_coder *coder, char *message)
{
	long now = get_time_ms();
	long start = coder->config->start_time;
	pthread_mutex_lock(&coder->config->write_mutex);
	pthread_mutex_lock(&coder->config->stop_mutex);
	if (!coder->config->simulation_stop || strcmp(message, "burned out") == 0)
		printf("%ld %d %s\n", now - start, coder->id, message);

	pthread_mutex_unlock(&coder->config->stop_mutex);
	pthread_mutex_unlock(&coder->config->write_mutex);
}

void smart_sleep(long time, t_config *config)
{
	long start = get_time_ms();
	
	while (get_time_ms() - start < time)
	{
		pthread_mutex_lock(&config->stop_mutex);
		if (config->simulation_stop)
		{
			pthread_mutex_unlock(&config->stop_mutex);
			break;
		}
		pthread_mutex_unlock(&config->stop_mutex);
		usleep(500);
	}
}
