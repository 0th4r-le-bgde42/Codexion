/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 13:41:16 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/06 07:53:54 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	compilation_step(t_coder *coder)
{
	manage_dongle_in(coder);
	print_log(coder, "is compiling");
	pthread_mutex_lock(&coder->config->write_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->config->write_mutex);
	smart_sleep(coder->config->time_to_compile, coder->config);
	manage_dongle_out(coder);
	pthread_mutex_lock(&coder->config->write_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->config->write_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		stop;

	coder = (t_coder *)arg;
	while (1)
	{
		pthread_mutex_lock(&coder->config->stop_mutex);
		stop = coder->config->simulation_stop;
		pthread_mutex_unlock(&coder->config->stop_mutex);
		if (stop)
			break ;
		compilation_step(coder);
		print_log(coder, "is debugging");
		smart_sleep(coder->config->time_to_debug, coder->config);
		print_log(coder, "is refactoring");
		smart_sleep(coder->config->time_to_refactor, coder->config);
	}
	return (NULL);
}

static int	check_death(t_data *data, int i, int *task_finished)
{
	long	last;

	pthread_mutex_lock(&data->config.write_mutex);
	last = data->coder[i].last_compile_start;
	pthread_mutex_unlock(&data->config.write_mutex);
	if (get_time_ms() - last >= data->config.time_to_burnout)
	{
		wake_up_call(data, 1);
		print_log(&data->coder[i], "burned out");
		return (1);
	}
	(void)task_finished;
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		task_finished;
	int		i;

	data = (t_data *)arg;
	while (1)
	{
		i = -1;
		task_finished = 1;
		while (++i < data->config.num_coders)
		{
			pthread_mutex_lock(&data->config.write_mutex);
			if (data->coder[i].compiles_done < data->config.required_compiles)
				task_finished = 0;
			pthread_mutex_unlock(&data->config.write_mutex);
		}
		if (task_finished)
			return (wake_up_call(data, 0), NULL);
		i = -1;
		while (++i < data->config.num_coders)
			if (check_death(data, i, &task_finished))
				return (NULL);
		usleep(1000);
	}
	return (NULL);
}
