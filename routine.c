/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 13:41:16 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 09:01:41 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void *coder_routine(void *arg)
{
	t_coder *coder = (t_coder *)arg;
	while (1)
	{
		pthread_mutex_lock(&coder->config->stop_mutex);
		if (coder->config->simulation_stop)
		{
			pthread_mutex_unlock(&coder->config->stop_mutex);
			break;
		}
		pthread_mutex_unlock(&coder->config->stop_mutex);
		manage_dongle_in(coder);
		print_log(coder, "is compiling");
		coder->last_compile_start = get_time_ms();
		smart_sleep(coder->config->time_to_compile, coder->config);
		coder->compiles_done++;
		manage_dongle_out(coder);
		print_log(coder, "is debugging");
		smart_sleep(coder->config->time_to_debug, coder->config);
		print_log(coder, "is refactoring");
		smart_sleep(coder->config->time_to_refactor, coder->config);
	}
	return (NULL);
}

void *monitor_routine(void *arg)
{
	t_data *data = (t_data *)arg;
	int task_finished;
	int i;
	int j;

	while (1)
	{
		i = 0;
		task_finished = 1;
		while (i < data->config.num_coders)
		{
			long now = get_time_ms();
			if (now - data->coder[i].last_compile_start >= data->config.time_to_burnout)
			{
				pthread_mutex_lock(&data->config.stop_mutex);
				data->config.simulation_stop = 1;
				j = 0;
				while (j < data->config.num_coders)
				{
					pthread_mutex_lock(&data->dongles[j].mutex);
					pthread_cond_broadcast(&data->dongles[j].cond);
					pthread_mutex_unlock(&data->dongles[j].mutex);
					j++;
				}
				pthread_mutex_unlock(&data->config.stop_mutex);
				print_log(&data->coder[i], "burned out");
				return (NULL);
			}
			if (data->coder[i].compiles_done < data->config.required_compiles)
				task_finished = 0;
			i++;
		}
		if (task_finished)
		{
			pthread_mutex_lock(&data->config.stop_mutex);
			data->config.simulation_stop = 1;
			pthread_mutex_unlock(&data->config.stop_mutex);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
