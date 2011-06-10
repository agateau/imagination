/*
** Copyright (c) 2009 Giuseppe Torelli <colossus73@gmail.com>
** Copyright (C) 2009 Tadej Borovšak   <tadeboro@gmail.com>
** Copyright (c) 2011 Robert Chéramy   <robert@cheramy.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "export.h"
#include "support.h"
#include "callbacks.h"
#include "audio.h"
#include "img_sox.h"
#include <fcntl.h>
#include <glib/gstdio.h>

static GtkWidget *
img_create_export_dialog( img_window_struct  *img,
						  const gchar        *title,
						  GtkWindow          *parent,
						  GtkEntry          **entry,
						  GtkWidget         **box );

static gboolean
img_prepare_audio( img_window_struct *img );

static gboolean
img_start_export( img_window_struct *img );

static gboolean
img_run_encoder( img_window_struct *img );

static gboolean
img_export_transition( img_window_struct *img );

static gboolean
img_export_transition( img_window_struct *img );

static gboolean
img_export_still( img_window_struct *img );

static void
img_export_pause_unpause( GtkToggleButton   *button,
						  img_window_struct *img );

static void
img_export_frame_to_ppm( cairo_surface_t *surface,
						 gint             file_desc );

/*
 * img_create_export_dialog:
 * @title: title to display with dialog
 * @parent: parent window of this dialog
 * @box: a pointer to set to GtkVBox, or NULL
 *
 * This is convenience function used to create base for every export dialog. It
 * returns GtkDialog that should be displayed using gtk_dialog_run.
 *
 * If parameter passed in is not NULL, it will be filled with GtkVBox that can
 * be used to add optional content (this is just a convience, since accessing
 * content area of dialog box is somewhat complicated and depends on the GTK+
 * version being used).
 *
 * Aditionally, it also performs checking to avoid export being called more that
 * once at any time or along side preview.
 *
 * Return value: newly created GtkDialog or NULL if export is already running.
 */
static GtkWidget *
img_create_export_dialog( img_window_struct  *img,
						  const gchar        *title,
						  GtkWindow          *parent,
						  GtkEntry          **entry,
						  GtkWidget         **box )
{
	GtkWidget    *dialog;
	GtkWidget    *vbox, *vbox1, *hbox_slideshow_name;
	GtkWidget    *vbox_frame1, *main_frame, *alignment_main_frame;
	GtkWidget    *label, *label1;
	GtkWidget    *slideshow_title_entry;
	GtkTreeModel *model;
	GtkTreeIter   iter;

	/* Abort if preview is running */
	if( img->preview_is_running )
		return( NULL );

	/* Abort if export is running */
	if( img->export_is_running )
		return( NULL );

	/* Switch mode */
	if( img->mode == 1 )
	{
		img->auto_switch = TRUE;
		img_switch_mode( img, 0 );
	}

	/* Abort if no slide is present */
	model = GTK_TREE_MODEL( img->thumbnail_model );
	if( ! gtk_tree_model_get_iter_first( model, &iter ) )
	{
		return( NULL );
	}

	/* Create dialog */
	dialog = gtk_dialog_new_with_buttons( title, parent,
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										  GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
										  NULL );

	gtk_button_box_set_layout (GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area), GTK_BUTTONBOX_SPREAD);
	gtk_window_set_default_size(GTK_WINDOW(dialog),520,-1);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

#if GTK_CHECK_VERSION( 2, 14, 0 )
	vbox = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );
#else
	vbox = GTK_DIALOG( dialog )->vbox;
#endif

	vbox1 = gtk_vbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
	gtk_box_pack_start (GTK_BOX (vbox), vbox1, TRUE, TRUE, 0);

	main_frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox1), main_frame, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (main_frame), GTK_SHADOW_IN);

	label1 = gtk_label_new (_("<b>Export Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (main_frame), label1);
	gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);

	alignment_main_frame = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (main_frame), alignment_main_frame);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_main_frame), 5, 15, 10, 10);

	vbox_frame1 = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( alignment_main_frame ), vbox_frame1 );

	hbox_slideshow_name = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_frame1), hbox_slideshow_name, TRUE, TRUE, 0);

	label = gtk_label_new( _("Filename:") );
	gtk_box_pack_start( GTK_BOX( hbox_slideshow_name ), label, FALSE, TRUE, 0 );
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	slideshow_title_entry = sexy_icon_entry_new();
	sexy_icon_entry_add_clear_button( SEXY_ICON_ENTRY(slideshow_title_entry), img, img_show_file_chooser);
    sexy_icon_entry_set_icon_highlight( SEXY_ICON_ENTRY(slideshow_title_entry), SEXY_ICON_ENTRY_PRIMARY, TRUE);
	gtk_box_pack_start( GTK_BOX( hbox_slideshow_name ), slideshow_title_entry, TRUE, TRUE, 0 );

	if( box )
		*box = vbox_frame1;

	*entry = GTK_ENTRY( slideshow_title_entry );

	return( dialog );
}

