/*
 *  Copyright (c) 2009 Giuseppe Torelli <colossus73@gmail.com>
 *  Copyright (c) 2009 Tadej Borovšak 	<tadeboro@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License,or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not,write to the Free Software
 *  Foundation,Inc.,59 Temple Place - Suite 330,Boston,MA 02111-1307,USA.
 *
 */

#include "callbacks.h"
#include "export.h"

static void img_file_chooser_add_preview(img_window_struct *);
static void img_update_preview_file_chooser(GtkFileChooser *,img_window_struct *);
static gboolean img_transition_timeout(img_window_struct *);
static gboolean img_sleep_timeout(img_window_struct *);
static void img_swap_toolbar_images( img_window_struct *, gboolean);
static void img_clean_after_preview(img_window_struct *);
static void img_about_dialog_activate_link(GtkAboutDialog * , const gchar *, gpointer );
static GdkPixbuf *img_rotate_pixbuf_c( GdkPixbuf *, GtkProgressBar *);
static GdkPixbuf *img_rotate_pixbuf_cc( GdkPixbuf *, GtkProgressBar *);
void img_set_window_title(img_window_struct *img, gchar *text)
{
	gchar *title = NULL;
	static gchar version[] = VERSION "-" REVISION;

	if (text == NULL)
	{
		title = g_strconcat("Imagination ", strcmp(REVISION, "-1") == 0 ? VERSION : version, NULL);
		gtk_window_set_title (GTK_WINDOW (img->imagination_window), title);
		g_free(title);
	}
	else
	{
		title = g_strconcat(text, " - Imagination ", strcmp(REVISION, "-1") == 0 ? VERSION : version, NULL);
		gtk_window_set_title (GTK_WINDOW (img->imagination_window), title);
		g_free(title);
	}
}

void img_new_slideshow(GtkMenuItem *item,img_window_struct *img_struct)
{
	img_new_slideshow_settings_dialog(img_struct, FALSE);
}

void img_project_properties(GtkMenuItem *item, img_window_struct *img_struct)
{
	img_new_slideshow_settings_dialog(img_struct, TRUE);
}

void img_add_slides_thumbnails(GtkMenuItem *item, img_window_struct *img)
{
	GSList	*slides = NULL, *bak;
	GdkPixbuf *thumb;
	GtkTreeIter iter;
	slide_struct *slide_info;
	gint slides_cnt = 0;

	slides = img_import_slides_file_chooser(img);

	if (slides == NULL)
		return;

	img->slides_nr += g_slist_length(slides);
	gtk_widget_show(img->progress_bar);

	/* Remove model from thumbnail iconview for efficiency */
	g_object_ref( G_OBJECT( img->thumbnail_model ) );
	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ), NULL );

	bak = slides;
	while (slides)
	{
		thumb = img_load_pixbuf_from_file(slides->data);
		if (thumb)
		{
			slide_info = img_set_slide_info(1, NORMAL, NULL, -1, "0", slides->data);
			if (slide_info)
			{
				gtk_list_store_append (img->thumbnail_model,&iter);
				gtk_list_store_set (img->thumbnail_model, &iter, 0, thumb, 1, slide_info, -1);
				g_object_unref (thumb);
				slides_cnt++;
			}
			g_free(slides->data);
		}
		img_increase_progressbar(img, slides_cnt);
		slides = slides->next;
	}
	gtk_widget_hide(img->progress_bar);
	g_slist_free(bak);
	img_set_total_slideshow_duration(img);
	img_set_statusbar_message(img,0);
	img->project_is_modified = TRUE;

	gtk_icon_view_set_model( GTK_ICON_VIEW( img->thumbnail_iconview ),
							 GTK_TREE_MODEL( img->thumbnail_model ) );
	g_object_unref( G_OBJECT( img->thumbnail_model ) );
}

void img_increase_progressbar(img_window_struct *img, gint nr)
{
	gchar *message;
	gdouble percent;

	percent = (gdouble)nr / img->slides_nr;
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (img->progress_bar), percent);
	message = g_strdup_printf(_("Please wait, importing slide %d out of %d"), nr, img->slides_nr);
	gtk_statusbar_push(GTK_STATUSBAR(img->statusbar), img->context_id, message);
	g_free(message);

	while (gtk_events_pending())
		gtk_main_iteration();
}

void img_remove_audio_files (GtkWidget *widget, img_window_struct *img)
{
	GtkTreeSelection *sel;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *rr_list = NULL;
	GList *node;
	gchar *time;
	gint secs;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(img->music_file_treeview));
	gtk_tree_selection_selected_foreach(sel, (GtkTreeSelectionForeachFunc) img_remove_foreach_func, &rr_list);

	for (node = rr_list; node != NULL; node = node->next)
	{
		path = gtk_tree_row_reference_get_path((GtkTreeRowReference *) node->data);
		if (path)
	    {
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(img->music_file_liststore), &iter, path))
			{
				gtk_tree_model_get(GTK_TREE_MODEL(img->music_file_liststore), &iter, 3, &secs, -1);
				gtk_list_store_remove(img->music_file_liststore, &iter);
			}
			gtk_tree_path_free(path);
		}
		img->total_music_secs -= secs;
	}
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(img->music_file_liststore), &iter) == FALSE)
	{
		gtk_widget_set_sensitive (img->remove_audio_button, FALSE);
		gtk_widget_set_sensitive (img->play_audio_button, FALSE);
		gtk_label_set_text(GTK_LABEL(img->music_time_data), "");
	}
	else
	{
		time = img_convert_seconds_to_time(img->total_music_secs);
		gtk_label_set_text(GTK_LABEL(img->music_time_data), time);
		g_free(time);
	}
	g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free(rr_list);
}

void img_remove_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, GList **rowref_list)
{
	GtkTreeRowReference *rowref;

	rowref = gtk_tree_row_reference_new(model, path);
	*rowref_list = g_list_append(*rowref_list, rowref);
}

