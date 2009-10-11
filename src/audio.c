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

	path = g_shell_quote( file );

	cmd_line = g_strdup_printf("play -t %s %s", img_get_audio_filetype(file), path);
	g_free( path );

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
	g_free( file );
	gtk_statusbar_push(GTK_STATUSBAR(img->statusbar), img->context_id, message);
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
	gchar *string;

	if (level == 1)
	{
		string = g_strdup_vprintf(fmt,ap);
		g_message( "%s", string );
		g_free(string);
	}
}

/* ****************************************************************************
 * EXPORT AUDIO
 * ************************************************************************* */
void
img_analyze_input_files( gchar   **inputs,
						 gint      no_inputs,
						 gdouble  *rate,
						 gint     *channels )
{
	gint    i, j, tmp = 1;
	GArray *array_ra = g_array_sized_new( FALSE, FALSE, sizeof( gdouble ), 10 );
	GArray *array_rc = g_array_sized_new( FALSE, FALSE, sizeof( gint ), 10 );
	GArray *array_ch = g_array_sized_new( FALSE, FALSE, sizeof( gdouble ), 2 );
	GArray *array_cc = g_array_sized_new( FALSE, FALSE, sizeof( gint ), 2 );

	/* Analyze all files */
	for( i = 0; i < no_inputs; i++ )
	{
		sox_format_t *in = sox_open_read( inputs[i], NULL, NULL, NULL );

		/* Get rate setting and increment counter if this rate is already
		 * present in array. */
		for( j = 0; j < array_ra->len; j++ )
			if( *( (gdouble *)( array_ra->data ) + j ) == in->signal.rate )
				break;
		if( j == array_ra->len )
		{
			g_array_append_val( array_ra, in->signal.rate );
			g_array_append_val( array_rc, tmp );
		}
		else
		{
			*( (gint *)( array_rc->data ) + j ) += 1;
		}

		/* Get channels and increment counter if this number is
		 * already present */
		for( j = 0; j < array_ch->len; j++ )
			if( *( (gint *)( array_ch->data ) + j ) == in->signal.channels )
				break;
		if( j == array_ch->len )
		{
			g_array_append_val( array_ch, in->signal.channels );
			g_array_append_val( array_cc, tmp );
		}
		else
		{
			*( (gint *)( array_cc->data ) + j ) += 1;
		}

		sox_close( in );
	}

	/* Do some statistics */
	for( i = 0, j = 0, tmp = 0; i < array_ra->len; i++ )
	{
		if( *( (gint *)( array_rc->data ) + i ) > tmp )
		{
			tmp = *( (gint *)( array_rc->data ) + i );
			j = i;
		}
	}
	*rate = *( (gdouble *)( array_ra->data ) + j );

	for( i = 0, j = 0, tmp = 0; i < array_ch->len; i++ )
	{
		if( *( (gint *)( array_cc->data ) + i ) > tmp )
		{
			tmp = *( (gint *)( array_cc->data ) + i );
			j = i;
		}
	}
	*channels = *( (gint *)( array_ch->data ) + j );

	/* Free data storage */
	g_array_free( array_ra, TRUE );
	g_array_free( array_rc, TRUE );
	g_array_free( array_ch, TRUE );
	g_array_free( array_cc, TRUE );
}

gboolean
img_eliminate_bad_files( gchar             **inputs,
						 gint                no_inputs,
						 gdouble             rate,
						 gint                channels,
						 img_window_struct  *img )
{
	gint       i, j, reduced_out = no_inputs;
	GString   *string;
	gboolean   warn = FALSE, ret = TRUE;
	GtkWidget *dialog;

	string = g_string_new( "" );

	/* Analyze all files */
	for( i = 0, j = 0; i < no_inputs; i++ )
	{
		sox_format_t *in = sox_open_read( inputs[i], NULL, NULL, NULL );
		if( in->signal.rate != rate || in->signal.channels != channels )
		{
			gchar *base = g_path_get_basename( inputs[i] );
			gint   bad = 0;
			
			bad += ( in->signal.rate != rate ? 1 : 0 );
			bad += ( in->signal.channels != channels ? 2 : 0 );

			switch( bad )
			{
				case 1: /* Incompatible signal rate */
					g_string_append_printf(
							string, _("%s:\nincompatible sample rate\n"), base );
					break;

				case 2: /* Incompatible number of channels */
					g_string_append_printf(
							string, _("%s:\nincompatible number of channels\n"),
									base );
					break;

				case 3: /* Both are incompatible */
					g_string_append_printf(
							string, _("%s:\nincompatible sample rate and "
									"number of channels\n"), base );
					break;
			}
			g_free( inputs[i] );
			g_free( base );
			inputs[i] = NULL;
			reduced_out--;
			warn = TRUE;
		}
	}

	/* Present results to user */
	if( warn )
	{
		dialog = gtk_message_dialog_new_with_markup(
							GTK_WINDOW( img->imagination_window ),
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_WARNING,
							GTK_BUTTONS_YES_NO,
							_("%s\n<b>Do you want to continue without "
							"these files?</b>"),
							string->str );

		gtk_window_set_title(GTK_WINDOW(dialog), _("Audio files mismatch:") );
		if( GTK_RESPONSE_YES != gtk_dialog_run( GTK_DIALOG( dialog ) ) )
			ret = FALSE;
		gtk_widget_destroy( dialog );
	}
	g_string_free( string, TRUE );

	/* If we continue with export process, set up proper audio files array */
	if( ret )
	{
		img->exported_audio = g_slice_alloc( sizeof( gchar * ) * reduced_out );
		img->exported_audio_no = reduced_out;

		for( i = 0, j = 0; i < no_inputs; i++ )
			if( inputs[i] )
				img->exported_audio[j++] = inputs[i];
	}

	return( ret );
}