/*
 * img_prepare_audio:
 * @img: global img_window_struct structure
 */
static gboolean
img_prepare_audio( img_window_struct *img )
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	gchar       **tmp;
	gchar        *inputs[100]; /* 100 audio files is current limit */
	gint          i = 0;
	gint          channels;
	gdouble       rate;


	/* Set the export info */
	img->export_is_running = 2;

	model = gtk_tree_view_get_model( GTK_TREE_VIEW( img->music_file_treeview ) );
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

	/* If no audio is present, simply update ffmpeg command line with -an */
	if( i == 0 )
	{
		/* Replace audio place holder */
		tmp = g_strsplit( img->export_cmd_line, "<#AUDIO#>", 0 );
		g_free( img->export_cmd_line );
		img->export_cmd_line = g_strjoin( NULL, tmp[0], "-an", tmp[1], NULL );

		/* Chain last export step - video export */
		g_idle_add( (GSourceFunc)img_start_export, img );

		return( FALSE );
	}

	img_analyze_input_files( inputs, i, &rate, &channels );
	if( img_eliminate_bad_files( inputs, i, rate, channels, img ) )
	{
		/* Thread data structure */
		ImgThreadData *tdata = g_slice_new( ImgThreadData );

		/* FIFO path */
		img->fifo = g_build_filename( g_get_tmp_dir(), "img_audio_fifo", NULL );

		/* Replace audio place holder */
		tmp = g_strsplit( img->export_cmd_line, "<#AUDIO#>", 0 );
		g_free( img->export_cmd_line );

		img->export_cmd_line = g_strdup_printf( "%s-f flac -i %s%s", tmp[0],
												img->fifo, tmp[1] );

		/* Fill thread structure with data */
		tdata->sox_flags = &img->sox_flags;
		tdata->files     =  img->exported_audio;
		tdata->no_files  =  img->exported_audio_no;
		tdata->length    =  img->total_secs;
		tdata->fifo      =  img->fifo;

		mkfifo( img->fifo, S_IRWXU );

		/* Spawn sox thread now. */
		g_atomic_int_set( &img->sox_flags, 0 );
		img->sox = g_thread_create( (GThreadFunc)img_produce_audio_data,
									tdata, TRUE, NULL );

		/* Chain last export step - video export */
		g_idle_add( (GSourceFunc)img_start_export, img );
	}
	else
	{
		/* User declined proposal */
		img_stop_export( img );
	}

	return( FALSE );
}

/*
 * img_start_export:
 * @img: global img_wndow_struct structure
 *
 * This function performs the last export step - spawns ffmpeg and initiates the
 * export progress indicators.
 *
 * Return value: Always returns FALSE, since we want it to be removed from main
 * context.
 */
