/*
 *  Copyright (C) 2009 Giuseppe Torelli - <colossus73@gmail.com>
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

void img_save_slideshow(img_window_struct *img)
{
	GKeyFile *img_key_file;
	gchar *conf, *string, *path, *filename, *file;
	gint count = 0;
	gsize len;
	GtkTreeIter iter;
	slide_struct *entry;
	GtkTreeModel *model;

	model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview));
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;

	img_key_file = g_key_file_new();

	/* Slideshow settings */
	g_key_file_set_comment(img_key_file, NULL, NULL, comment_string, NULL);


	if ((img->image_area)->allocation.height == 480)
		g_key_file_set_integer(img_key_file,"slideshow settings","video format", 480);
	else
		g_key_file_set_integer(img_key_file,"slideshow settings","video format", 576);
	conf = g_strdup_printf( "%lx", (gulong)img->background_color );
	g_key_file_set_string(img_key_file,"slideshow settings", "background color", conf);
	g_free( conf );
	g_key_file_set_boolean(img_key_file,"slideshow settings", "distort images", img->distort_images);

	/* Slide individual settings */
	g_key_file_set_integer(img_key_file, "images", "number", img->slides_nr);
	do
	{
		count++;
		gtk_tree_model_get(model, &iter,1,&entry,-1);
		conf = g_strdup_printf("image_%d",count);

		if (entry->slide_original_filename)
			g_key_file_set_string(img_key_file, "images",		conf, entry->slide_original_filename);
		else
			g_key_file_set_string(img_key_file, "images",		conf, entry->filename);
		g_key_file_set_integer(img_key_file,"transition speed", conf, entry->speed);
		g_key_file_set_integer(img_key_file,"slide duration",	conf, entry->duration);
		g_key_file_set_integer(img_key_file,"transition type",	conf, entry->transition_id);
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
			g_key_file_set_string(img_key_file, "music",			conf, file);
			g_free(file);
			g_free(conf);
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	/* Write the project file */
	conf = g_key_file_to_data(img_key_file, &len, NULL);
	g_file_set_contents(img->project_filename, conf, len, NULL);
	g_free (conf);

	string = g_path_get_basename(img->project_filename);
	img_set_window_title(img,string);
	g_free(string);
	g_key_file_free(img_key_file);

	img->project_is_modified = FALSE;
}

void img_load_slideshow(img_window_struct *img)
{
	GdkPixbuf *thumb;
	slide_struct *slide_info;
	GtkTreeIter iter;
	GKeyFile *img_key_file;
	gchar *dummy, *slide_filename, *time;
	GtkWidget *dialog;
	gint not_found = 0,number,i,transition_id, height, duration;
	guint speed;
	GtkTreeModel *model;
	void (*render);
	GHashTable *table;
	gchar      *spath;

	img_key_file = g_key_file_new();
	if(!g_key_file_load_from_file(img_key_file,img->project_filename,G_KEY_FILE_KEEP_COMMENTS,NULL))
	{
		g_key_file_free( img_key_file );
		return;
	}

	dummy = g_key_file_get_comment(img_key_file,NULL,NULL,NULL);

	if (strncmp(dummy,comment_string,strlen(comment_string)) != 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(img->imagination_window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("This is not an Imagination project file!"));
		gtk_window_set_title(GTK_WINDOW(dialog),"Imagination");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
		g_free(dummy);
		return;
	}
	g_free(dummy);

	/* Create hash table for efficient searching */
	table = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, g_free );
	model = gtk_combo_box_get_model( GTK_COMBO_BOX( img->transition_type ) );
	gtk_tree_model_foreach( model, (GtkTreeModelForeachFunc)img_populate_hash_table, &table );

	/* Set the slideshow options */
	height = g_key_file_get_integer(img_key_file,"slideshow settings","video format", NULL);
	gtk_widget_set_size_request( img->image_area, 720, height );
	dummy = g_key_file_get_string(img_key_file, "slideshow settings", "background color", NULL );
	img->background_color = (guint32)strtoul( dummy, NULL, 16 );
	g_free(dummy);
	img->distort_images = g_key_file_get_boolean(img_key_file, "slideshow settings", "distort images", NULL );

	/* Make loading more efficient by removing model from icon view */
	g_object_ref( G_OBJECT( img->thumbnail_model ) );
	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ), NULL );

	/* Loads the thumbnails and set the slides info */
	number = g_key_file_get_integer(img_key_file,"images","number", NULL);
	img->slides_nr = number;
	gtk_widget_show(img->progress_bar);
	for (i = 1; i <= number; i++)
	{
		dummy = g_strdup_printf("image_%d",i);
		slide_filename = g_key_file_get_string(img_key_file,"images",dummy, NULL);

		thumb = img_load_pixbuf_from_file(slide_filename);
		if (thumb)
		{
			speed 	=		g_key_file_get_integer(img_key_file, "transition speed",	dummy, NULL);
			duration= 		g_key_file_get_integer(img_key_file, "slide duration",		dummy, NULL);
			transition_id = g_key_file_get_integer(img_key_file, "transition type",		dummy, NULL);

			/* Get the mem address of the transition */
			spath = (gchar *)g_hash_table_lookup( table, GINT_TO_POINTER( transition_id ) );
			gtk_tree_model_get_iter_from_string( model, &iter, spath );
			gtk_tree_model_get( model, &iter, 2, &render, -1 );
			slide_info = img_set_slide_info( duration, speed, render, transition_id, spath, slide_filename );
			if( slide_info )
			{
				gtk_list_store_append( img->thumbnail_model, &iter );
				gtk_list_store_set( img->thumbnail_model, &iter, 0, thumb, 1, slide_info, -1 );
				g_object_unref( G_OBJECT( thumb ) );

				/* If we're loading the first slide, apply some of it's
				 * data to final pseudo-slide */
				if( img->slides_nr == 1 )
				{
					img->final_transition.speed  = slide_info->speed;
					img->final_transition.render = slide_info->render;
				}
			}
		}
		else
			not_found++;

		img_increase_progressbar(img, i);
		g_free(slide_filename);
		g_free(dummy);
	}
	img->slides_nr -= not_found;
	gtk_widget_hide(img->progress_bar);

	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ),
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

	time = img_convert_seconds_to_time(img->total_music_secs);
	gtk_label_set_text(GTK_LABEL(img->music_time_data), time);
	g_free(time);

	img_set_statusbar_message(img, 0);

	dummy = g_path_get_basename(img->project_filename);
	img_set_window_title(img, dummy);
	g_free(dummy);

	g_hash_table_destroy( table );
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

