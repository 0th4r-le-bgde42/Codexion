/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 07:49:26 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 14:45:22 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_stop(t_coder *coder)
{
	int	stop;

	pthread_mutex_lock(&coder->config->stop_mutex);
	stop = coder->config->simulation_stop;
	pthread_mutex_unlock(&coder->config->stop_mutex);
	return (stop);
}
void	take_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	coder->request_time = get_time_ms();
	heap_push(&dongle->request_queue, coder, coder->config);
	while (!check_stop(coder))
	{
		if (heap_peek(&dongle->request_queue) == coder && !dongle->is_taken)
		{
			if (get_time_ms() >= dongle->last_release_time
				+ coder->config->dongle_cooldown)
				break ;
			pthread_mutex_unlock(&dongle->mutex);
			usleep(500);
			pthread_mutex_lock(&dongle->mutex);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_pop(&dongle->request_queue, coder->config);
	if (!coder->config->simulation_stop)
		dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->mutex);
	if (!coder->config->simulation_stop)
		print_log(coder, "has taken a dongle");
}

void	drop_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_taken = 0;
	dongle->last_release_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	manage_dongle_in(t_coder *coder)
{
	if (coder->id % 2 == 0)
	{
		take_dongle(coder->right_dongle, coder);
		take_dongle(coder->left_dongle, coder);
	}
	else
	{
		take_dongle(coder->left_dongle, coder);
		take_dongle(coder->right_dongle, coder);
	}
}

void	manage_dongle_out(t_coder *coder)
{
	drop_dongle(coder->left_dongle);
	drop_dongle(coder->right_dongle);
}