void
img_update_inc_audio_display( img_window_struct *img )
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	gchar        *inputs[100]; /* 100 audio files is current limit */
	gint          i = 0;
	gint          channels;
	gdouble       rate;
	gint          warn = 0;
	gint          total_time = 0;


	model = gtk_tree_view_get_model( GTK_TREE_VIEW( img->music_file_treeview ) );

	/* If no audio is present, simply return */
	if( gtk_tree_model_get_iter_first( model, &iter ) )
	{
		gchar *path, *filename;

		do
		{
			gtk_tree_model_get( model, &iter, 0, &path, 1, &filename, -1 );
			inputs[i] = g_strdup_printf( "%s%s%s", path,
										 G_DIR_SEPARATOR_S, filename );
			i++;
			g_free( path );
			g_free( filename );
		}
		while( gtk_tree_model_iter_next( model, &iter ) );
	}
	else
		return;

	img_analyze_input_files( inputs, i, &rate, &channels );

	/* Update display */
	gtk_tree_model_get_iter_first( model, &iter );
	do
	{
		gchar *path, *file;
		gchar *full;
		gint   bad = 0;
		gint   duration;

		gtk_tree_model_get( model, &iter, 0, &path,
										  1, &file,
										  3, &duration,
										  -1 );
		full = g_strdup_printf( "%s%s%s", path, G_DIR_SEPARATOR_S, file );
		g_free( path );
		g_free( file );

		sox_format_t *in = sox_open_read( full, NULL, NULL, NULL );
			
		bad += ( in->signal.rate != rate ? 1 : 0 );
		bad += ( in->signal.channels != channels ? 2 : 0 );
		if( bad )
			warn++;

		switch( bad )
		{
			case 0: /* File is compatible */
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter,
									4, NULL,
									5, NULL,
									-1 );
				total_time += duration;
				break;

			case 1: /* Incompatible signal rate */
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter,
									4, "red",
									5, _("Incompatible sample rate."),
									-1 );
				break;

			case 2: /* Incompatible number of channels */
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter,
									4, "blue",
									5, _("Incompatible number of channels."),
									-1 );
				break;

			case 3: /* Both are incompatible */
				gtk_list_store_set( GTK_LIST_STORE( model ), &iter,
									4, "orange",
									5, _("Incompatible sample rate and "
										 "number of channels."),
									-1 );
				break;
		}
		sox_close( in );
		g_free( full );
	}
	while( gtk_tree_model_iter_next( model, &iter ) );

	/* Update total audio length */
	img->total_music_secs = total_time;

	/* Inform user that some files are incompatible and cannot be concatenated
	 * for export. */
	if( warn )
	{
		GtkWidget *dialog;
		gchar     *message,
				  *count;

		count = g_strdup_printf( 
					ngettext( "File selection contains %d audio file that "
							  "is incompatible with other files.",
							  "File selection contains %d audio files that "
							  "are incompatible with other files.",
							  warn ), warn );
		message = g_strconcat( count, "\n\n",
							  _("Please check audio tab for more information."),
							  NULL );
		g_free( count );

		dialog = gtk_message_dialog_new_with_markup(
							GTK_WINDOW( img->imagination_window ),
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_WARNING,
							GTK_BUTTONS_OK,
							"%s", message );

		gtk_window_set_title(GTK_WINDOW(dialog), _("Audio files mismatch:") );
		g_free( message );
		gtk_dialog_run( GTK_DIALOG( dialog ) );
		gtk_widget_destroy( dialog );
	}
}