static gboolean
img_start_export( img_window_struct *img )
{
	GtkTreeIter   iter;
	slide_struct *entry;
	GtkTreeModel *model;
	GtkWidget    *dialog;
	GtkWidget    *vbox, *hbox;
	GtkWidget    *label;
	GtkWidget    *progress;
	GtkWidget    *button;
	gchar        *string;
	cairo_t      *cr;

	/* Set export info */
	img->export_is_running = 3;

	/* Spawn ffmepg and abort if needed */
	if( ! img_run_encoder( img ) )
	{
		img_stop_export( img );
		return( FALSE );
	}

	/* Create progress window with cancel and pause buttons, calculate
	 * the total number of frames to display. */
	dialog = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	img->export_dialog = dialog;
	gtk_window_set_title( GTK_WINDOW( img->export_dialog ),
						  _("Exporting the slideshow") );
	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 10 );
	gtk_window_set_default_size( GTK_WINDOW( dialog ), 400, -1 );
	gtk_window_set_type_hint( GTK_WINDOW( dialog ), GDK_WINDOW_TYPE_HINT_DIALOG );
	gtk_window_set_modal( GTK_WINDOW( dialog ), TRUE );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ),
								  GTK_WINDOW( img->imagination_window ) );

	vbox = gtk_vbox_new( FALSE, 6 );
	gtk_container_add( GTK_CONTAINER( dialog ), vbox );

	label = gtk_label_new( _("Preparing for export ...") );
	gtk_misc_set_alignment( GTK_MISC( label ), 0, 0.5 );
	img->export_label = label;
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );

	progress = gtk_progress_bar_new();
	img->export_pbar1 = progress;
	string = g_strdup_printf( "%.2f", .0 );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( progress ), string );
	gtk_box_pack_start( GTK_BOX( vbox ), progress, FALSE, FALSE, 0 );

	label = gtk_label_new( _("Overall progress:") );
	gtk_misc_set_alignment( GTK_MISC( label ), 0, 0.5 );
	gtk_box_pack_start( GTK_BOX( vbox ), label, FALSE, FALSE, 0 );

	progress = gtk_progress_bar_new();
	img->export_pbar2 = progress;
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( progress ), string );
	gtk_box_pack_start( GTK_BOX( vbox ), progress, FALSE, FALSE, 0 );
	g_free( string );

	hbox = gtk_hbox_new( TRUE, 6 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

	button = gtk_button_new_from_stock( GTK_STOCK_CANCEL );
	g_signal_connect_swapped( G_OBJECT( button ), "clicked",
							  G_CALLBACK( img_stop_export ), img );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

	button = gtk_toggle_button_new_with_label( GTK_STOCK_MEDIA_PAUSE );
	gtk_button_set_use_stock( GTK_BUTTON( button ), TRUE );
	g_signal_connect( G_OBJECT( button ), "toggled",
					  G_CALLBACK( img_export_pause_unpause ), img );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

	gtk_widget_show_all( dialog );

	/* Create first slide */
	img->image1 = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
											  img->video_size[0],
											  img->video_size[1] );
	cr = cairo_create( img->image1 );
	cairo_set_source_rgb( cr, img->background_color[0],
							  img->background_color[1],
							  img->background_color[2] );
	cairo_paint( cr );
	cairo_destroy( cr );

	/* Load first image from model */
	model = GTK_TREE_MODEL( img->thumbnail_model );
	gtk_tree_model_get_iter_first( model, &iter );
	gtk_tree_model_get( model, &iter, 1, &entry, -1 );

	if( ! entry->o_filename )
	{
		img_scale_gradient( entry->gradient, entry->g_start_point,
							entry->g_stop_point, entry->g_start_color,
							entry->g_stop_color, img->video_size[0],
							img->video_size[1], NULL, &img->image2 );
	}
	else
	{
		img_scale_image( entry->r_filename, img->video_ratio,
						 0, 0, img->distort_images,
						 img->background_color, NULL, &img->image2 );
	}

	/* Add export idle function and set initial values */
	img->export_is_running = 4;
	img->work_slide = entry;
	img->total_nr_frames = img->total_secs * img->export_fps;
	img->displayed_frame = 0;
	img->next_slide_off = 0;
	img_calc_next_slide_time_offset( img, img->export_fps );

	/* Create surfaces to be passed to transition renderer */
	img->image_from = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
												  img->video_size[0],
												  img->video_size[1] );
	img->image_to = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
												img->video_size[0],
												img->video_size[1] );
	img->exported_image = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
													  img->video_size[0],
													  img->video_size[1] );

	/* Set stop points */
	img->cur_point = NULL;
	img->point1 = NULL;
	img->point2 = (ImgStopPoint *)( img->work_slide->no_points ?
									img->work_slide->points->data :
									NULL );

	/* Set first slide */
	gtk_tree_model_get_iter_first( GTK_TREE_MODEL( img->thumbnail_model ),
								   &img->cur_ss_iter );

	img->export_slide = 1;
	img->export_idle_func = (GSourceFunc)img_export_transition;
	img->source_id = g_idle_add( (GSourceFunc)img_export_transition, img );

	string = g_strdup_printf( _("Slide %d export progress:"), 1 );
	/* I did this for the translators. ^^ */
	gtk_label_set_label( GTK_LABEL( img->export_label ), string );
	g_free( string );

	/* Update display */
	gtk_widget_queue_draw( img->image_area );

	return( FALSE );
}

/*
 * img_stop_export:
 * @img: global img_window_struct structure
 *
 * This function should be called whenever we want to terminate export. It'll
 * take care of removing any idle functions, terminating any spawned processes
 * or threads, etc.
 *
 * If this function needs to be connected to some kind of widget as a callback,
 * this should be done using g_signal_connect_swapped, since we're not
 * interested in widget that caused the termination.
 *
 * Return value: always FALSE, since this function can be also called as an idle
 * callback and needs to remove itelf from main context.
 */
gboolean
img_stop_export( img_window_struct *img )
{
	/* Do any additional tasks */
	if( img->export_is_running > 3 )
	{
		kill( img->ffmpeg_export, SIGINT );
		g_source_remove( img->source_id );

		close(img->file_desc);
		g_spawn_close_pid( img->ffmpeg_export );

		/* Destroy images that were used */
		cairo_surface_destroy( img->image1 );
		cairo_surface_destroy( img->image2 );
		cairo_surface_destroy( img->image_from );
		cairo_surface_destroy( img->image_to );
		cairo_surface_destroy( img->exported_image );

		/* Close export dialog */
		gtk_widget_destroy( img->export_dialog );
	}

	if( img->export_is_running > 1 )
	{
		/* Kill sox thread */
		if( img->exported_audio_no )
		{
			int i;

			if( g_atomic_int_get( &img->sox_flags ) != 2 )
			{
				gint   fd;
				guchar buf[1024];

				g_atomic_int_set( &img->sox_flags, 1 );

				/* Flush any buffered audio data from pipe */
				fd = open( img->fifo, O_RDONLY );
				while( read( fd, buf, sizeof( buf ) ) )
					;
				close( fd );
			}

			/* Wait for thread to finish */
			g_thread_join( img->sox );
			img->sox = NULL;

			for( i = 0; i < img->exported_audio_no; i++ )
				g_free( img->exported_audio[i] );
			img->exported_audio = NULL;
			img->exported_audio_no = 0;
		}
	}

	/* If we created FIFO, we need to destroy it now */
	if( img->fifo )
	{
		g_unlink( img->fifo );
		g_free( img->fifo );
		img->fifo = NULL;
	}

	/* Free ffmpeg cmd line */
	g_free( img->export_cmd_line );

	/* Indicate that export is not running any more */
	img->export_is_running = 0;

	/* Switch mode if needed */
	if( img->auto_switch )
	{
		img->auto_switch = FALSE;
		img_switch_mode( img, 1 );
	}
	else
	{
		/* Redraw preview area */
		gtk_widget_queue_draw( img->image_area );
	}

	return( FALSE );
}

