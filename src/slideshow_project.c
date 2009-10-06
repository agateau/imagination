/*
 *  Copyright (C) 2009 Giuseppe Torelli <colossus73@gmail.com>
 *  Copyright (c) 2009 Tadej Borovšak 	<tadeboro@gmail.com>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include "slideshow_project.h"

static gboolean img_populate_hash_table( GtkTreeModel *, GtkTreePath *, GtkTreeIter *, GHashTable ** );

void
img_save_slideshow( img_window_struct *img,
					const gchar       *output )
{
	GKeyFile *img_key_file;
	gchar *conf, *string, *path, *filename, *file, *font_desc;
	gint count = 0;
	gsize len;
	GtkTreeIter iter;
	slide_struct *entry;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL( img->thumbnail_model );
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;

	img_key_file = g_key_file_new();

	/* Slideshow settings */
	g_key_file_set_comment(img_key_file, NULL, NULL, comment_string, NULL);

	g_key_file_set_integer( img_key_file, "slideshow settings",
							"video format", img->video_size[1] );
	g_key_file_set_double_list( img_key_file, "slideshow settings",
								"background color", img->background_color, 3 );
	g_key_file_set_boolean(img_key_file,"slideshow settings", "distort images", img->distort_images);

	g_key_file_set_integer(img_key_file, "slideshow settings", "number of slides", img->slides_nr);

	/* Slide settings */
	do
	{
		count++;
		gtk_tree_model_get(model, &iter,1,&entry,-1);
		conf = g_strdup_printf("slide %d",count);

		if( entry->o_filename)
		{
			/* Save original filename and rotation */
			g_key_file_set_string( img_key_file, conf,
								   "filename", entry->o_filename);
			g_key_file_set_integer( img_key_file, conf, "angle", entry->angle );
		}
		else
		{
			/* We are dealing with an empty slide */
			gdouble *start_color = entry->g_start_color,
					*stop_color  = entry->g_stop_color,
					*start_point = entry->g_start_point,
					*stop_point  = entry->g_stop_point;

			g_key_file_set_integer(img_key_file, conf, "gradient",	entry->gradient);
			g_key_file_set_double_list(img_key_file, conf, "start_color", start_color, 3 );
			g_key_file_set_double_list(img_key_file, conf, "stop_color" , stop_color , 3 );
			g_key_file_set_double_list(img_key_file, conf, "start_point", start_point, 2 );
			g_key_file_set_double_list(img_key_file, conf, "stop_point" , stop_point , 2 );
		}

		/* Duration */
		g_key_file_set_integer(img_key_file,conf, "duration",		entry->duration);

		/* Transition */
		g_key_file_set_integer(img_key_file,conf, "transition_id",	entry->transition_id);
		g_key_file_set_integer(img_key_file,conf, "speed", 			entry->speed);

		/* Stop points */
		g_key_file_set_integer(img_key_file,conf, "no_points",		entry->no_points);
		if (entry->no_points > 0)
		{
			gint    point_counter;
			gdouble my_points[entry->no_points * 4];

			for( point_counter = 0;
				 point_counter < entry->no_points;
				 point_counter++ )
			{
				ImgStopPoint *my_point = g_list_nth_data(entry->points,point_counter);
				my_points[ (point_counter * 4) + 0] = (gdouble)my_point->time;
				my_points[ (point_counter * 4) + 1] = my_point->offx;
				my_points[ (point_counter * 4) + 2] = my_point->offy;
				my_points[ (point_counter * 4) + 3] = my_point->zoom;
			}
			g_key_file_set_double_list(img_key_file,conf, "points", my_points, (gsize) entry->no_points * 4);
		}

		/* Subtitle */
		/* EXPLANATION: Only facultative field here is subtitle text, since user
		 * may have set other text properties on slide and we would let them out
		 * if we only save slides with text present. */
		if( entry->subtitle )
			g_key_file_set_string (img_key_file, conf,"text",			entry->subtitle);

		font_desc = pango_font_description_to_string(entry->font_desc);
		g_key_file_set_integer(img_key_file,conf, "anim id",		entry->anim_id);
		g_key_file_set_integer(img_key_file,conf, "anim duration",	entry->anim_duration);
		g_key_file_set_integer(img_key_file,conf, "text pos",		entry->position);
		g_key_file_set_integer(img_key_file,conf, "placing",		entry->placing);
		g_key_file_set_string (img_key_file, conf,"font",			font_desc);
		g_key_file_set_double_list(img_key_file, conf,"font color",entry->font_color,4);
		g_free(font_desc);
		g_free(conf);
	}
	while (gtk_tree_model_iter_next (model,&iter));
	count = 0;

	/* Background music */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(img->music_file_treeview));
	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		g_key_file_set_integer(img_key_file, "music", "number", gtk_tree_model_iter_n_children(model, NULL));
		do
		{
			count++;
			gtk_tree_model_get(model, &iter, 0, &path, 1, &filename ,-1);
			conf = g_strdup_printf("music_%d",count);
			file = g_build_filename(path, filename, NULL);
			g_free(path);
			g_free(filename);
			g_key_file_set_string(img_key_file, "music", conf, file);
			g_free(file);
			g_free(conf);
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	/* Write the project file */
	conf = g_key_file_to_data(img_key_file, &len, NULL);
	g_file_set_contents( output, conf, len, NULL );
	g_free (conf);

	string = g_path_get_basename( output );
	img_set_window_title(img,string);
	g_free(string);
	g_key_file_free(img_key_file);

	if( img->project_filename )
		g_free( img->project_filename );
	img->project_filename = g_strdup( output );

	img->project_is_modified = FALSE;
}

