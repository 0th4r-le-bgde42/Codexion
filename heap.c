/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldauber <ldauber@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 08:46:12 by ldauber           #+#    #+#             */
/*   Updated: 2026/03/04 12:43:01 by ldauber          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int compare_coders(t_coder *a, t_coder *b, t_config *config)
{
	if (config->scheduler_type == FIFO)
		return (a->id < b->id);
	else
	{
		long deadline_a = a->last_compile_start + config->time_to_burnout;
		long deadline_b = b->last_compile_start + config->time_to_burnout;
		return (deadline_a < deadline_b);
	}
}

void heap_push(t_heap *heap, t_coder *coder, t_config *config)
{
	int i = heap->size;
	heap->array[i] = coder;
	heap->size++;

	while (i > 0)
	{
		int parent = (i - 1) / 2;
		if (compare_coders(heap->array[i], heap->array[parent], config))
		{
			t_coder *tmp = heap->array[i];
			heap->array[i] = heap->array[parent];
			heap->array[parent] = tmp;
			i = parent;
		}
		else
			break;
	}
}

void heap_pop(t_heap *heap, t_config *config)
{
	if (heap->size == 0)
		return;
	
	heap->array[0] = heap->array[heap->size - 1];
	heap->size--;

	int i = 0;
	while (1)
	{
		int smallest = i;
		int left = 2 * i + 1;
		int right = 2 * i + 2;
		
		if (left < heap->size && compare_coders(heap->array[left], heap->array[smallest], config))
			smallest = left;
		if (right < heap->size && compare_coders(heap->array[right], heap->array[smallest], config))
			smallest = right;
		
		if (smallest != i)
		{
			t_coder *tmp = heap->array[i];
			heap->array[i] = heap->array[smallest];
			heap->array[smallest] = tmp;
			i = smallest;
		}
		else
			break;
	}
}

t_coder *heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (heap->array[0]);
}