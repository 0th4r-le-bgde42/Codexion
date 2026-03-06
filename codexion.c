/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 09:42:45 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/06 08:14:58 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	parsing(int ac, char **av, t_config *config)
{
	if (ac != 9)
		return (printf("Error: Wrong number of arguments\n"), 1);
	init_arg(av, config);
	if (config->num_coders <= 0 || config->time_to_burnout < 0
		|| config->time_to_compile < 0 || config->time_to_debug < 0
		|| config->time_to_refactor < 0 || config->required_compiles < 0
		|| config->dongle_cooldown < 0)
		return (printf("Error: Invalid numeric values\n"), 1);
	if (strcmp(av[8], "fifo") == 0)
		config->scheduler_type = FIFO;
	else if (strcmp(av[8], "edf") == 0)
		config->scheduler_type = EDF;
	else
		return (printf("Error: Scheduler must be 'fifo' or 'edf'\n"), 1);
	return (0);
}

static int	launch_sim(t_data *data, pthread_t *id)
{
	int	i;

	i = 0;
	while (i < data->config.num_coders)
	{
		pthread_mutex_lock(&data->config.write_mutex);
		data->coder[i].last_compile_start = get_time_ms();
		pthread_mutex_unlock(&data->config.write_mutex);
		if (pthread_create(&data->coder[i].thread_id, NULL,
				&coder_routine, &data->coder[i]))
			return (1);
		i++;
	}
	if (pthread_create(id, NULL, &monitor_routine, data))
		return (1);
	return (0);
}

static void	cleanup_sim(t_data *data, pthread_t monitor_id)
{
	int	i;
	int	reason;

	pthread_join(monitor_id, NULL);
	i = -1;
	while (++i < data->config.num_coders)
		pthread_join(data->coder[i].thread_id, NULL);
	pthread_mutex_lock(&data->config.stop_mutex);
	reason = data->config.stop_reason;
	pthread_mutex_unlock(&data->config.stop_mutex);
	if (reason == 1)
		printf("%sSimulation Stopped (Detected Burnout)%s\n", RED, RESET);
	else
		printf("%sSimulation Finished (All Job Done)%s\n", GREEN, RESET);
}

int	main(int ac, char **av)
{
	t_data		data;
	pthread_t	id;

	if (parsing(ac, av, &data.config))
		return (1);
	data.coder = NULL;
	data.dongles = NULL;
	pthread_mutex_init(&data.config.write_mutex, NULL);
	pthread_mutex_init(&data.config.stop_mutex, NULL);
	data.config.simulation_stop = 0;
	if (init_dongles(&data) || init_coders(&data))
		return (free_all(&data), 1);
	printf("%s=== Starting Simulation ===%s\n", CYAN, RESET);
	data.config.start_time = get_time_ms();
	if (!launch_sim(&data, &id))
		cleanup_sim(&data, id);
	return (free_all(&data), 0);
}
