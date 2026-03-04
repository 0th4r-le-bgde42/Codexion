/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 09:42:45 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/04 15:13:48 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int parsing(char **av, t_config *config)
{
	init_arg(av, config);
	if (config->num_coders <= 0 || config->time_to_burnout < 0 ||
		config->time_to_compile < 0 || config->time_to_debug < 0 ||
		config->time_to_refactor < 0 || config->required_compiles < 0 ||
		config->dongle_cooldown < 0)
	{
		printf("Error: Invalid numeric values\n");
		return (1);
	}

	if (strcmp(av[8], "fifo") == 0)
		config->scheduler_type = FIFO;
	else if (strcmp(av[8], "edf") == 0)
		config->scheduler_type = EDF;
	else
	{
		printf("Error: Scheduler must be 'fifo' or 'edf'\n");
		return (1);
	}
	return (0);
}

int main(int ac, char **av)
{
	t_data data;
	int i;

	if (ac != 9 || parsing(av, &data.config) != 0)
		return (1);

	data.config.simulation_stop = 0;
	// 1. Initialisation des ressources
	pthread_mutex_init(&data.config.write_mutex, NULL);
	pthread_mutex_init(&data.config.stop_mutex, NULL);
	init_dongles(&data);
	init_coders(&data);
	
	data.config.start_time = get_time_ms();
	printf("--- Démarrage de la simulation (5s de test) ---\n");

	// 2. Lancement des threads codeurs
	i = 0;
	while (i < data.config.num_coders)
	{
		data.coder[i].last_compile_start = data.config.start_time;
		pthread_create(&data.coder[i].thread_id, NULL, &coder_routine, &data.coder[i]);
		i++;
	}

	// 3. On laisse tourner 5 secondes
	usleep(1000 * 1000);

	// 4. On force l'arrêt
	pthread_mutex_lock(&data.config.stop_mutex);
	data.config.simulation_stop = 1;
	pthread_mutex_unlock(&data.config.stop_mutex);
	printf("--- Fin de simulation ---\n");

	// 5. Attente des threads
	i = 0;
	while (i < data.config.num_coders)
	{
		pthread_join(data.coder[i].thread_id, NULL);
		i++;
	}

	return (0);
}
/* int main(int ac, char **av)
{
	t_config config;
	if (ac != 9)
	{
		printf("Error: Wrong number of arguments\n");
		return (1);
	}
	if (parsing(av, &config) != 0)
		return (1);
	char *scheduler;
	if (config.scheduler_type == 1)
		scheduler = "edf";
	else
		scheduler = "fifo";
		
	printf("Coders: %d\n", config.num_coders);
	printf("Time to burnout: %d\n", config.time_to_burnout);
	printf("Time to compile: %d\n", config.time_to_compile);
	printf("Time to debug: %d\n", config.time_to_debug);
	printf("Time to refactor : %d\n", config.time_to_refactor);
	printf("Compilation required: %d\n", config.required_compiles);
	printf("Dongles cooldown: %d\n", config.dongle_cooldown);
	printf("Scheduler: %s\n", scheduler);

	return (0);
} */