void img_select_audio_files_to_add ( GtkMenuItem* button, img_window_struct *img)
{
	GtkFileFilter *audio_filter, *all_files_filter;
	GtkWidget *fs;
	GSList *files = NULL;
	gint response;
	gchar *time = NULL;

	fs = gtk_file_chooser_dialog_new (_("Please choose the audio files to import"),
							GTK_WINDOW (img->imagination_window),
							GTK_FILE_CHOOSER_ACTION_OPEN,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_CANCEL,
							GTK_STOCK_OPEN,
							GTK_RESPONSE_ACCEPT,
							NULL);

	/* only audio files filter */
	audio_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (audio_filter, _("All audio files") );
	gtk_file_filter_add_pattern (audio_filter, "*.wav");
	gtk_file_filter_add_pattern (audio_filter, "*.mp3");
	gtk_file_filter_add_pattern (audio_filter, "*.ogg");
	gtk_file_filter_add_pattern (audio_filter, "*.flac");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fs), audio_filter);

	/* All files filter */
	all_files_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name(all_files_filter, _("All files"));
	gtk_file_filter_add_pattern(all_files_filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fs), all_files_filter);

	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (fs), TRUE);

	response = gtk_dialog_run (GTK_DIALOG (fs));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		files = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (fs));
		g_slist_foreach( files, (GFunc) img_add_audio_files, img);
	}
	if (files != NULL)
	{
		g_slist_foreach(files, (GFunc) g_free, NULL);
		g_slist_free (files);
	}	

	time = img_convert_seconds_to_time(img->total_music_secs);
	gtk_label_set_text(GTK_LABEL(img->music_time_data), time);
	g_free(time);

	gtk_widget_destroy (fs);
}

void img_add_audio_files (gchar *filename, img_window_struct *img)
{
	GtkTreeIter iter;
	gchar *path, *file, *time;
	gint secs;

	path = g_path_get_dirname(filename);
	file = g_path_get_basename(filename);
	time = img_get_audio_length(img, filename, &secs);

	if (time != NULL)
	{
		gtk_list_store_append(img->music_file_liststore, &iter);
		gtk_list_store_set (img->music_file_liststore, &iter, 0, path, 1, file, 2, time, 3, secs, -1);

		img->total_music_secs += secs;
		g_free(time);
	}
	g_free(path);
	g_free(file);
}

GSList *img_import_slides_file_chooser(img_window_struct *img)
{
	GtkFileFilter *all_images_filter, *all_files_filter;
	GSList *slides = NULL;
	int response;

	img->import_slide_chooser = gtk_file_chooser_dialog_new (_("Import slides, use SHIFT key for multiple select"),
						GTK_WINDOW (img->imagination_window),
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN,
						GTK_RESPONSE_ACCEPT,
						NULL);
	img_file_chooser_add_preview(img);

	/* Image files filter */
	all_images_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name(all_images_filter,_("All image files"));
	gtk_file_filter_add_pixbuf_formats( all_images_filter );
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(img->import_slide_chooser),all_images_filter);

	/* All files filter */
	all_files_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name(all_files_filter,_("All files"));
	gtk_file_filter_add_pattern(all_files_filter,"*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(img->import_slide_chooser),all_files_filter);

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(img->import_slide_chooser),TRUE);
	if (img->current_dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(img->import_slide_chooser),img->current_dir);
	response = gtk_dialog_run (GTK_DIALOG(img->import_slide_chooser));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		slides = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(img->import_slide_chooser));
		if (img->current_dir)
			g_free(img->current_dir);
		img->current_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(img->import_slide_chooser));
	}
	gtk_widget_destroy (img->import_slide_chooser);
	return slides;
}

void img_free_allocated_memory(img_window_struct *img_struct)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GSList *bak = NULL;
	slide_struct *entry;

	/* Free the memory allocated the single slides one by one */
	if (img_struct->slides_nr)
	{
		model = gtk_icon_view_get_model (GTK_ICON_VIEW(img_struct->thumbnail_iconview));

		gtk_tree_model_get_iter_first(model,&iter);
		do
		{
			gtk_tree_model_get(model, &iter,1,&entry,-1);
			if (entry->slide_original_filename)
				g_free(entry->slide_original_filename);
			g_free(entry->filename);
			g_free(entry->path);
			g_free(entry->resolution);
			g_free(entry->type);
			g_free(entry);
			img_struct->slides_nr--;
		}
		while (gtk_tree_model_iter_next (model,&iter));
		g_signal_handlers_block_by_func((gpointer)img_struct->thumbnail_iconview, (gpointer)img_iconview_selection_changed, img_struct);
		gtk_list_store_clear(GTK_LIST_STORE(img_struct->thumbnail_model));
		g_signal_handlers_unblock_by_func((gpointer)img_struct->thumbnail_iconview, (gpointer)img_iconview_selection_changed, img_struct);
	}

	/* Unlink the possible created rotated pictures and free the GSlist */
	if (img_struct->rotated_files)
	{
		bak = img_struct->rotated_files;
		while(img_struct->rotated_files)
		{
			unlink (img_struct->rotated_files->data);
			g_free(img_struct->rotated_files->data);
			img_struct->rotated_files = img_struct->rotated_files->next;
		}
		g_slist_free(bak);
	}

	/* Delete the audio files in the liststore */
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(img_struct->music_file_liststore), &iter))
	{
		gtk_list_store_clear(img_struct->music_file_liststore);
		img_struct->total_music_secs = 0;
		gtk_label_set_text(GTK_LABEL(img_struct->music_time_data), "");
	}

	/* Free gchar pointers */
	if (img_struct->current_dir)
	{
		g_free(img_struct->current_dir);
		img_struct->current_dir = NULL;
	}

	if (img_struct->project_filename)
	{
		g_free(img_struct->project_filename);
		img_struct->project_filename = NULL;
	}
}

gint img_ask_user_confirmation(img_window_struct *img_struct, gchar *msg)
{
	GtkWidget *dialog;
	gint response;

	dialog = gtk_message_dialog_new(GTK_WINDOW(img_struct->imagination_window),GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL, "%s.", msg);
	gtk_window_set_title(GTK_WINDOW(dialog),"Imagination");
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
	return response;
}

