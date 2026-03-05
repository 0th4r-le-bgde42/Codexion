/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 07:49:26 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 08:22:53 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

void manage_dongle_in(t_coder *coder)
{
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
}

void manage_dongle_out(t_coder *coder)
{
	drop_dongle(coder->left_dongle);
	drop_dongle(coder->right_dongle);
}