/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 14:47:25 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/06 07:57:46 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) == -1)
		return (0);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	print_log(t_coder *coder, char *message)
{
	long	now;
	long	start;
	char	*color;

	now = get_time_ms();
	start = coder->config->start_time;
	color = RESET;
	if (strcmp(message, "has taken a dongle") == 0)
		color = BLUE;
	else if (strcmp(message, "is compiling") == 0)
		color = GREEN;
	else if (strcmp(message, "is debugging") == 0)
		color = MAGENTA;
	else if (strcmp(message, "is refactoring") == 0)
		color = YELLOW;
	else if (strcmp(message, "burned out") == 0)
		color = RED;
	pthread_mutex_lock(&coder->config->stop_mutex);
	pthread_mutex_lock(&coder->config->write_mutex);
	if (!coder->config->simulation_stop || strcmp(message, "burned out") == 0)
		printf("%ld %d %s%s%s\n", now - start, coder->id,
			color, message, RESET);
	pthread_mutex_unlock(&coder->config->stop_mutex);
	pthread_mutex_unlock(&coder->config->write_mutex);
}

void	smart_sleep(long time, t_config *config)
{
	long	start;

	start = get_time_ms();
	while (get_time_ms() - start < time)
	{
		pthread_mutex_lock(&config->stop_mutex);
		if (config->simulation_stop)
		{
			pthread_mutex_unlock(&config->stop_mutex);
			break ;
		}
		pthread_mutex_unlock(&config->stop_mutex);
		usleep(500);
	}
}

void	wake_up_call(t_data *data, int reason)
{
	int	j;

	pthread_mutex_lock(&data->config.stop_mutex);
	data->config.simulation_stop = 1;
	data->config.stop_reason = reason;
	pthread_mutex_unlock(&data->config.stop_mutex);
	j = 0;
	while (j < data->config.num_coders)
	{
		pthread_mutex_lock(&data->dongles[j].mutex);
		pthread_cond_broadcast(&data->dongles[j].cond);
		pthread_mutex_unlock(&data->dongles[j].mutex);
		j++;
	}
}

void	free_all(t_data *data)
{
	int	i;

	if (!data)
		return ;
	if (data->dongles)
	{
		i = 0;
		while (i < data->config.num_coders)
		{
			pthread_mutex_destroy(&data->dongles[i].mutex);
			pthread_cond_destroy(&data->dongles[i]. cond);
			if (data->dongles[i].request_queue.array)
				free(data->dongles[i].request_queue.array);
			i++;
		}
		free(data->dongles);
		data->dongles = NULL;
	}
	if (data->coder)
	{
		free(data->coder);
		data->coder = NULL;
	}
	pthread_mutex_destroy(&data->config.write_mutex);
	pthread_mutex_destroy(&data->config.stop_mutex);
}