gboolean img_quit_application(GtkWidget *widget, GdkEvent *event, img_window_struct *img_struct)
{
	gint response;

	if (img_struct->project_is_modified)
	{
		response = img_ask_user_confirmation(img_struct, _("You didn't save your slideshow yet. Are you sure you want to close it?"));
		if (response != GTK_RESPONSE_OK)
			return TRUE;
	}
	img_free_allocated_memory(img_struct);

	/* Unloads the plugins */
	g_slist_foreach(img_struct->plugin_list,(GFunc)g_module_close,NULL);
	g_slist_free(img_struct->plugin_list);
	return FALSE;
}

static void img_file_chooser_add_preview(img_window_struct *img_struct)
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), 10);

	img_struct->preview_image = gtk_image_new ();

	img_struct->dim_label  = gtk_label_new (NULL);
	img_struct->size_label = gtk_label_new (NULL);

	gtk_box_pack_start (GTK_BOX (vbox), img_struct->preview_image, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), img_struct->dim_label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), img_struct->size_label, FALSE, TRUE, 0);
	gtk_widget_show_all (vbox);

	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER(img_struct->import_slide_chooser), vbox);
	gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER(img_struct->import_slide_chooser), FALSE);

	g_signal_connect (img_struct->import_slide_chooser, "update-preview",G_CALLBACK (img_update_preview_file_chooser), img_struct);
}

static void	img_update_preview_file_chooser(GtkFileChooser *file_chooser,img_window_struct *img_struct)
{
	gchar *filename,*size;
	gboolean has_preview = FALSE;
	gint width,height;
	GdkPixbuf *pixbuf;
	GdkPixbufFormat *pixbuf_format;

	filename = gtk_file_chooser_get_filename(file_chooser);
	if (filename == NULL)
	{
		gtk_file_chooser_set_preview_widget_active (file_chooser, has_preview);
		return;
	}
	pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, 93, 70, TRUE, NULL);
	has_preview = (pixbuf != NULL);
	if (has_preview)
	{
		pixbuf_format = gdk_pixbuf_get_file_info(filename,&width,&height);
		gtk_image_set_from_pixbuf (GTK_IMAGE(img_struct->preview_image), pixbuf);
		g_object_unref (pixbuf);

		size = g_strdup_printf(ngettext("%d x %d pixels", "%d x %d pixels", height),width,height);
		gtk_label_set_text(GTK_LABEL(img_struct->dim_label),size);
		g_free(size);
	}
	g_free(filename);
	gtk_file_chooser_set_preview_widget_active (file_chooser, has_preview);
}

void img_delete_selected_slides(GtkMenuItem *item,img_window_struct *img_struct)
{
	GList *selected, *bak;
	GtkTreeIter iter;
	GtkTreeModel *model;
	slide_struct *entry;

	model =	gtk_icon_view_get_model(GTK_ICON_VIEW(img_struct->thumbnail_iconview));
	
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(img_struct->thumbnail_iconview));
	if (selected == NULL)
		return;
	
	/* Free the slide struct for each slide and remove it from the iconview */
	/* Stored list start to avoid memory leak + pulled signal blocks one level
	 * higher to avoid calling them separately for each entry in list (we are
	 * removing items synchronously from withing the callback, so the GUI is
	 * already "frozen" and selection cannot be changed through the user
	 * intervention until the callback returns). */
	bak = selected;
	g_signal_handlers_block_by_func((gpointer)img_struct->thumbnail_iconview,(gpointer)img_iconview_selection_changed, img_struct);
	while (selected)
	{
		gtk_tree_model_get_iter(model, &iter,selected->data);
		gtk_tree_model_get(model, &iter,1,&entry,-1);
		g_free(entry->filename); g_free(entry->resolution);
		g_free(entry->type); g_free(entry);

		gtk_list_store_remove(GTK_LIST_STORE(img_struct->thumbnail_model),&iter);
 
		img_struct->slides_nr--; selected = selected->next;
	}
	g_signal_handlers_unblock_by_func((gpointer)img_struct->thumbnail_iconview,(gpointer)img_iconview_selection_changed, img_struct);
	g_list_foreach (bak, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(bak);

	img_set_statusbar_message(img_struct,0);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_struct->image_area),NULL);
	img_struct->project_is_modified = TRUE;
	img_iconview_selection_changed(GTK_ICON_VIEW(img_struct->thumbnail_iconview),img_struct);
}

void img_rotate_selected_slide(GtkWidget *button, img_window_struct *img)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GList *selected = NULL;
	GdkPixbuf *thumb, *rotated_thumb;
	GError *error = NULL;
	GtkWidget *dialog;
	slide_struct *info_slide;
	gchar *filename;
	gint handle;

	/* Obtain the selected slideshow filename */
	model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview));
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(img->thumbnail_iconview));

	if( selected == NULL)
		return;

	gtk_widget_show(img->progress_bar);
	gtk_tree_model_get_iter(model,&iter,selected->data);
	gtk_tree_path_free(selected->data);
	g_list_free (selected);
	gtk_tree_model_get(model,&iter,1,&info_slide,-1);

	/* Load the image, save a copy in the temp dir, rotate it and display it in the image area */
	thumb = gdk_pixbuf_new_from_file(info_slide->filename, NULL);
	if (button == img->rotate_left_button)
		rotated_thumb = img_rotate_pixbuf_c( thumb, GTK_PROGRESS_BAR( img->progress_bar ) );
	else
		rotated_thumb = img_rotate_pixbuf_cc( thumb, GTK_PROGRESS_BAR( img->progress_bar ) );
	g_object_unref(thumb);

	gtk_widget_hide(img->progress_bar);

	handle = g_file_open_tmp("img-XXXXXX.jpg", &filename, NULL);
	close(handle);
	if ( ! gdk_pixbuf_save(rotated_thumb, filename, "jpeg",  &error, NULL))
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(img->imagination_window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("An error occurred while trying to rotate the slide:"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s.", error->message);
		gtk_window_set_title(GTK_WINDOW(dialog),"Imagination");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
		g_error_free(error);
		g_free(filename);
		return;
	}
	g_object_unref(rotated_thumb);

	if (info_slide->slide_original_filename == NULL)
		info_slide->slide_original_filename = info_slide->filename;

	info_slide->filename = filename;
	img->rotated_files = g_slist_append(img->rotated_files, g_strdup(filename));

	img->slide_pixbuf = img_scale_pixbuf(img, filename);
	gtk_image_set_from_pixbuf(GTK_IMAGE (img->image_area),img->slide_pixbuf);
	g_object_unref(img->slide_pixbuf);

	/* Display the rotated image in thumbnails iconview */
	thumb = gdk_pixbuf_new_from_file_at_scale(filename, 93, 70, TRUE, NULL);
	gtk_list_store_set (img->thumbnail_model, &iter, 0, thumb, -1);
	g_object_unref (thumb);
}

