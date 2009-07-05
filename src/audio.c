/*
 *  Copyright (c) 2009 Giuseppe Torelli <colossus73@gmail.com>
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

#include "audio.h"

static gchar *img_get_audio_filetype(gchar *);
static void img_play_audio_ended (GPid ,gint ,img_window_struct *);
static void img_swap_audio_files_button(img_window_struct *, gboolean );

static gchar *img_get_audio_filetype(gchar *filename)
{
	if (g_str_has_suffix(filename, ".mp3") || g_str_has_suffix(filename, ".MP3"))
		return "mp3";
	else if (g_str_has_suffix(filename, ".ogg") || g_str_has_suffix(filename, ".OGG"))
		return "ogg";
	else if (g_str_has_suffix(filename, ".flac") || g_str_has_suffix(filename, ".FLAC"))
		return "flac";
	else if (g_str_has_suffix(filename, ".wav") || g_str_has_suffix(filename, ".WAV"))
		return "wav";

	return NULL;
}

gchar *img_get_audio_length(img_window_struct *img, gchar *filename, gint *secs)
{
	gint seconds = -1;
	gchar *filetype = NULL;
	sox_format_t *ft;

	filetype = img_get_audio_filetype(filename);
	
	ft = sox_open_read(filename, NULL, NULL, filetype);

	if (ft != NULL)
	{
		seconds = (ft->signal.length / ft->signal.channels) / ft->signal.rate;
		sox_close(ft); 
	}

	*secs = seconds;
	return seconds == -1 ? NULL : img_convert_seconds_to_time(*secs);
}

void img_play_stop_selected_file(GtkButton *button, img_window_struct *img)
{
	GError *error = NULL;
	gchar	*cmd_line, *path, *filename, *file, *message;
	gchar 	**argv;
	gint argc;
	gboolean ret;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (img->play_child_pid)
	{
		kill (img->play_child_pid, SIGINT);
		/* This is not required on unices, but per docs it should be called
		 * anyway. */
		g_spawn_close_pid( img->play_child_pid );
		img_swap_audio_files_button(img, TRUE);
		return;
	}
	if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(img->music_file_treeview)), &model, &iter) == FALSE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(img->music_file_liststore), &iter, 0, &path, 1, &filename, -1);

	file = g_build_filename(path, filename, NULL);
	g_free(path);
	g_free(filename);

	cmd_line = g_strdup_printf("play -t %s '%s'", img_get_audio_filetype(file), file);
	g_print ("%s\n",cmd_line);

	g_shell_parse_argv (cmd_line, &argc, &argv, NULL);
	g_free(cmd_line);

	ret = g_spawn_async_with_pipes( NULL, argv, NULL,
									G_SPAWN_SEARCH_PATH | 
									G_SPAWN_DO_NOT_REAP_CHILD | 
									G_SPAWN_STDOUT_TO_DEV_NULL | 
									G_SPAWN_STDERR_TO_DEV_NULL,
									NULL, NULL, &img->play_child_pid, NULL, NULL, NULL, &error );

	/* Free argument vector */
	g_strfreev( argv );

	g_child_watch_add(img->play_child_pid, (GChildWatchFunc) img_play_audio_ended, img);

	img_swap_audio_files_button(img, FALSE);
	message = g_strdup_printf(_("Playing %s..."), file);
	gtk_statusbar_push(GTK_STATUSBAR(img->statusbar), img->context_id, message);
	g_free(file);
	g_free(message);
}

static void img_play_audio_ended (GPid pid, gint status, img_window_struct *img)
{
	g_spawn_close_pid( img->play_child_pid );
	img_swap_audio_files_button (img, TRUE);
}

static void img_swap_audio_files_button(img_window_struct *img, gboolean flag)
{
	GtkWidget *tmp_image;

	if (flag)
	{
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_MENU);
		gtk_button_set_image(GTK_BUTTON(img->play_audio_button), tmp_image);
		gtk_widget_set_tooltip_text(img->play_audio_button, _("Play the selected file"));
		gtk_statusbar_pop(GTK_STATUSBAR(img->statusbar), img->context_id);
		img->play_child_pid = 0;
	}
	else
	{
		tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_STOP,GTK_ICON_SIZE_MENU);
		gtk_button_set_image(GTK_BUTTON(img->play_audio_button), tmp_image);
		gtk_widget_set_tooltip_text(img->play_audio_button, _("Stop the playback"));
	}
}

void output_message(unsigned level, const char *filename, const char *fmt, va_list ap)
{
	GtkWidget *dialog;
	gchar *string;

	if (level == 1)
	{
		string = g_strdup_vprintf(fmt,ap);
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, string);
		gtk_window_set_title(GTK_WINDOW(dialog), "Imagination");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));

		g_free(string);
	}
}
