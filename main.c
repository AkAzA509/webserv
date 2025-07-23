/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:16:00 by ggirault          #+#    #+#             */
/*   Updated: 2025/06/10 11:23:14 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/miniRT.h"
#include <stdio.h>

void	clear_objects(t_scene *scene)
{
	t_objs	*obj;
	t_objs	*next;

	obj = scene->objs;
	while (obj)
	{
		next = obj->next;
		free(obj);
		obj = next;
	}
	scene->objs = NULL;
}

void	clear_lights(t_scene *scene)
{
	t_light	*light;
	t_light	*next;

	light = scene->lights;
	while (light)
	{
		next = light->next;
		free(light);
		light = next;
	}
	scene->lights = NULL;
}

void	clear_scene(t_scene *scene)
{
	if (!scene)
		return ;
	mlx_delete_image(scene->mlx, scene->img[0]);
	mlx_delete_image(scene->mlx, scene->img[1]);
	clear_objects(scene);
	clear_lights(scene);
}

void	close_func(mlx_key_data_t keydata, void *param)
{
	t_scene	*scene;

	scene = (t_scene *)param;
	if (keydata.key == MLX_KEY_ESCAPE && keydata.action == MLX_PRESS)
	{
		clear_scene(scene);
		mlx_close_window(scene->mlx);
	}
}

int	main(int argc, char **argv)
{
	t_scene	scene;

	scene.objs = NULL;
	scene.lights = NULL;
	if (argc != 2)
		return (printf("Usage: ./miniRT <scene.rt>"), 1);
	scene.num_cam = 0;
	scene.num_ambiants = 0;
	if (!parse_rt(argv[1], &scene))
		return (1);
	scene.mlx = mlx_init(WIDTH, HEIGHT, "miniRT", false);
	if (!scene.mlx)
		return (1);
	scene.img[0] = mlx_new_image(scene.mlx, WIDTH, HEIGHT);
	scene.img[1] = mlx_new_image(scene.mlx, WIDTH, HEIGHT);
	scene.current_img = 0;
	render(scene.img[scene.current_img], &scene);
	scene.current_img = 1 - scene.current_img;
	mlx_key_hook(scene.mlx, close_func, &scene);
	mlx_loop_hook(scene.mlx, move, &scene);
	mlx_loop(scene.mlx);
	clear_scene(&scene);
	mlx_terminate(scene.mlx);
	return (0);
}