/* Rotate clockwise */
static GdkPixbuf *img_rotate_pixbuf_c( GdkPixbuf      *original,
									   GtkProgressBar *progress )
{
	GdkPixbuf     *new;
	gint           w, h, r1, r2, channels, bps;
	GdkColorspace  colorspace;
	gboolean       alpha;
	guchar        *pixels1, *pixels2;
	gint           i, j;

	/* Get data from source */
	g_object_get( G_OBJECT( original ), "width", &w,
										"height", &h,
										"rowstride", &r1,
										"n_channels", &channels,
										"bits_per_sample", &bps,
										"colorspace", &colorspace,
										"has_alpha", &alpha,
										"pixels", &pixels1,
										NULL );

	/* Create new rotated image */
	new = gdk_pixbuf_new( colorspace, alpha, bps, h, w );
	g_object_get( G_OBJECT( new ), "rowstride", &r2,
								   "pixels", &pixels2,
								   NULL );

	/* Copy data, applying transormation along the way */
	for( j = 0; j < h; j++ )
	{
		for( i = 0; i < w; i++ )
		{
			int source = i * channels + r1 * j;
			int dest   = j * channels + r2 * ( w - i - 1 );
			int n;

			for( n = 0; n < channels; n++ )
				pixels2[dest + n] = pixels1[source + n];
		}

		if( j % 100 )
			continue;

		/* Update progress bar */
		gtk_progress_bar_set_fraction( progress, (gdouble)( j + 1 ) / h );
		while( gtk_events_pending() )
			gtk_main_iteration();
	}

	return( new );
}

/* Rotate counter-clockwise */
static GdkPixbuf *img_rotate_pixbuf_cc( GdkPixbuf      *original,
										GtkProgressBar *progress )
{
	GdkPixbuf     *new;
	gint           w, h, r1, r2, channels, bps;
	GdkColorspace  colorspace;
	gboolean       alpha;
	guchar        *pixels1, *pixels2;
	gint           i, j;

	/* Get data from source */
	g_object_get( G_OBJECT( original ), "width", &w,
										"height", &h,
										"rowstride", &r1,
										"n_channels", &channels,
										"bits_per_sample", &bps,
										"colorspace", &colorspace,
										"has_alpha", &alpha,
										"pixels", &pixels1,
										NULL );

	/* Create new rotated image */
	new = gdk_pixbuf_new( colorspace, alpha, bps, h, w );
	g_object_get( G_OBJECT( new ), "rowstride", &r2,
								   "pixels", &pixels2,
								   NULL );

	/* Copy data, applying transormation along the way */
	for( j = 0; j < h; j++ )
	{
		for( i = 0; i < w; i++ )
		{
			int source = i * channels + r1 * j;
			int dest   = ( h - j - 1 ) * channels + r2 * i;
			int n;

			for( n = 0; n < channels; n++ )
				pixels2[dest + n] = pixels1[source + n];
		}

		if( j % 100 )
			continue;

		/* Update progress bar */
		gtk_progress_bar_set_fraction( progress, (gdouble)( j + 1 ) / h );
		while( gtk_events_pending() )
			gtk_main_iteration();
	}

	return( new );
}

void img_show_about_dialog (GtkMenuItem *item,img_window_struct *img_struct)
{
	static GtkWidget *about = NULL;
	static gchar version[] = VERSION "-" REVISION;
    const char *authors[] = {"\nDevelopers:\nGiuseppe Torelli <colossus73@gmail.com>\nTadej Borovšak <tadeboro@gmail.com>\n\nImagination logo: http://linuxgraphicsusers.com\n\n",NULL};
    //const char *documenters[] = {NULL};

	if (about == NULL)
	{
		about = gtk_about_dialog_new ();
		gtk_about_dialog_set_url_hook(img_about_dialog_activate_link, NULL, NULL);
		gtk_window_set_position (GTK_WINDOW (about),GTK_WIN_POS_CENTER_ON_PARENT);
		gtk_window_set_transient_for (GTK_WINDOW (about),GTK_WINDOW (img_struct->imagination_window));
		gtk_window_set_destroy_with_parent (GTK_WINDOW (about),TRUE);
		g_object_set (about,
			"name", "Imagination",
			"version", strcmp(REVISION, "-1") == 0 ? VERSION : version,
			"copyright","Copyright \xC2\xA9 2009 Giuseppe Torelli",
			"comments","A simple and lightweight DVD slideshow maker",
			"authors",authors,
			"documenters",NULL,
			"translator_credits",_("translator-credits"),
			"logo_icon_name","imagination",
			"website","http://imagination.sf.net",
			"license","Copyright \xC2\xA9 2009 Giuseppe Torelli - Colossus <colossus73@gmail.com>\n\n"
		    			"This is free software; you can redistribute it and/or\n"
    					"modify it under the terms of the GNU Library General Public License as\n"
    					"published by the Free Software Foundation; either version 2 of the\n"
    					"License,or (at your option) any later version.\n"
    					"\n"
    					"This software is distributed in the hope that it will be useful,\n"
    					"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    					"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
    					"Library General Public License for more details.\n"
    					"\n"
    					"You should have received a copy of the GNU Library General Public\n"
    					"License along with the Gnome Library; see the file COPYING.LIB.  If not,\n"
    					"write to the Free Software Foundation,Inc.,59 Temple Place - Suite 330,\n"
    					"Boston,MA 02111-1307,USA.\n",
		      NULL);
	}
	gtk_dialog_run ( GTK_DIALOG(about));
	gtk_widget_hide (about);
}

