/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 13:41:16 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/04 15:04:22 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

void take_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->request_queue, coder, coder->config);
	while (heap_peek(&dongle->request_queue) != coder ||
			get_time_ms() < dongle->last_release_time + coder->config->dongle_cooldown)
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	heap_pop(&dongle->request_queue, coder->config);
	print_log(coder, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
}

void drop_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->last_release_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void *coder_routine(void *arg)
{
	t_coder * coder = (t_coder *)arg;
	while (1)
	{
		pthread_mutex_lock(&coder->config->stop_mutex);
		if (coder->config->simulation_stop)
		{
			pthread_mutex_unlock(&coder->config->stop_mutex);
			break;
		}
		pthread_mutex_unlock(&coder->config->stop_mutex);
		if (coder->id % 2 == 0)
		{
			take_dongle(coder->left_dongle, coder);
			take_dongle(coder->right_dongle, coder);
		}
		else
		{
			take_dongle(coder->right_dongle, coder);
			take_dongle(coder->left_dongle, coder);
		}
		print_log(coder, "is compiling");
		coder->last_compile_start = get_time_ms();
		smart_sleep(coder->config->time_to_compile, coder->config);
		coder->compiles_done++;
		drop_dongle(coder->left_dongle);
		drop_dongle(coder->right_dongle);

		print_log(coder, "is debugging");
		smart_sleep(coder->config->time_to_debug, coder->config);
		print_log(coder, "is refactoring");
		smart_sleep(coder->config->time_to_refactor, coder->config);
	}
	return (NULL);
}
