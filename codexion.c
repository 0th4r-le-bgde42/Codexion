/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 09:42:45 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 08:54:51 by ldauber          ###   ########.fr       */
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

#include "codexion.h"

int main(int ac, char **av)
{
	t_data    data;
	pthread_t monitor_id;
	int       i;

	if (ac != 9)
	{
		printf("Error: Wrong number of arguments\n");
		return (1);
	}
	if (parsing(av, &data.config) != 0)
		return (1);

	// 1. Initialisation globale
	data.config.simulation_stop = 0;
	pthread_mutex_init(&data.config.write_mutex, NULL);
	pthread_mutex_init(&data.config.stop_mutex, NULL);
	
	if (init_dongles(&data) != 0 || init_coders(&data) != 0)
		return (1); // Idéalement, appelle une fonction free_all ici

	// 2. Point de départ temporel
	data.config.start_time = get_time_ms();

	// 3. Lancement des threads Codeurs
	i = 0;
	while (i < data.config.num_coders)
	{
		// On synchronise le temps de survie initial sur le début de la simulation
		data.coder[i].last_compile_start = get_time_ms();
		pthread_create(&data.coder[i].thread_id, NULL, &coder_routine, &data.coder[i]);
		i++;
	}

	// 4. Lancement du thread Moniteur
	if (pthread_create(&monitor_id, NULL, &monitor_routine, &data) != 0)
		return (1);

	// 5. Attente de la fin (Le moniteur finit quand quelqu'un meurt ou a fini)
	pthread_join(monitor_id, NULL);

	// 6. Nettoyage des threads Codeurs
	i = 0;
	while (i < data.config.num_coders)
	{
		pthread_join(data.coder[i].thread_id, NULL);
		i++;
	}

	// 7. Libération des ressources (Appelle ton futur free_all)
	// free_all(&data);
	
	char *scheduler;
	if (data.config.scheduler_type == 1)
		scheduler = "edf";
	else
		scheduler = "fifo";
	printf("Coders: %d\n", data.config.num_coders);
	printf("Time to burnout: %d\n", data.config.time_to_burnout);
	printf("Time to compile: %d\n", data.config.time_to_compile);
	printf("Time to debug: %d\n", data.config.time_to_debug);
	printf("Time to refactor : %d\n", data.config.time_to_refactor);
	printf("Compilation required: %d\n", data.config.required_compiles);
	printf("Dongles cooldown: %d\n", data.config.dongle_cooldown);
	printf("Scheduler: %s\n", scheduler);
	
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