/*
 * img_prepare_pixbufs:
 * @img: global img_window_struct
 * @preview: do we load image for preview
 *
 * This function is used when previewing or exporting slideshow. It goes
 * through the model and prepares everything for next transition.
 *
 * If @preview is TRUE, we also respect quality settings.
 *
 * This function also sets img->point[12] that are used for transitions.
 *
 * Return value: TRUE if images have been succefully prepared, FALSE otherwise.
 */
gboolean
img_prepare_pixbufs( img_window_struct *img,
					 gboolean           preview )
{
	GtkTreeModel    *model;
	static gboolean  last_transition = TRUE;

	model = GTK_TREE_MODEL( img->thumbnail_model );

	/* Get last stop point of current slide */
	img->point1 = (ImgStopPoint *)( img->work_slide->no_points ?
									g_list_last( img->work_slide->points )->data :
									NULL );

	if( last_transition && gtk_tree_model_iter_next( model, &img->cur_ss_iter ) )
	{
		/* We have next iter, so prepare for next round */
		cairo_surface_destroy( img->image1 );
		img->image1 = img->image2;
		gtk_tree_model_get( model, &img->cur_ss_iter, 1, &img->work_slide, -1 );

		if( ! img->work_slide->o_filename )
		{
			img_scale_gradient( img->work_slide->gradient,
								img->work_slide->g_start_point,
								img->work_slide->g_stop_point,
								img->work_slide->g_start_color,
								img->work_slide->g_stop_color,
								img->video_size[0],
								img->video_size[1], NULL, &img->image2 );
		}
		else if( preview && img->low_quality )
			img_scale_image( img->work_slide->r_filename, img->video_ratio,
							 0, img->video_size[1], img->distort_images,
							 img->background_color, NULL, &img->image2 );
		else
			img_scale_image( img->work_slide->r_filename, img->video_ratio,
							 0, 0, img->distort_images,
							 img->background_color, NULL, &img->image2 );

		/* Get first stop point */
		img->point2 = (ImgStopPoint *)( img->work_slide->no_points ?
										img->work_slide->points->data :
										NULL );

		return( TRUE );
	}
	else if( last_transition )
	{
		cairo_t *cr;

		/* We displayed last image, but bye-bye transition hasn't
		 * been displayed. */
		last_transition = FALSE;

		cairo_surface_destroy( img->image1 );
		img->image1 = img->image2;

		img->image2 = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
												  img->video_size[0],
												  img->video_size[1] );
		cr = cairo_create( img->image2 );
		cairo_set_source_rgb( cr, img->background_color[0],
								  img->background_color[1],
								  img->background_color[2] );
		cairo_paint( cr );
		cairo_destroy( cr );

		img->work_slide = &img->final_transition;
		img->point2 = NULL;

		return( TRUE );
	}

	/* We're done now */
	last_transition = TRUE;

	return( FALSE );
}

/*
 * img_run_encoder:
 * @img:
 *
 * Tries to start encoder.
 *
 * Return value: TRUE if encoder has been started succesfully, else FALSE.
 */
static gboolean
img_run_encoder( img_window_struct *img )
{
	GtkWidget  *message;
	GError     *error = NULL;
	gchar     **argv;
	gint		argc;
	gboolean    ret;

	g_shell_parse_argv( img->export_cmd_line, &argc, &argv, NULL);
	img_message(img, FALSE, "Running %s\n", img->export_cmd_line);

	ret = g_spawn_async_with_pipes( NULL, argv, NULL,
									G_SPAWN_SEARCH_PATH,
									NULL, NULL, &img->ffmpeg_export,
									&img->file_desc,
                                    NULL,               /* print to standard_output */
                                    NULL,               /* print to standard_error */
                                    &error );
	if( ! ret )
	{
		message = gtk_message_dialog_new( GTK_WINDOW( img->imagination_window ),
										  GTK_DIALOG_MODAL |
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
										  _("Failed to launch the encoder!" ) );
		gtk_message_dialog_format_secondary_text( GTK_MESSAGE_DIALOG( message ),
												  "%s.", error->message );
		gtk_dialog_run( GTK_DIALOG( message ) );
		gtk_widget_destroy( message );
		g_error_free( error );
	}

	g_strfreev( argv );

	return( ret );
}

