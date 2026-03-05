/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 08:17:57 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/05 14:52:42 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

# define FIFO 0
# define EDF 1 
# define RESET   "\033[0m"
# define RED     "\033[1;31m"
# define GREEN   "\033[1;32m"
# define YELLOW  "\033[1;33m"
# define BLUE    "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN    "\033[1;36m"
# define WHITE   "\033[1;37m"

typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_config	t_config;

typedef struct s_config
{
	int				num_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				required_compiles;
	int				dongle_cooldown;
	int				scheduler_type;
	int				simulation_stop;
	long			start_time;
	pthread_mutex_t	write_mutex;
	pthread_mutex_t	stop_mutex;
}	t_config;

typedef struct s_heap
{
	t_coder	**array;
	int		size;
	int		capacity;
}	t_heap;

struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				is_taken;
	long			last_release_time;
	t_heap			request_queue;
};

struct s_coder
{
	pthread_t	thread_id;
	int			id;
	long		last_compile_start;
	int			compiles_done;
	long		request_time;
	t_config	*config;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
};

typedef struct s_data
{
	t_config	config;
	t_coder		*coder;
	t_dongle	*dongles;
}	t_data;

// HEAP
int		compare_coders(t_coder *a, t_coder *b, t_config *config);
void	heap_push(t_heap *heap, t_coder *coder, t_config *config);
void	heap_pop(t_heap *heap, t_config *config);
t_coder	*heap_peek(t_heap *heap);

// ROUTINE
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

// DONGLES
void	take_dongle(t_dongle *dongle, t_coder *coder);
void	drop_dongle(t_dongle *dongle);
void	manage_dongle_in(t_coder *coder);
void	manage_dongle_out(t_coder *coder);

// INIT
void	init_arg(char **av, t_config *config);
int		init_dongles(t_data *data);
int		init_coders(t_data *data);

// UTILS
long	get_time_ms(void);
void	print_log(t_coder *coder, char *message);
void	smart_sleep(long time, t_config *config);
void	wake_up_call(t_data *data);
void	free_all(t_data *data);

#endif