static void img_about_dialog_activate_link(GtkAboutDialog * dialog, const gchar *link, gpointer data)
{
	char * argv[] = { "xdg-open", (char*)link, NULL };

	g_spawn_async( NULL, argv, NULL, G_SPAWN_SEARCH_PATH,NULL, NULL, NULL, NULL);
}

void img_set_total_slideshow_duration(img_window_struct *img)
{
	gchar *time;
	GtkTreeIter iter;
	slide_struct *entry;
	GtkTreeModel *model;

	model = GTK_TREE_MODEL( img->thumbnail_model );
	if (!gtk_tree_model_get_iter_first (model,&iter))
		return;

	img->total_secs = 0;
	do
	{
		gtk_tree_model_get(model, &iter,1,&entry,-1);
		img->total_secs += entry->duration;

		if(entry->render)
			img->total_secs += entry->speed;
	}
	while (gtk_tree_model_iter_next (model,&iter));

	/* Add time of last pseudo slide */
	if( img->final_transition.render )
		img->total_secs += img->final_transition.speed;

	time = img_convert_seconds_to_time(img->total_secs);
	gtk_label_set_text(GTK_LABEL (img->total_time_data),time);
	g_free(time);
}

void img_start_stop_preview(GtkWidget *button, img_window_struct *img)
{
	GtkTreeIter iter, prev;
	GtkTreePath *path = NULL;
	slide_struct *entry;
	GtkTreeModel *model;
	GList *list = NULL;

	/* If no images are present, abort */
	if( img->slides_nr == 0 )
		return;

	if(img->export_is_running)
		return;

	if (img->preview_is_running)
	{
		/* Preview is already running */

		/* Remove timeout function from main loop */
		g_source_remove(img->source_id);

		/* Clean resources used by preview and prepare application for
		 * next preview. */
		img_clean_after_preview(img);
	}
	else
	{
		/* Start the preview */
		model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview));
		list = gtk_icon_view_get_selected_items( GTK_ICON_VIEW( img->thumbnail_iconview ) );
		if( list )
			gtk_icon_view_get_cursor(GTK_ICON_VIEW(img->thumbnail_iconview), &path, NULL);
		if( list && path )
		{
			/* Start preview from this slide */
			gtk_tree_model_get_iter( model, &iter, path );
			g_list_foreach( list, (GFunc)gtk_tree_path_free, NULL );
			g_list_free( list );
		}
		else
		{
			/* Start preview from the beginning */
			if(!gtk_tree_model_get_iter_first (model,&iter))
				return;
		}

		/* Replace button and menu images */
		img_swap_toolbar_images( img, FALSE );

		/* Store currently displayed image and then clear image_area.
		 * If the image is not cleared, the transition from*/
		img->slide_pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(img->image_area));
		if (img->slide_pixbuf)
			g_object_ref(G_OBJECT(img->slide_pixbuf));
		gtk_image_clear(GTK_IMAGE(img->image_area));

		/* Connect expose event to handler */
		gtk_widget_set_app_paintable(img->image_area, TRUE);
		g_signal_connect( G_OBJECT(img->image_area), "expose-event",G_CALLBACK(img_on_expose_event),img);

		/* Load the first image in the pixbuf */
		gtk_tree_model_get(model, &iter,1,&entry,-1);
		img->pixbuf2 = img_scale_pixbuf(img,entry->filename);
		img->current_slide = entry;

		/* If we started our preview from beginning, create empty pixbuf and
		 * fill it with background color. Else load image that is before
		 * currently selected slide. */
		if( path != NULL && gtk_tree_path_prev( path ) )
		{
			if( ! img->cur_ss_iter )
				img->cur_ss_iter = g_slice_new( GtkTreeIter );

			gtk_tree_model_get_iter( model, &prev, path );
			gtk_tree_model_get( model, &prev, 1, &entry, -1 );
			img->pixbuf1 = img_scale_pixbuf(img, entry->filename);
			*img->cur_ss_iter = iter;
		}
		else
		{
			img->pixbuf1 = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,img->image_area->allocation.width,img->image_area->allocation.height);
			gdk_pixbuf_fill(img->pixbuf1,img->background_color);
		}
		if( path )
			gtk_tree_path_free( path );

		/* Add transition timeout function */
		img->preview_is_running = TRUE;
		img->progress = 0;
		img->source_id = g_timeout_add(1000 / PREVIEW_FPS,(GSourceFunc)img_transition_timeout,img);
	}
	return;
}

void img_goto_first_slide(GtkWidget *button, img_window_struct *img)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	gchar *slide = NULL;

	model = gtk_icon_view_get_model( GTK_ICON_VIEW( img->thumbnail_iconview ) );
	if ( ! gtk_tree_model_get_iter_first(model,&iter))
		return;

	slide = g_strdup_printf("%d", 1);
	gtk_entry_set_text(GTK_ENTRY(img->slide_number_entry), slide);
	g_free(slide);
	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
	path = gtk_tree_path_new_from_indices(0,-1);
	gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->thumbnail_iconview), path, NULL, FALSE);
	gtk_icon_view_select_path (GTK_ICON_VIEW (img->thumbnail_iconview), path);
	gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->thumbnail_iconview), path, FALSE, 0, 0);
	gtk_tree_path_free (path);
}

void img_goto_prev_slide(GtkWidget *button, img_window_struct *img)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GList *icons_selected = NULL;
	gchar *slide = NULL;
	gint slide_nr;

	icons_selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(img->thumbnail_iconview) );
	if( ! icons_selected )
		return;

	model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview) );
	slide_nr = gtk_tree_path_get_indices(icons_selected->data)[0];

	if (slide_nr == 0)
		return;

	slide = g_strdup_printf("%d", slide_nr);
	gtk_entry_set_text(GTK_ENTRY(img->slide_number_entry), slide);
	g_free(slide);
	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
	path = gtk_tree_path_new_from_indices(--slide_nr,-1);

	gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->thumbnail_iconview), path, NULL, FALSE);
	gtk_icon_view_select_path (GTK_ICON_VIEW (img->thumbnail_iconview), path);
	gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->thumbnail_iconview), path, FALSE, 0, 0);
	gtk_tree_path_free (path);

	g_list_foreach (icons_selected, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (icons_selected);
}

