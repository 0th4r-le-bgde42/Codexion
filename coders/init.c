/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 12:55:53 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 14:43:39 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	init_arg(char **av, t_config *config)
{
	config->num_coders = atoi(av[1]);
	config->time_to_burnout = atoi(av[2]);
	config->time_to_compile = atoi(av[3]);
	config->time_to_debug = atoi(av[4]);
	config->time_to_refactor = atoi(av[5]);
	config->required_compiles = atoi(av[6]);
	config->dongle_cooldown = atoi(av[7]);
}

int	init_dongles(t_data *data)
{
	int	i;
	int	mumu;

	data->dongles = malloc(sizeof(t_dongle) * data->config.num_coders);
	if (!data->dongles)
		return (1);
	i = 0;
	while (i < data->config.num_coders)
	{
		mumu = pthread_mutex_init(&data->dongles[i].mutex, NULL);
		if (mumu != 0)
			return (free_all(data), 1);
		pthread_cond_init(&data->dongles[i].cond, NULL);
		data->dongles[i].last_release_time = 0;
		data->dongles[i].request_queue.array = malloc(sizeof(t_coder *)
				* data->config.num_coders);
		data->dongles[i].request_queue.size = 0;
		data->dongles[i].request_queue.capacity = data->config.num_coders;
		data->dongles[i].is_taken = 0;
		i++;
	}
	return (0);
}

int	init_coders(t_data *data)
{
	int	i;

	data->coder = malloc(sizeof(t_coder) * data->config.num_coders);
	if (!data->coder)
		return (1);
	i = 0;
	while (i < data->config.num_coders)
	{
		data->coder[i].id = i + 1;
		data->coder[i].last_compile_start = 0;
		data->coder[i].compiles_done = 0;
		data->coder[i].config = &data->config;
		data->coder[i].left_dongle = &data->dongles[i];
		data->coder[i].right_dongle = &data->dongles[(i + 1)
			% data->config.num_coders];
		i++;
	}
	return (0);
}
