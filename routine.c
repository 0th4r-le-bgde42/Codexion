/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 13:41:16 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 13:41:51 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (1)
	{
		pthread_mutex_lock(&coder->config->stop_mutex);
		if (coder->config->simulation_stop)
		{
			pthread_mutex_unlock(&coder->config->stop_mutex);
			break ;
		}
		pthread_mutex_unlock(&coder->config->stop_mutex);
		manage_dongle_in(coder);
		print_log(coder, "is compiling");
		pthread_mutex_lock(&coder->config->write_mutex);
		coder->last_compile_start = get_time_ms();
		pthread_mutex_unlock(&coder->config->write_mutex);
		smart_sleep(coder->config->time_to_compile, coder->config);
		pthread_mutex_lock(&coder->config->write_mutex);
		coder->compiles_done++;
		pthread_mutex_unlock(&coder->config->write_mutex);
		manage_dongle_out(coder);
		print_log(coder, "is debugging");
		smart_sleep(coder->config->time_to_debug, coder->config);
		print_log(coder, "is refactoring");
		smart_sleep(coder->config->time_to_refactor, coder->config);
	}
	return (NULL);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		task_finished;
	int		i;
	int		done;
	long	last;
	

	data = (t_data *)arg;
	while (1)
	{
		i = 0;
		task_finished = 1;
		while (i < data->config.num_coders)
		{
			pthread_mutex_lock(&data->config.write_mutex);
			last = data->coder[i].last_compile_start;
			done = data->coder[i].compiles_done;
			pthread_mutex_unlock(&data->config.write_mutex);
			if (get_time_ms() - last >= data->config.time_to_burnout)
				return (wake_up_call(data),
					print_log(&data->coder[i], "burned out"),
					NULL);
			if (done < data->config.required_compiles)
				task_finished = 0;
			i++;
		}
		if (task_finished)
			return (wake_up_call(data), NULL);
		usleep(1000);
	}
	return (NULL);
}