void img_goto_next_slide(GtkWidget *button, img_window_struct *img)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GList *icons_selected = NULL;
	gchar *slide = NULL;
	gint slide_nr;

	icons_selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(img->thumbnail_iconview) );
	if( ! icons_selected )
		return;

	/* Now get previous iter :) */
	model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview) );
	slide_nr = gtk_tree_path_get_indices(icons_selected->data)[0];

	if (slide_nr == (img->slides_nr-1) )
		return;

	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
	path = gtk_tree_path_new_from_indices(++slide_nr,-1);

	slide = g_strdup_printf("%d", slide_nr+1);
	gtk_entry_set_text(GTK_ENTRY(img->slide_number_entry), slide);
	g_free(slide);
	gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->thumbnail_iconview), path, NULL, FALSE);
	gtk_icon_view_select_path (GTK_ICON_VIEW (img->thumbnail_iconview), path);
	gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->thumbnail_iconview), path, FALSE, 0, 0);
	gtk_tree_path_free (path);

	g_list_foreach (icons_selected, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (icons_selected);
}


void img_goto_last_slide(GtkWidget *button, img_window_struct *img)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	gchar *slide = NULL;

	model = gtk_icon_view_get_model( GTK_ICON_VIEW( img->thumbnail_iconview ) );
	if ( ! gtk_tree_model_get_iter_first(model,&iter))
		return;

	slide = g_strdup_printf("%d", img->slides_nr);
	gtk_entry_set_text(GTK_ENTRY(img->slide_number_entry), slide);
	g_free(slide);
	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
	path = gtk_tree_path_new_from_indices(img->slides_nr - 1, -1);
	gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->thumbnail_iconview), path, NULL, FALSE);
	gtk_icon_view_select_path (GTK_ICON_VIEW (img->thumbnail_iconview), path);
	gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->thumbnail_iconview), path, FALSE, 0, 0);
	gtk_tree_path_free (path);
}

void img_on_drag_data_received (GtkWidget *widget,GdkDragContext *context,int x,int y,GtkSelectionData *data,unsigned int info,unsigned int time, img_window_struct *img)
{
	gchar **pictures = NULL;
	gchar *filename;
	GtkWidget *dialog;
	GdkPixbuf *thumb;
	GtkTreeIter iter;
	gint len = 0, slides_cnt = 0;
	slide_struct *slide_info;

	pictures = gtk_selection_data_get_uris(data);
	if (pictures == NULL)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(img->imagination_window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("Sorry,I could not perform the operation!"));
		gtk_window_set_title(GTK_WINDOW(dialog),"Imagination");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
		gtk_drag_finish(context,FALSE,FALSE,time);
		return;
	}
	gtk_drag_finish (context,TRUE,FALSE,time);
	while(pictures[len])
	{
		filename = g_filename_from_uri (pictures[len],NULL,NULL);
		thumb = img_load_pixbuf_from_file(filename);
		if (thumb)
		{
			slide_info = img_set_slide_info(1, NORMAL, NULL, -1, "0", filename);
			if (slide_info)
			{
				gtk_list_store_append (img->thumbnail_model,&iter);
				gtk_list_store_set (img->thumbnail_model, &iter, 0, thumb, 1, slide_info, -1);
				g_object_unref (thumb);
				slides_cnt++;
			}
		}
		g_free(filename);
		len++;
	}
	if (slides_cnt > 0)
	{
		img->slides_nr += slides_cnt;
		img->project_is_modified = TRUE;
		img_set_total_slideshow_duration(img);
		img_set_statusbar_message(img, 0);
	}
	g_strfreev (pictures);
}

gboolean img_on_expose_event(GtkWidget *widget,GdkEventExpose *event,img_window_struct *img)
{
	/* We always pass negative number as a last parameter when we want to
	 * draw on screen. */
	if ((img->current_slide)->render)
		(img->current_slide)->render (widget->window, img->pixbuf1, img->pixbuf2,img->progress, -1);
	else
	{
		/* This is "None" transition renderer */
		cairo_t *cr;
		gint     offset_x, offset_y, width, height;

		gdk_drawable_get_size(widget->window, &width, &height);
		offset_x = (width  - gdk_pixbuf_get_width (img->pixbuf2)) / 2;
		offset_y = (height - gdk_pixbuf_get_height(img->pixbuf2)) / 2;
		
		cr = gdk_cairo_create(widget->window);
		gdk_cairo_set_source_pixbuf(cr,img->pixbuf2,offset_x,offset_y);
		cairo_paint(cr);
		
		cairo_destroy(cr);
	}
	return FALSE;
}