/*
 * img_calc_next_slide_time_offset:
 * @img: global img_window_struct structure
 * @rate: frame rate to be used for calculations
 *
 * This function will calculate:
 *   - time offset of next slide (img->next_slide_off)
 *   - number of frames for current slide (img->slide_nr_frames)
 *   - number of slides needed for transition (img->slide_trans_frames)
 *   - number of slides needed for still part (img->slide_still_franes)
 *   - reset current slide counter to 0 (img->slide_cur_frame)
 *   - number of frames for subtitle animation (img->no_text_frames)
 *   - reset current subtitle counter to 0 (img->cur_text_frame)
 *
 * Return value: new time offset. The same value is stored in
 * img->next_slide_off.
 */
guint
img_calc_next_slide_time_offset( img_window_struct *img,
								 gdouble            rate )
{
	if( img->work_slide->render )
	{
		img->next_slide_off += img->work_slide->duration +
							   img->work_slide->speed;
		img->slide_trans_frames = img->work_slide->speed * rate;
	}
	else
	{
		img->next_slide_off += img->work_slide->duration;
		img->slide_trans_frames = 0;
	}

	img->slide_nr_frames = img->next_slide_off * rate - img->displayed_frame;
	img->slide_cur_frame = 0;
	img->slide_still_frames = img->slide_nr_frames - img->slide_trans_frames;

	/* Calculate subtitle frames */
	if( img->work_slide->subtitle )
	{
		img->cur_text_frame = 0;
		img->no_text_frames = img->work_slide->anim_duration * rate;
	}

	return( img->next_slide_off );
}

/*
 * img_export_transition:
 * @img:
 *
 * This is idle callback function that creates transition frames. When
 * transition is complete, it detaches itself from main context and connects
 * still export function.
 *
 * Return value: TRUE if transition isn't exported completely, FALSE otherwise.
 */
static gboolean
img_export_transition( img_window_struct *img )
{
	gchar   string[10];
	gdouble export_progress;

	/* If we rendered all transition frames, connect still export */
	if( img->slide_cur_frame == img->slide_trans_frames )
	{
		img->export_idle_func = (GSourceFunc)img_export_still;
		img->source_id = g_idle_add( (GSourceFunc)img_export_still, img );

		return( FALSE );
	}

	/* Draw one frame of transition animation */
	img_render_transition_frame( img );

	/* Export frame */
	img_export_frame_to_ppm( img->exported_image, img->file_desc );

	/* Increment global frame counters and update progress bars */
	img->slide_cur_frame++;
	img->displayed_frame++;

	export_progress = CLAMP( (gdouble)img->slide_cur_frame /
									  img->slide_nr_frames, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar1 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar1 ), string );

	export_progress = CLAMP( (gdouble)img->displayed_frame /
									  img->total_nr_frames, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar2 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar2 ), string );

	/* Draw every 10th frame of animation on screen */
	if( img->displayed_frame % 10 == 0 )
		gtk_widget_queue_draw( img->image_area );

	return( TRUE );
}

/*
 * img_export_still:
 * @img:
 *
 * Idle callback that outputs still image frames. When enough frames has been
 * outputed, it connects transition export.
 *
 * Return value: TRUE if more still frames need to be exported, else FALSE.
 */
static gboolean
img_export_still( img_window_struct *img )
{
	gdouble export_progress;
	gchar   string[10];

	/* If there is next slide, connect transition preview, else finish
	 * preview. */
	if( img->slide_cur_frame == img->slide_nr_frames )
	{
		if( img_prepare_pixbufs( img, FALSE ) )
		{
			gchar *string;

			img_calc_next_slide_time_offset( img, img->export_fps );
			img->export_slide++;

			/* Make dialog more informative */
			if( img->work_slide->duration == 0 )
				string = g_strdup_printf( _("Final transition export progress:") );
			else
				string = g_strdup_printf( _("Slide %d export progress:"),
										  img->export_slide );
			gtk_label_set_label( GTK_LABEL( img->export_label ), string );
			g_free( string );

			img->export_idle_func = (GSourceFunc)img_export_transition;
			img->source_id = g_idle_add( (GSourceFunc)img_export_transition, img );

			img->cur_point = NULL;
		}
		else
			img_stop_export( img );

		return( FALSE );
	}

	/* Draw frames until we have enough of them to fill slide duration gap. */
	img_render_still_frame( img, img->export_fps );

	/* Export frame */
	img_export_frame_to_ppm( img->exported_image, img->file_desc );

	/* Increment global frame counter and update progress bar */
	img->still_counter++;
	img->slide_cur_frame++;
	img->displayed_frame++;

	/* CLAMPS are needed here because of the loosy conversion when switching
	 * from floating point to integer arithmetics. */
	export_progress = CLAMP( (gdouble)img->slide_cur_frame /
									  img->slide_nr_frames, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar1 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar1 ), string );

	export_progress = CLAMP( (gdouble)img->displayed_frame /
									  img->total_nr_frames, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar2 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar2 ), string );

	/* Draw every 10th frame of animation on screen */
	if( img->displayed_frame % 10 == 0 )
		gtk_widget_queue_draw( img->image_area );

	return( TRUE );
}