void
img_load_slideshow( img_window_struct *img,
					const gchar       *input )
{
	GdkPixbuf *thumb;
	slide_struct *slide_info;
	GtkTreeIter iter;
	GKeyFile *img_key_file;
	gchar *dummy, *slide_filename, *time;
	GtkWidget *dialog;
	gint number,i,transition_id, duration, no_points, previous_nr_of_slides;
	guint speed;
	GtkTreeModel *model;
	void (*render);
	GHashTable *table;
	gchar      *spath, *conf;
	gdouble    *color, *font_color;
	gboolean    old_file = FALSE;
	gboolean    first_slide = TRUE;

	/* Cretate new key file */
	img_key_file = g_key_file_new();
	if( ! g_key_file_load_from_file( img_key_file, input,
									 G_KEY_FILE_KEEP_COMMENTS, NULL ) )
	{
		g_key_file_free( img_key_file );
		return;
	}

	/* Are we able to load this project? */
	dummy = g_key_file_get_comment( img_key_file, NULL, NULL, NULL);

	if( strncmp( dummy, comment_string, strlen( comment_string ) ) != 0 )
	{
		/* Enable loading of old projects too */
		if( strncmp( dummy, old_comment_string,
					 strlen( old_comment_string ) ) != 0 )
		{
			dialog = gtk_message_dialog_new(
						GTK_WINDOW( img->imagination_window ), GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
						_("This is not an Imagination project file!") );
			gtk_window_set_title( GTK_WINDOW( dialog ), "Imagination" );
			gtk_dialog_run( GTK_DIALOG( dialog ) );
			gtk_widget_destroy( GTK_WIDGET( dialog ) );
			g_free( dummy );
			return;
		}
		old_file = TRUE;
	}
	g_free( dummy );

	/* Create hash table for efficient searching */
	table = g_hash_table_new_full( g_direct_hash, g_direct_equal,
								   NULL, g_free );
	model = gtk_combo_box_get_model( GTK_COMBO_BOX( img->transition_type ) );
	gtk_tree_model_foreach( model,
							(GtkTreeModelForeachFunc)img_populate_hash_table,
							&table );

	/* Set the slideshow options */
	img->video_size[1] = g_key_file_get_integer( img_key_file,
												 "slideshow settings",
												 "video format", NULL);
	gtk_widget_set_size_request( img->image_area,
								 img->video_size[0] * img->image_area_zoom,
								 img->video_size[1] * img->image_area_zoom );

	/* Make loading more efficient by removing model from icon view */
	g_object_ref( G_OBJECT( img->thumbnail_model ) );
	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ), NULL );
	gtk_icon_view_set_model( GTK_ICON_VIEW( img->over_icon ), NULL );

	/* Enable loading of old projects too */
	if( old_file )
	{
		guint32 tmp;
		dummy = g_key_file_get_string( img_key_file, "slideshow settings",
									   "background color", NULL );
		tmp = (guint32)strtoul( dummy, NULL, 16 );
		img->background_color[0] = (gdouble)( ( tmp >> 24 ) & 0xff ) / 0xff;
		img->background_color[1] = (gdouble)( ( tmp >> 16 ) & 0xff ) / 0xff;
		img->background_color[2] = (gdouble)( ( tmp >>  8 ) & 0xff ) / 0xff;

		/* Loads the thumbnails and set the slides info */
		number = g_key_file_get_integer(img_key_file,"images","number", NULL);

		/* Store the previous number of slides and set img->slides_nr so to have the correct number of slides displayed on the status bar */
		previous_nr_of_slides = img->slides_nr;
		img->slides_nr = number;
		gtk_widget_show(img->progress_bar);
		for (i = 1; i <= number; i++)
		{
			dummy = g_strdup_printf("image_%d",i);
			slide_filename = g_key_file_get_string(img_key_file,"images",dummy, NULL);

			if( img_scale_image( slide_filename, img->video_ratio, 88, 0,
								 img->distort_images, img->background_color,
								 &thumb, NULL ) )
			{
				GdkPixbuf *pix;

				speed 	=		g_key_file_get_integer(img_key_file, "transition speed",	dummy, NULL);
				duration= 		g_key_file_get_integer(img_key_file, "slide duration",		dummy, NULL);
				transition_id = g_key_file_get_integer(img_key_file, "transition type",		dummy, NULL);

				/* Get the mem address of the transition */
				spath = (gchar *)g_hash_table_lookup( table, GINT_TO_POINTER( transition_id ) );
				gtk_tree_model_get_iter_from_string( model, &iter, spath );
				gtk_tree_model_get( model, &iter, 2, &render, 0, &pix, -1 );
				slide_info = img_create_new_slide();
				if( slide_info )
				{
					img_set_slide_file_info( slide_info, slide_filename );
					gtk_list_store_append( img->thumbnail_model, &iter );
					gtk_list_store_set( img->thumbnail_model, &iter, 0, thumb, 1, slide_info, -1 );
					g_object_unref( G_OBJECT( thumb ) );

					/* Set non-default data */
					img_set_slide_still_info( slide_info, duration, img );
					img_set_slide_transition_info( slide_info,
												   img->thumbnail_model, &iter,
												   pix, spath, transition_id,
												   render, speed, img );
					g_object_unref( G_OBJECT( pix ) );

					/* Increment slide counter */
					img->slides_nr++;

					/* If we're loading the first slide, apply some of it's
				 	 * data to final pseudo-slide */
					if( first_slide )
					{
						first_slide = FALSE;
						img->final_transition.speed  = slide_info->speed;
						img->final_transition.render = slide_info->render;
					}
				}
			}

			img_increase_progressbar(img, i);
			g_free(slide_filename);
			g_free(dummy);
		}
	}
	else
	{
		gchar *subtitle = NULL, *font_desc;
		gdouble *my_points = NULL, *p_start, *p_stop, *c_start, *c_stop;
		gsize length;
		gint anim_id,anim_duration, text_pos, placing, gradient;
		GdkPixbuf *pix = NULL;
		gboolean   load_ok;
		ImgAngle   angle;
	
		/* Load project backgroud color */
		color = g_key_file_get_double_list( img_key_file, "slideshow settings",
											"background color", NULL, NULL );
		img->background_color[0] = color[0];
		img->background_color[1] = color[1];
		img->background_color[2] = color[2];
		g_free( color );

		/* Loads the thumbnails and set the slides info */
		number = g_key_file_get_integer( img_key_file, "slideshow settings",
										 "number of slides", NULL);
		/* Store the previous number of slides and set img->slides_nr so to have the correct number of slides displayed on the status bar */
		previous_nr_of_slides = img->slides_nr;
		img->slides_nr = number;

		gtk_widget_show( img->progress_bar );
		for( i = 1; i <= number; i++ )
		{
			conf = g_strdup_printf("slide %d", i);
			slide_filename = g_key_file_get_string(img_key_file,conf,"filename", NULL);

			if( slide_filename )
			{
				angle = (ImgAngle)g_key_file_get_integer( img_key_file, conf,
														  "angle", NULL );
				load_ok = img_scale_image( slide_filename, img->video_ratio,
										   88, 0, img->distort_images,
										   img->background_color, &thumb, NULL );
			}
			else
			{
				/* We are loading an empty slide */
				gradient = g_key_file_get_integer(img_key_file, conf, "gradient", NULL);
				c_start = g_key_file_get_double_list(img_key_file, conf, "start_color", NULL, NULL);
				c_stop  = g_key_file_get_double_list(img_key_file, conf, "stop_color", NULL, NULL);
				p_start = g_key_file_get_double_list(img_key_file, conf, "start_point", NULL, NULL);
				p_stop = g_key_file_get_double_list(img_key_file, conf, "stop_point", NULL, NULL);

				/* Create thumbnail */
				load_ok = img_scale_gradient( gradient, p_start, p_stop,
											  c_start, c_stop, 88, 72,
											  &thumb, NULL );
			}

			/* Try to load image. If this fails, skip this slide */
			if( load_ok )
			{
				duration	  = g_key_file_get_integer(img_key_file, conf, "duration", NULL);
				transition_id = g_key_file_get_integer(img_key_file, conf, "transition_id", NULL);
				speed 		  =	g_key_file_get_integer(img_key_file, conf, "speed",	NULL);

				/* Load the stop points if any */
				no_points	  =	g_key_file_get_integer(img_key_file, conf, "no_points",	NULL);
				if (no_points > 0)
					my_points = g_key_file_get_double_list(img_key_file, conf, "points", &length, NULL);

				/* Load the slide text related data */
				subtitle	  =	g_key_file_get_string (img_key_file, conf, "text",	NULL);
				anim_id 	  = g_key_file_get_integer(img_key_file, conf, "anim id", 		NULL);
				anim_duration = g_key_file_get_integer(img_key_file, conf, "anim duration",	NULL);
				text_pos      = g_key_file_get_integer(img_key_file, conf, "text pos",		NULL);
				placing 	  = g_key_file_get_integer(img_key_file, conf, "placing",		NULL);
				font_desc     = g_key_file_get_string (img_key_file, conf, "font", 			NULL);
				font_color 	  = g_key_file_get_double_list(img_key_file, conf, "font color", NULL, NULL );

				/* Get the mem address of the transition */
				spath = (gchar *)g_hash_table_lookup( table, GINT_TO_POINTER( transition_id ) );
				gtk_tree_model_get_iter_from_string( model, &iter, spath );
				gtk_tree_model_get( model, &iter, 2, &render, 0, &pix, -1 );

				slide_info = img_create_new_slide();
				if( slide_info )
				{
					if( slide_filename )
						img_set_slide_file_info( slide_info, slide_filename );
					else
						img_set_slide_gradient_info( slide_info, gradient,
													 c_start, c_stop,
													 p_start, p_stop );

					/* If image has been rotated, rotate it now too. */
					if( angle )
					{
						img_rotate_slide( slide_info, angle, NULL );
						g_object_unref( thumb );
						img_scale_image( slide_info->r_filename, img->video_ratio,
										 88, 0, img->distort_images,
										 img->background_color, &thumb, NULL );
					}

					gtk_list_store_append( img->thumbnail_model, &iter );
					gtk_list_store_set( img->thumbnail_model, &iter,
										0, thumb,
										1, slide_info,
										-1 );
					g_object_unref( G_OBJECT( thumb ) );

					/* Set duration */
					img_set_slide_still_info( slide_info, duration, img );

					/* Set transition */
					img_set_slide_transition_info( slide_info,
												   img->thumbnail_model, &iter,
												   pix, spath, transition_id,
												   render, speed, img );

					/* Set stop points */
					if( no_points > 0 )
					{
						img_set_slide_ken_burns_info( slide_info, 0,
													  length, my_points );
						g_free( my_points );
					}

					/* Set subtitle */
					img_set_slide_text_info( slide_info, img->thumbnail_model,
											 &iter, subtitle, anim_id,
											 anim_duration, text_pos, placing,
											 font_desc, font_color, img );

					/* If we're loading the first slide, apply some of it's
				 	 * data to final pseudo-slide */
					if( first_slide )
					{
						first_slide = FALSE;
						img->final_transition.speed  = slide_info->speed;
						img->final_transition.render = slide_info->render;
					}
				}
				if (pix)
					g_object_unref( G_OBJECT( pix ) );
				g_free( font_desc );
			}
			else
				img->slides_nr--;

			img_increase_progressbar(img, i);
			g_free(slide_filename);
			if (subtitle)
				g_free(subtitle);
			g_free(conf);
		}
	}
	img->slides_nr += previous_nr_of_slides;
	img->distort_images = g_key_file_get_boolean( img_key_file,
												  "slideshow settings",
												  "distort images", NULL );
	gtk_widget_hide(img->progress_bar);

	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ),
							 GTK_TREE_MODEL( img->thumbnail_model ) );
	gtk_icon_view_set_model( GTK_ICON_VIEW( img->over_icon ),
							 GTK_TREE_MODEL( img->thumbnail_model ) );
	g_object_unref( G_OBJECT( img->thumbnail_model ) );

	/* Loads the audio files in the liststore */
	number = g_key_file_get_integer(img_key_file, "music", "number", NULL);
	for (i = 1; i <= number; i++)
	{
		dummy = g_strdup_printf("music_%d", i);
		slide_filename = g_key_file_get_string(img_key_file, "music", dummy, NULL);
		img_add_audio_files(slide_filename, img);

		/* slide_filename is freed in img_add_audio_files */
		g_free(dummy);
	}
	g_key_file_free (img_key_file);
	img_set_total_slideshow_duration(img);

	img_set_statusbar_message(img, 0);

	dummy = g_path_get_basename( input );
	img_set_window_title(img, dummy);
	g_free(dummy);

	g_hash_table_destroy( table );

	/* If we made it to here, we succesfully loaded project, so it's safe to set
	 * filename field in global data structure. */
	if( img->project_filename )
		g_free( img->project_filename );
	img->project_filename = g_strdup( input );
	
	/* Select the first slide */
	img_goto_first_slide(NULL, img);
	img->project_is_modified = FALSE;

	/* Update incompatibilities display */
	img_update_inc_audio_display( img );

	time = img_convert_seconds_to_time(img->total_music_secs);
	gtk_label_set_text(GTK_LABEL(img->music_time_data), time);
	g_free(time);
}

static gboolean img_populate_hash_table( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GHashTable **table )
{
	gint         id;

	gtk_tree_model_get( model, iter, 3, &id, -1 );

	/* Leave out family names, since hey don't get saved. */
	if( ! id )
		return( FALSE );

	/* Freeing of this memory is done automatically when the list gets
	 * destroyed, since we supplied destroy notifier handler. */
	g_hash_table_insert( *table, GINT_TO_POINTER( id ), (gpointer)gtk_tree_path_to_string( path ) );

	return( FALSE );
}