GdkPixbuf *img_scale_pixbuf(img_window_struct *img, gchar *filename)
{
	GdkPixbuf *pixbuf, *compose;
	gint       i_width, i_height;		/* Image dimensions */
	gint       a_width, a_height;		/* Display dimensions */
	gint       offset_x, offset_y;
	gdouble    a_ratio, i_ratio;
	gdouble    max_stretch = 0.1280;	/* Maximum amount of stretch */
	gdouble    max_crop    = 0.8500;	/* Maximum amount of crop */
	gboolean   too_small;

	/* Obtaint information about display area */
	a_width  = img->image_area->allocation.width;
	a_height = img->image_area->allocation.height;
	a_ratio  = (gdouble)a_width / a_height;

	/* Obtain information about image being loaded */
	gdk_pixbuf_get_file_info( filename, &i_width, &i_height );
	i_ratio = (gdouble)i_width / i_height;

	/* If the image is smaller than display area, just center it and fill
	 * the background with color. We do approximatelly the same thing if
	 * the user doesn't want to distort images. */
	too_small = ( i_width < a_width && i_height < a_height );
	if( ( ! img->distort_images ) || too_small )
	{
		if( too_small )
			pixbuf = gdk_pixbuf_new_from_file( filename, NULL );
		else
			pixbuf = gdk_pixbuf_new_from_file_at_size( filename, a_width, a_height, NULL );

		i_width  = gdk_pixbuf_get_width( pixbuf );
		i_height = gdk_pixbuf_get_height( pixbuf );
		offset_x = ( a_width  - i_width  ) / 2;
		offset_y = ( a_height - i_height ) / 2;
		compose = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8,
								  a_width, a_height );
		gdk_pixbuf_fill( compose, img->background_color );
		gdk_pixbuf_composite( pixbuf, compose, offset_x, offset_y, i_width,
							  i_height, offset_x, offset_y, 1, 1,
							  GDK_INTERP_BILINEAR, 255 );
		g_object_unref( G_OBJECT( pixbuf ) );
		return( compose );
	}

	/* If the user doesn't want to have images distorted, we only scale
	 * image. */

	/* If we are here, the image is too big for display area, so we need to
	 * scale it. Depending on values of i_ratio and a_ratio, we'll do some
	 * transformations to, but won't distort the image for more than 6.25%
	 * of it's size (just enough to snuggly convert 4:3 image to PAL) or crop
	 * it more than 10%. */

	if( i_ratio < a_ratio )
	{
		if( i_ratio > a_ratio * ( 1 - max_stretch ) )
		{
			/* We can shrink image vertically enough to fit. */
			return( gdk_pixbuf_new_from_file_at_scale( filename, a_width, a_height, FALSE, NULL ) );
		}
		else
		{
			if( i_ratio > a_ratio * ( 1 - max_stretch ) * max_crop )
			{
				/* We can shrink image vertically and crop it to fit. */
				pixbuf = gdk_pixbuf_new_from_file_at_scale(
							filename, a_width, a_height * ( 1 + max_stretch ), FALSE, NULL );
			}
			else
			{
				/* We cannot avoid white stripes on the left/right. Sorry. */
				pixbuf = gdk_pixbuf_new_from_file_at_size( filename, a_width, a_height, NULL );
			}
		}
	}
	else
	{
		if( i_ratio < a_ratio * ( 1 + max_stretch ) )
		{
			/* We can shrink image horizontally enough to fit. */
			return( gdk_pixbuf_new_from_file_at_scale( filename, a_width, a_height, FALSE, NULL ) );
		}
		else
		{
			if( i_ratio < a_ratio * ( 1 + max_stretch ) / max_crop )
			{
				/* We can shrink image horizontally and crop it to fit. */
				pixbuf = gdk_pixbuf_new_from_file_at_scale(
							filename, a_width * ( 1 + max_stretch ), a_height, FALSE, NULL );
			}
			else
			{
				/* We cannot avoid white stripes at the bottom/top. Sorry. */
				pixbuf = gdk_pixbuf_new_from_file_at_size( filename, a_width, a_height, NULL );
			}
		}
	}

	/* Do composing */
	i_width  = gdk_pixbuf_get_width( pixbuf );
	i_height = gdk_pixbuf_get_height( pixbuf );
	offset_x = ( a_width - i_width ) / 2;
	offset_y = ( a_height - i_height ) / 2;

	compose = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, a_width, a_height );
	gdk_pixbuf_fill( compose, img->background_color );
	gdk_pixbuf_composite( pixbuf, compose,
						  offset_x < 0 ? 0 : offset_x,
						  offset_y < 0 ? 0 : offset_y,
						  i_width < a_width ? i_width : a_width,
						  i_height < a_height ? i_height : a_height,
						  offset_x, offset_y, 1, 1,
						  GDK_INTERP_BILINEAR, 255 );
	g_object_unref( G_OBJECT( pixbuf ) );

	return( compose );
}

static gboolean img_transition_timeout(img_window_struct *img)
{
	/* Increment progress variable (this is being passed as a parameter
	 * to plug-in provided transition function).
	 * 1000 ms / TRANSITION_TIMEOUT gives us a frame rate of preview
	 * (currently, this is 1000 / 40 = 25 fps). */
	img->progress += (gdouble)1 / ( img->current_slide->speed * PREVIEW_FPS );
	
	/* if the transition render is NULL (no transition is set for this
	 * slide), we stop this timeout function, but still invalidate
	 * preview area so expose event gets called. */
	if( img->current_slide->render == NULL )
	{
		img->progress = 0;
		img->source_id = g_timeout_add( img->current_slide->duration * 1000, (GSourceFunc)img_sleep_timeout, img );
		gtk_widget_queue_draw( img->image_area );
		return FALSE;
	}

	/* If the progress reached 1, the transition should be finished and
	 * it's time to stop this timeout function and add sleep timeout
	 * function to the main loop. The 0.0000005 is added to accomodate
	 * floating number rounding errors (if we would write this condition
	 * as img->progress > 1, we would miss the last frame, since
	 * (gdouble)1 > 1 */
	if( img->progress > 1.0000005 )
	{
		img->progress = 0;
		img->source_id = g_timeout_add( img->current_slide->duration * 1000,(GSourceFunc)img_sleep_timeout, img );
		return FALSE;
	}

	/* Schedule our image redraw */
	gtk_widget_queue_draw( img->image_area );

	return TRUE;
}

static gboolean img_sleep_timeout(img_window_struct *img)
{
	if(img_prepare_pixbufs(img))
	{
		img->source_id = g_timeout_add( 1000 / PREVIEW_FPS,(GSourceFunc)img_transition_timeout,img );
	}
	else
	{
		/* Clean resources used in preview and prepare application for
		 * next preview. */
		img_clean_after_preview(img);
	}

	return FALSE;
}

static void img_swap_toolbar_images( img_window_struct *img,gboolean flag )
{
	GtkWidget *tmp_image;

	if( flag )
	{
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (img->preview_menu),tmp_image);
		
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_widget_show(tmp_image);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(img->preview_button), tmp_image);
		gtk_widget_set_tooltip_text(img->preview_button,_("Starts the preview"));
	}
	else
	{
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_STOP,GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (img->preview_menu),tmp_image);
		
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_STOP,GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_widget_show(tmp_image);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(img->preview_button), tmp_image);
		gtk_widget_set_tooltip_text(img->preview_button,_("Stops the preview"));
	}
}