/*
 * img_export_pause_unpause:
 * @img:
 *
 * Temporarily disconnect export functions. This doesn't stop ffmpeg!!!
 */
static void
img_export_pause_unpause( GtkToggleButton   *button,
						  img_window_struct *img )
{
	if( gtk_toggle_button_get_active( button ) )
		/* Pause export */
		g_source_remove( img->source_id );
	else
		img->source_id = g_idle_add(img->export_idle_func, img);
}

void
img_render_transition_frame( img_window_struct *img )
{
	ImgStopPoint  point = { 0, 0, 0, 1.0 }; /* Default point */
	gdouble       progress;
	cairo_t      *cr;

	/* Do image composing here and place result in exported_image */
	/* Create first image */
	cr = cairo_create( img->image_from );
	img_draw_image_on_surface( cr, img->video_size[0], img->image1,
							   ( img->point1 ? img->point1 : &point ), img );

#if 0
	/* Render subtitle if present */
	if( img->work_slide->subtitle )
	{
		gdouble       progress;     /* Text animation progress */
		ImgStopPoint *p_draw_point; 

		progress = (gdouble)img->cur_text_frame / ( img->no_text_frames - 1 );
		progress = CLAMP( progress, 0, 1 );
		img->cur_text_frame++;

		p_draw_point = ( img->point1 ? img->point1 : &point );

		img_render_subtitle( cr,
							 img->video_size[0],
							 img->video_size[1],
							 1.0,
							 img->work_slide->position,
							 img->work_slide->placing,
							 p_draw_point->zoom,
							 p_draw_point->offx,
							 p_draw_point->offy,
							 img->work_slide->subtitle,
							 img->work_slide->font_desc,
							 img->work_slide->font_color,
							 img->work_slide->anim,
							 progress );
	}
#endif
	cairo_destroy( cr );

	/* Create second image */
	cr = cairo_create( img->image_to );
	img_draw_image_on_surface( cr, img->video_size[0], img->image2,
							   ( img->point2 ? img->point2 : &point ), img );
	/* FIXME: Add subtitles here */
	cairo_destroy( cr );

	/* Compose them together */
	progress = (gdouble)img->slide_cur_frame / ( img->slide_trans_frames - 1 );
	cr = cairo_create( img->exported_image );
	cairo_save( cr );
	img->work_slide->render( cr, img->image_from, img->image_to, progress );
	cairo_restore( cr );

	cairo_destroy( cr );
}

void
img_render_still_frame( img_window_struct *img,
						gdouble            rate )
{
	cairo_t      *cr;
	ImgStopPoint *p_draw_point;                  /* Pointer to current sp */
	ImgStopPoint  draw_point = { 0, 0, 0, 1.0 }; /* Calculated stop point */

	/* If no stop points are specified, we simply draw img->image2 with default
	 * stop point on each frame.
	 *
	 * If we have only one stop point, we draw img->image2 on each frame
	 * properly scaled, with no movement.
	 *
	 * If we have more than one point, we draw movement from point to point.
	 */
	switch( img->work_slide->no_points )
	{
		case( 0 ): /* No stop points */
			p_draw_point = &draw_point;
			break;

		case( 1 ): /* Single stop point */
			p_draw_point = (ImgStopPoint *)img->work_slide->points->data;
			break;

		default:   /* Many stop points */
			{
				ImgStopPoint *point1,
							 *point2;
				gdouble       progress;
				GList        *tmp;

				if( ! img->cur_point )
				{
					/* This is initialization */
					img->cur_point = img->work_slide->points;
					point1 = (ImgStopPoint *)img->cur_point->data;
					img->still_offset = point1->time;
					img->still_max = img->still_offset * rate;
					img->still_counter = 0;
					img->still_cmlt = 0;
				}
				else if( img->still_counter == img->still_max )
				{
					/* This is advancing to next point */
					img->cur_point = g_list_next( img->cur_point );
					point1 = (ImgStopPoint *)img->cur_point->data;
					img->still_offset += point1->time;
					img->still_cmlt += img->still_counter;
					img->still_max = img->still_offset * rate -
									 img->still_cmlt;
					img->still_counter = 0;
				}

				point1 = (ImgStopPoint *)img->cur_point->data;
				tmp = g_list_next( img->cur_point );
				if( tmp )
				{
					point2 = (ImgStopPoint *)tmp->data;
					progress = (gdouble)img->still_counter /
										( img->still_max - 1);
					img_calc_current_ken_point( &draw_point, point1, point2,
												progress, 0 );
					p_draw_point = &draw_point;
				}
				else
					p_draw_point = point1;
			}
			break;
	}

	/* Paint surface */
	cr = cairo_create( img->exported_image );
	img_draw_image_on_surface( cr, img->video_size[0], img->image2,
							   p_draw_point, img );

	/* Render subtitle if present */
	if( img->work_slide->subtitle )
	{
		gdouble progress; /* Text animation progress */

		progress = (gdouble)img->cur_text_frame / ( img->no_text_frames - 1 );
		progress = CLAMP( progress, 0, 1 );
		img->cur_text_frame++;

		img_render_subtitle( cr,
							 img->video_size[0],
							 img->video_size[1],
							 1.0,
							 img->work_slide->position,
							 img->work_slide->placing,
							 p_draw_point->zoom,
							 p_draw_point->offx,
							 p_draw_point->offy,
							 img->work_slide->subtitle,
							 img->work_slide->font_desc,
							 img->work_slide->font_color,
                             img->work_slide->font_bgcolor,
							 img->work_slide->anim,
							 progress );
	}

	/* Destroy drawing context */
	cairo_destroy( cr );
}

