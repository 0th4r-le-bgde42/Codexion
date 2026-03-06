/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 08:46:12 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/06 13:55:21 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	compare_coders(t_coder *a, t_coder *b, t_config *config)
{
	long	deadline_a;
	long	deadline_b;

	if (config->scheduler_type == FIFO)
		return (a->request_time < b->request_time);
	else
	{
		deadline_a = a->last_compile_start + config->time_to_burnout;
		deadline_b = b->last_compile_start + config->time_to_burnout;
		if (deadline_a == deadline_b)
			return (a->request_time < b->request_time);
		return (deadline_a < deadline_b);
	}
}

void	heap_push(t_heap *heap, t_coder *coder, t_config *config)
{
	int		i;
	int		parent;
	t_coder	*tmp;

	i = heap->size;
	heap->array[i] = coder;
	heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (compare_coders(heap->array[i], heap->array[parent], config))
		{
			tmp = heap->array[i];
			heap->array[i] = heap->array[parent];
			heap->array[parent] = tmp;
			i = parent;
		}
		else
			break ;
	}
}

static void	sift_down(t_heap *heap, int i, t_config *config)
{
	int		smallest;
	int		left;
	int		right;
	t_coder	*tmp;

	while (1)
	{
		smallest = i;
		left = 2 * i + 1;
		right = 2 * i + 2;
		if (left < heap->size && compare_coders(heap->array[left],
				heap->array[smallest], config))
			smallest = left;
		if (right < heap->size && compare_coders(heap->array[right],
				heap->array[smallest], config))
			smallest = right;
		if (smallest == i)
			break ;
		tmp = heap->array[i];
		heap->array[i] = heap->array[smallest];
		heap->array[smallest] = tmp;
		i = smallest;
	}
}

void	heap_pop(t_heap *heap, t_config *config)
{
	if (heap->size == 0)
		return ;
	heap->array[0] = heap->array[heap->size - 1];
	heap->size--;
	if (heap->size > 0)
		sift_down(heap, 0, config);
}

t_coder	*heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (heap->array[0]);
}