static void img_clean_after_preview(img_window_struct *img)
{
	/* Disconnect expose event handler */
	g_signal_handlers_disconnect_by_func(img->image_area,img_on_expose_event,img);
	gtk_widget_set_app_paintable(img->image_area, FALSE);

	/* Restore image that was used before preview */
	gtk_image_set_from_pixbuf(GTK_IMAGE(img->image_area), img->slide_pixbuf);
	if( img->slide_pixbuf )
		g_object_unref(G_OBJECT(img->slide_pixbuf));

	/* Swap toolbar and menu icons */
	img_swap_toolbar_images( img, TRUE );

	/* Indicate that preview is not running */
	img->preview_is_running = FALSE;

	/* Clean the resources used by timeout handlers */
	g_slice_free( GtkTreeIter, img->cur_ss_iter );
	img->cur_ss_iter = NULL;

	return;
}

void img_choose_slideshow_filename(GtkWidget *widget, img_window_struct *img)
{
	GtkWidget *fc;
	GtkFileChooserAction action = 0;
	gint response;

	/* Determine the mode of the chooser. */
	if (widget == img->open_menu || widget == img->open_button)
		action = GTK_FILE_CHOOSER_ACTION_OPEN;
	else if (widget == img->save_as_menu || widget == img->save_menu || widget == img->save_button)
		action = GTK_FILE_CHOOSER_ACTION_SAVE;

	/* If user wants to save empty slideshow, simply abort */
	if( img->slides_nr == 0 && action == GTK_FILE_CHOOSER_ACTION_SAVE  )
		return;

	if (img->project_filename == NULL || widget == img->save_as_menu || action == GTK_FILE_CHOOSER_ACTION_OPEN)
	{
		fc = gtk_file_chooser_dialog_new (action == GTK_FILE_CHOOSER_ACTION_OPEN ? _("Load an Imagination slideshow project") : 
					_("Save an Imagination slideshow project"),
					GTK_WINDOW (img->imagination_window),
					action,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL,
					action == GTK_FILE_CHOOSER_ACTION_OPEN ?  GTK_STOCK_OPEN : GTK_STOCK_SAVE,
					GTK_RESPONSE_ACCEPT,NULL);

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (fc),TRUE);
		response = gtk_dialog_run (GTK_DIALOG (fc));
		if (response == GTK_RESPONSE_ACCEPT)
		{
			gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
			if(filename)
			{
				/* Free any previous filename */
				if(img->project_filename)
					g_free(img->project_filename);
				
				/* Store new filename */
				img->project_filename = filename;
				gtk_widget_destroy(fc);
			}
		}
		else if (response == GTK_RESPONSE_CANCEL || GTK_RESPONSE_DELETE_EVENT)
		{
			gtk_widget_destroy(fc);
			return;
		}
	}
	if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
		img_load_slideshow(img);
	else
		img_save_slideshow(img);
}

void img_close_slideshow(GtkWidget *widget, img_window_struct *img)
{
	if (img->project_is_modified)
	{
		if (GTK_RESPONSE_OK != img_ask_user_confirmation(img, _("You didn't save your slideshow yet. Are you sure you want to close it?")))
			return;
	}
	img->project_is_modified = FALSE;
	img_free_allocated_memory(img);
	img_set_window_title(img,NULL);
	img_set_statusbar_message(img,0);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img->image_area),NULL);
	gtk_widget_set_sensitive(img->random_button, FALSE);
	gtk_widget_set_sensitive(img->transition_type, FALSE);
	gtk_widget_set_sensitive(img->duration, FALSE);
	gtk_label_set_text(GTK_LABEL (img->total_time_data),"");

	/* Reset slideshow properties */
	img->distort_images = TRUE;
	img->background_color = 0;
	img->final_transition.speed = NORMAL;
	img->final_transition.render = NULL;
}

/*
// In GdkPixbuf 2.12 or above, this returns the EXIF orientation value.
const char* exif_orientation = gdk_pixbuf_get_option(thumb->pixbuf, "orientation");
if (exif_orientation != NULL) {
switch (exif_orientation[0]) {
case '3':
thumb->rotation = 180;
          break;
       case '6':
         thumb->rotation = 270;
          break;
       case '8':
         thumb->rotation = 90;
          break;
       // '1' means no rotation.  The other four values are all various
          transpositions, which are rare in real photos so we don't
          implement them. 
      }
    }
  }

  // Rotate if necessary 
  if (thumb->rotation != 0)
  {
      GdkPixbuf *new_pixbuf = gdk_pixbuf_rotate_simple (thumb->pixbuf, thumb->rotation);
      g_object_unref (thumb->pixbuf);
      thumb->pixbuf = new_pixbuf;
      // Clean up 
      thumb->rotation = 0;
 }
 */

void img_move_audio_up( GtkButton *button, img_window_struct *img )
{
	GtkTreeSelection *sel;
	GtkTreeModel     *model;
	GtkTreeIter       iter1, iter2;

	/* We need path, since there is no gtk_tree_model_iter_prev function!! */
	GtkTreePath      *path;

	/* First we need to get selected iter. This function won't work if
	 * selection's mode is set to GTK_SELECTION_MULTIPLE!! */
	sel = gtk_tree_view_get_selection( GTK_TREE_VIEW( img->music_file_treeview ) );
	if( ! gtk_tree_selection_get_selected( sel, &model, &iter1 ) )
		return;

	/* Now get previous iter and swap two items if previous iter exists. */
	path = gtk_tree_model_get_path( model, &iter1 );
	if( gtk_tree_path_prev( path ) )
	{
		gtk_tree_model_get_iter( model, &iter2, path );
		gtk_list_store_swap( GTK_LIST_STORE( model ), &iter1, &iter2 );
	}
	gtk_tree_path_free( path );
}

void img_move_audio_down( GtkButton *button, img_window_struct *img )
{
	GtkTreeSelection *sel;
	GtkTreeModel     *model;
	GtkTreeIter       iter1, iter2;

	/* First we need to get selected iter. This function won't work if
	 * selection's mode is set to GTK_SELECTION_MULTIPLE!! */
	sel = gtk_tree_view_get_selection( GTK_TREE_VIEW( img->music_file_treeview ) );
	if( ! gtk_tree_selection_get_selected( sel, &model, &iter1 ) )
		return;

	/* Get next iter and swap rows if iter exists. */
	iter2 = iter1;
	if( gtk_tree_model_iter_next( model, &iter2 ) )
		gtk_list_store_swap( GTK_LIST_STORE( model ), &iter1, &iter2 );
}