static void
img_export_frame_to_ppm( cairo_surface_t *surface,
						 gint             file_desc )
{
	cairo_format_t  format;
	gint            width, height, stride, row, col;
	guchar         *data, *pix;
	gchar          *header;

	guchar         *buffer, *tmp;
	gint            buf_size;

	/* Get info about cairo surface passed in. */
	format = cairo_image_surface_get_format( surface );

	/* For more information on diferent formats, see
	 * www.cairographics.org/manual/cairo-image-surface.html#cairo-format-t */
	/* Currently this exporter only handles CAIRO_FORMAT_(ARGB32|RGB24)
	 * formats. */
	if( ! format == CAIRO_FORMAT_ARGB32 && ! format == CAIRO_FORMAT_RGB24 )
	{
		g_print("Unsupported cairo surface format!\n" );
		return;
	}

	/* Image info and pixel data */
	width  = cairo_image_surface_get_width( surface );
	height = cairo_image_surface_get_height( surface );
	stride = cairo_image_surface_get_stride( surface );
	pix    = cairo_image_surface_get_data( surface );

	/* Output PPM file header information:
	 *   - P6 is a magic number for PPM file
	 *   - width and height are image's dimensions
	 *   - 255 is number of colors
	 * */
	header = g_strdup_printf( "P6\n%d %d\n255\n", width, height );
	write( file_desc, header, sizeof( gchar ) * strlen( header ) );
	g_free( header );

	/* PRINCIPLES BEHING EXPORT LOOP
	 *
	 * Cairo surface data is composed of height * stride 32-bit numbers. The
	 * actual data for displaying image is inside height * width boundary,
	 * and each pixel is represented with 1 32-bit number.
	 *
	 * In CAIRO_FORMAT_ARGB32, first 8 bits contain alpha value, second 8
	 * bits red value, third green and fourth 8 bits blue value.
	 *
	 * In CAIRO_FORMAT_RGB24, groups of 8 bits contain values for red, green
	 * and blue color respectively. Last 8 bits are unused.
	 *
	 * Since guchar type contains 8 bits, it's usefull to think of cairo
	 * surface as a height * stride gropus of 4 guchars, where each guchar
	 * holds value for each color. And this is the principle behing my method
	 * of export.
	 * */

	/* Output PPM data */
	buf_size = sizeof( guchar ) * width * height * 3;
	buffer = g_slice_alloc( buf_size );
	tmp = buffer;
	data = pix;
	for( row = 0; row < height; row++ )
	{
		data = pix + row * stride;

		for( col = 0; col < width; col++ )
		{
			/* Output data. This is done differenty on little endian
			 * and big endian machines. */
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			/* Little endian machine sees pixel data as being stored in
			 * BGRA format. This is why we skip the last 8 bit group and
			 * read the other three groups in reverse order. */
			tmp[0] = data[2];
			tmp[1] = data[1];
			tmp[2] = data[0];
#elif G_BYTE_ORDER == G_BIG_ENDIAN
			tmp[0] = data[1];
			tmp[1] = data[2];
			tmp[2] = data[3];
#endif
			data += 4;
			tmp  += 3;
		}
	}
	write( file_desc, buffer, buf_size );
	g_slice_free1( buf_size, buffer );
}

/* ****************************************************************************
 * Exporter function
 *
 * This is the function that will be called when user selects export format.
 *
 * There is one function for all formats, data is stored in video_format_list
 * (new_slideshow.c). If you wish to add a new format, have a look at 
 * video_format_list.
 *
 * The exporter function receives a pointer to main img_window_struct structure,
 * from which it calculates appropriate ffmpeg export string.
 *
 * Structure, passed in as a parameter, should be treated like read-only
 * information source. Exceptions to this rule is the export_cmd_line field.
 *
 * For example, if we spawn ffmpeg with "-r 25" in it's cmd line, export_fps
 * should be set to 25. This will ensure that ffmpeg will receive proper amount
 * of data to fill the video with frames.
 *
 * String should be newly allocated using g_strdup(_printf)? functions, since
 * export framework will try to free it using g_free. It should also contain
 * placeholder named <#AUDIO#>, which will be in next stage replaced by real
 * path to newly produced audio file (at this stage, we don't have any).
 * ************************************************************************* */

void img_exporter (GtkWidget *button, img_window_struct *img )
{
	gchar          *cmd_line, *bitrate_cmd, *aspect_ratio_cmd;
	const gchar    *filename;
	GtkWidget      *dialog;
	GtkEntry       *entry;
	GtkWidget      *vbox;

	/* This function call should be the first thing exporter does, since this
	 * function will take some preventive measures and also switches mode into
	 * preview if needed. */
    /* FIXME: change with a gtk_file_chooser ? */
	dialog = img_create_export_dialog( img, _(video_format_list[img->video_format_index].name),
									   GTK_WINDOW( img->imagination_window ),
									   &entry, &vbox );

	/* If dialog is NULL, abort. */
	if( dialog == NULL )
		return;

	gtk_widget_show_all (dialog );

    /* Run dialog and abort if needed */
	if ( gtk_dialog_run( GTK_DIALOG( dialog ) ) != GTK_RESPONSE_ACCEPT )
	{
		gtk_widget_destroy( dialog );
		return;
	}

    if (gtk_entry_get_text_length (entry) == 0) /* No filename given */
    {
        gtk_widget_destroy( dialog );
        return;
    }

    filename = gtk_entry_get_text( entry );

	/* User is serious, so we better prepare ffmepg command line;) */
	img->export_is_running = 1;

    if (NULL != video_format_list[img->video_format_index].aspect_ratio_list)
        aspect_ratio_cmd = g_strdup_printf("%s%s",
                    img->ffmpeg_aspect_ratio_cmd,
                    video_format_list[img->video_format_index].aspect_ratio_list[img->aspect_ratio_index].ffmpeg_option);
    else
        aspect_ratio_cmd = g_strdup("");

    if (NULL != video_format_list[img->video_format_index].bitratelist)
        bitrate_cmd = g_strdup_printf("-b %d",
                    video_format_list[img->video_format_index].bitratelist[img->bitrate_index].value);
    else
        bitrate_cmd = g_strdup("");


	cmd_line = g_strdup_printf("ffmpeg -f image2pipe -vcodec ppm "
                "-i pipe: <#AUDIO#> "
                "-y "                   /* overwrite output */
                "%s "                   /* ffmpeg option */
                "-r %s "                /* frame rate */
                "-s %dx%d "             /* size */
                "%s "                   /* aspect ratio */
                "%s "                   /* Bitrate */
                "\"%s\"",               /*filename */
                video_format_list[img->video_format_index].ffmpeg_option,
                video_format_list[img->video_format_index].fps_list[img->fps_index].ffmpeg_option,
                img->video_size[0], img->video_size[1],
                aspect_ratio_cmd,
                bitrate_cmd,
                filename);
	img->export_cmd_line = cmd_line;

	/* Initiate stage 2 of export - audio processing */
	g_idle_add( (GSourceFunc)img_prepare_audio, img );

    g_free(aspect_ratio_cmd);
    g_free(bitrate_cmd);
	gtk_widget_destroy( dialog );
}


/* Test ffmpeg abilities */
void test_ffmpeg(img_window_struct *img)
{
    /* ffmpeg test */
    gchar *ffmpeg_test_result;
    gchar **argv;
    gint    argc;

    /* Check if ffmpeg is compiled with avfilter setdar */
    img_message(img, FALSE, "Testing ffmpeg abilities with \"ffmpeg -filters\" ... ");

    g_shell_parse_argv("ffmpeg -filters", &argc, &argv, NULL);
    g_spawn_sync(NULL, argv, NULL,
                 G_SPAWN_STDERR_TO_DEV_NULL|G_SPAWN_SEARCH_PATH,
                 NULL, NULL,
                 &ffmpeg_test_result, NULL,
                 NULL, NULL);
    if (NULL != ffmpeg_test_result && NULL != g_strrstr(ffmpeg_test_result, "setdar"))
    {
        img_message(img, FALSE, "setdar found!\n");
        img->ffmpeg_aspect_ratio_cmd = "-vf setdar=";
    }
    else
    {
        img_message(img, FALSE, "setdar not found!\n");
        img->ffmpeg_aspect_ratio_cmd = "-aspect ";

    }
    g_strfreev( argv );

}
