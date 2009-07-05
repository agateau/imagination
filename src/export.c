/*
** Copyright (c) 2009 Giuseppe Torelli <colossus73@gmail.com>
** Copyright (C) 2009 Tadej Borovšak   <tadeboro@gmail.com>
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

static void
img_export_pixbuf_to_ppm( GdkPixbuf  *pixbuf,
						  guchar   **data,
						  guint     *lenght );

static gboolean
img_run_encoder( img_window_struct *img );

static void
img_export_calc_slide_frames( img_window_struct *img );

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
img_exporter_vob( img_window_struct *img );
static void
img_exporter_ogg( img_window_struct *img );
static void
img_exporter_flv( img_window_struct *img );


/*
 * img_get_exporters:
 * @exporters: location to put list of available exporters
 *
 * This function is here to simplify accessing all available exporters.
 *
 * Any newly added exporters should be listed in array returned by this function
 * or Imagination WILL NOT create menu entries for them.
 *
 * List that is placed in exporters parameter should be considered read-only and
 * freed after usage with img_free_exporters_list.
 *
 * Return value: Size of list in exporters.
 */
gint
img_get_exporters_list( Exporter **exporters )
{
	Exporter *list;             /* List of all exporters */
	gint      no_exporters = 3; /* Total number of exporters */
	gint      i = 0;
	
	list = g_slice_alloc( sizeof( Exporter ) * no_exporters );

	/* Populate list with data */
	list[i].description = g_strdup( "VOB (DVD video)" );
	list[i++].func = G_CALLBACK( img_exporter_vob );
	list[i].description = g_strdup( "OGG (Theora/Vorbis)" );
	list[i++].func = G_CALLBACK( img_exporter_ogg );
	list[i].description = g_strdup( "FLV (Flash video)" );
	list[i++].func = G_CALLBACK( img_exporter_flv );

	*exporters = list;

	return( no_exporters );
}

/*
 * img_free_exporters_list:
 * @no_exporters: number of exporters in exporters list
 * @exporters: array of Exporter structs
 *
 * This function takes care of freeing any memory allocated by
 * img_get_exporters_list function.
 */
void
img_free_exporters_list( gint      no_exporters,
						 Exporter *exporters )
{
	register gint i;

	for( i = 0; i < no_exporters; i++ )
		g_free( exporters[i].description );

	g_slice_free1( sizeof( Exporter ) * no_exporters, exporters );
}


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

	/* Abort if no slide is present */
	model = gtk_icon_view_get_model( GTK_ICON_VIEW( img->thumbnail_iconview ) );
	if( ! gtk_tree_model_get_iter_first( model, &iter ) )
	{
		return( NULL );
	}

	/* Indicate that export has been started */
	img->export_is_running = 1;

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

	label = gtk_label_new( _("Filename of the slideshow:") );
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
 *
 * This function represents second link in export chain. It concatenates audio
 * files into single file, trims this file and places full path into
 * img->export_audio_file.
 *
 * Currently, it simply places first audio file from store into
 * export_audio_file. This is only temporary solution, but I needed to place
 * something here in order to be able to test the whole chain. In final version,
 * this function will probably just the first of three.
 *
 * Task of this function will be to create audio preparation dialog, spawn sox
 * thread and setup and communication between threads.
 *
 * Second function will only update progress based on data sent from sox thread.
 * It'll also offer user a chance to terminate export process.
 *
 * Third function will be responsible for hiding audio dialog, finalizing ffmpeg
 * command line and linking to video export process.
 *
 * Return value: This function must always return FALSE, since we want it
 * removed from main context.
 */
static gboolean
img_prepare_audio( img_window_struct *img )
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	gchar        *audio_file;
	gchar       **tmp;


	/* Set the export info */
	img->export_is_running = 2;

	model = gtk_tree_view_get_model( GTK_TREE_VIEW( img->music_file_treeview ) );
	if( gtk_tree_model_get_iter_first( model, &iter ) )
	{
		gchar   *path, *filename;
		GString *audio_string;

		audio_string = g_string_new( "" );
		do
		{
			gtk_tree_model_get( model, &iter, 0, &path, 1, &filename, -1 );
			g_string_append_printf( audio_string, " -i \"%s%s%s\"",
									path, G_DIR_SEPARATOR_S, filename );
			
			g_free( path );
			g_free( filename );
		}
		while( gtk_tree_model_iter_next( model, &iter ) );

		audio_file = audio_string->str;
		g_string_free( audio_string, FALSE );
	}
	else
		/* Disable audio */
		audio_file = g_strdup( "-an" );

	/* Replace audio place holder */
	tmp = g_strsplit( img->export_cmd_line, "<#AUDIO#>", 0 );
	g_free( img->export_cmd_line );
	img->export_cmd_line = g_strjoin( NULL, tmp[0], audio_file,
									  tmp[1], NULL );

	img->export_audio_file = audio_file;

	/* Chain last export step - video export */
	g_idle_add( (GSourceFunc)img_start_export, img );

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

	/* Display some visual feedback */
	while( gtk_events_pending() )
		gtk_main_iteration();

#ifdef TB_EDITS
	img->stored_image = img->current_image;

	/* FIXME: Here background color is fixed to BLACK!!! */
	img->image1 = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
											  img->image_area->allocation.width,
											  img->image_area->allocation.height );
#else
	img->slide_pixbuf = gtk_image_get_pixbuf( GTK_IMAGE( img->image_area ) );
	if( img->slide_pixbuf )
		g_object_ref( G_OBJECT( img->slide_pixbuf ) );
	gtk_image_clear( GTK_IMAGE( img->image_area ) );
	gtk_widget_set_app_paintable( img->image_area, TRUE );
	g_signal_connect( G_OBJECT( img->image_area ), "expose-event",
					  G_CALLBACK( img_on_expose_event ), img );

	/* Create an empty pixbuf for starting image. */
	img->pixbuf1 = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8,
								   img->image_area->allocation.width,
								   img->image_area->allocation.height );
	gdk_pixbuf_fill( img->pixbuf1, img->background_color );
#endif

	/* Load first image from model */
	model = gtk_icon_view_get_model( GTK_ICON_VIEW( img->thumbnail_iconview ) );
	gtk_tree_model_get_iter_first( model, &iter );
	gtk_tree_model_get( model, &iter, 1, &entry, -1 );
#ifdef TB_EDITS
	img->image1 = img_scale_image( img, entry->filename, 0, 0 );
#else
	img->pixbuf2 = img_scale_pixbuf( img, entry->filename );
#endif

	/* Add export idle function and set initial values */
	img->export_is_running = 4;
	img->current_slide = entry;
	img->progress = 0;
	img->export_frame_nr = img->total_secs * img->export_fps;
	img->export_frame_cur = 0;

	/* Fix for the wrong progress bar indicators. */
	img_export_calc_slide_frames( img );

	img->export_slide = 1;
	img->export_idle_func = (GSourceFunc)img_export_transition;
	img->source_id = g_idle_add( (GSourceFunc)img_export_transition, img );

	string = g_strdup_printf( _("Slide %d export progress:"), 1 );
	/* I did this for the translators. ^^ */
	gtk_label_set_label( GTK_LABEL( img->export_label ), string );
	g_free( string );

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
	switch( img->export_is_running )
	{
		case 2:
			/* Kill sox thread here and delete files */
			g_source_remove( img->source_id );
			break;

		case 4:
			kill( img->ffmpeg_export, SIGINT );
			g_source_remove( img->source_id );

#ifdef TB_EDITS
			img->current_image = img->stored_image;
#else
			/* Disconnect expose event */
			g_signal_handlers_disconnect_by_func( img->image_area,
												  img_on_expose_event, img );
			gtk_widget_set_app_paintable( img->image_area, FALSE );
			
			/* Restore image that was used before export */
			gtk_image_set_from_pixbuf( GTK_IMAGE( img->image_area),
									   img->slide_pixbuf );
			if( img->slide_pixbuf )
				g_object_unref( G_OBJECT( img->slide_pixbuf ) );
#endif
			
			/* Clean other resources */
			g_slice_free( GtkTreeIter, img->cur_ss_iter );
			img->cur_ss_iter = NULL;
			g_free(img->pixbuf_data);
			img->pixbuf_data = NULL;
			
			close(img->file_desc);
			g_spawn_close_pid( img->ffmpeg_export );

			/* Close export dialog */
			gtk_widget_destroy( img->export_dialog );
			break;
	}

	/* This will be neede when we start producing our own audio file */
#if 0
	/* Delete audio file it present */
	if( strcmp( img->export_audio_file, "-an" ) )
		g_unlink( img->export_audio_file );
#endif

	/* Free ffmpeg cmd line and audio file */
	g_free( img->export_cmd_line );
	g_free( img->export_audio_file );

	/* Indicate that export is not running any more */
	img->export_is_running = 0;

	return( FALSE );
}

/*
 * img_prepare_pixbufs:
 * @img: global img_window_struct
 *
 * This function is used when previewing or exporting slideshow. It goes
 * through the model and prepares everything for next transition.
 *
 * Return value: TRUE if images have been succefully prepared, FALSE otherwise.
 */
gboolean
img_prepare_pixbufs( img_window_struct *img )
{
	GtkTreeModel    *model;
	static gboolean  last_transition = TRUE;

	model = gtk_icon_view_get_model( GTK_ICON_VIEW( img->thumbnail_iconview ) );

	if( ! img->cur_ss_iter )
	{
		img->cur_ss_iter = g_slice_new( GtkTreeIter );
		gtk_tree_model_get_iter_first( model, img->cur_ss_iter );
	}

	if( gtk_tree_model_iter_next( model, img->cur_ss_iter ) )
	{
		/* We have next iter, so prepare for next round */
#ifdef TB_EDITS
		cairo_surface_destroy( img->image1 );
		img->image1 = img->image2;
		gtk_tree_model_get( model, img->cur_ss_iter, 1, &img->current_slide, -1 );
		img->image2 = img_scale_image( img, img->current_slide->filename, 0, 0 );
#else
		g_object_unref( G_OBJECT( img->pixbuf1 ) );
		img->pixbuf1 = img->pixbuf2;
		gtk_tree_model_get( model, img->cur_ss_iter, 1, &img->current_slide, -1 );
		img->pixbuf2 = img_scale_pixbuf( img, img->current_slide->filename );
#endif

		return(TRUE);
	}
	else if( last_transition )
	{
		/* We displayed last image, but bye-bye transition hasn't
		 * been displayed. */
		last_transition = FALSE;

#ifdef TB_EDITS
		cairo_surface_destroy( img->image1 );
		img->image1 = img->image2;
		/* FIXME: This image is always black!!! */
		img->image2 = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
												  img->image_area->allocation.width,
												  img->image_area->allocation.height );
#else
		g_object_unref( G_OBJECT( img->pixbuf1 ) );
		img->pixbuf1 = img->pixbuf2;
		img->pixbuf2 = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8,
									   img->image_area->allocation.width,
									   img->image_area->allocation.height );
		gdk_pixbuf_fill( img->pixbuf2, img->background_color );
#endif
		img->current_slide = &img->final_transition;

		return( TRUE );
	}

	/* We're done now */
	last_transition = TRUE;

	return(FALSE);
}

/*
 * img_export_pixbuf_to_ppm:
 * @pixbuf: GdkPixbuf to be exported
 * @data: location for data
 * @lenght: location for @data lenght
 *
 * Converts GdkPixbuf into PPM format.
 */
static void
img_export_pixbuf_to_ppm( GdkPixbuf  *pixbuf,
						  guchar    **data,
						  guint      *lenght )
{
	gint      width, height, stride, channels;
	guchar   *pixels, *tmp;
	gint      col, row;
	gchar    *header;
	gint      header_lenght;

	width    = gdk_pixbuf_get_width( pixbuf );
	height   = gdk_pixbuf_get_height( pixbuf );
	stride   = gdk_pixbuf_get_rowstride( pixbuf );
	channels = gdk_pixbuf_get_n_channels( pixbuf );
	pixels   = gdk_pixbuf_get_pixels( pixbuf );

	header = g_strdup_printf( "P6\n%d %d\n255\n", width, height );
	header_lenght = strlen( header ) * sizeof(gchar);

	*lenght = sizeof( guchar ) * width * height * channels + header_lenght;
	*data = g_slice_alloc( sizeof( guchar ) * *lenght);

	memcpy( *data, header, header_lenght );
	tmp = *data + header_lenght;
	for( row = 0; row < height; row++ )
	{
		for( col = 0; col < width; col++ )
		{
			tmp[0] = pixels[0];
			tmp[1] = pixels[1];
			tmp[2] = pixels[2];

			tmp    += 3;
			pixels += channels;
		}
	}
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
	g_print( "%s\n", img->export_cmd_line);

	ret = g_spawn_async_with_pipes( NULL, argv, NULL,
									G_SPAWN_SEARCH_PATH /*|
									G_SPAWN_DO_NOT_REAP_CHILD |
									G_SPAWN_STDOUT_TO_DEV_NULL |
									G_SPAWN_STDERR_TO_DEV_NULL*/,
									NULL, NULL, &img->ffmpeg_export,
									&img->file_desc, NULL, NULL, &error );
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
 * img_export_calc_slide_frames:
 * @img:
 *
 * This function calculates how many frames will this slide need in order to be
 * exported completely. We need this information in order to be able display
 * slide export progress.
 */
static void
img_export_calc_slide_frames( img_window_struct *img )
{

	if( img->current_slide->render )
		/* Duration + transition time */
		img->export_slide_nr = ( img->current_slide->duration +
								 img->current_slide->speed ) * img->export_fps;
	else
		/* Duration only */
		img->export_slide_nr = img->current_slide->duration * img->export_fps;

	img->export_slide_cur = 0;
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

	/* If no transition effect is set, just connect still export
	 * idle function and remove itself from main loop. */
	if( img->current_slide->render == NULL )
	{
		img->source_id = g_idle_add( (GSourceFunc)img_export_still, img );
		return( FALSE );
	}

	/* Switch to still export phase if progress reached 1. */
	img->progress += (gdouble)1 / ( img->current_slide->speed * img->export_fps );
	if(img->progress > 1.00000005)
	{
		img->progress = 0;
		img->export_idle_func = (GSourceFunc)img_export_still;
		img->source_id = g_idle_add( (GSourceFunc)img_export_still, img );

		return( FALSE );
	}

	/* Draw one frame of transition animation */
#ifdef TB_EDITS
	/* FIXME!!!! */
#else
	img->current_slide->render( img->image_area->window, img->pixbuf1,
								img->pixbuf2, img->progress, img->file_desc );
#endif

	/* Increment global frame counters and update progress bars */
	img->export_frame_cur++;
	img->export_slide_cur++;

	export_progress = CLAMP( (gdouble)img->export_slide_cur /
									  img->export_slide_nr, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar1 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar1 ), string );

	export_progress = CLAMP( (gdouble)img->export_frame_cur /
									  img->export_frame_nr, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar2 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar2 ), string );

	/* Draw every 10th frame of animation on screen */
	if( img->export_frame_cur % 10 == 0 )
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
	static guint length;
	gdouble      export_progress;
	gchar        string[10];

	/* Initialize pixbuf data buffer */
	if( img->pixbuf_data == NULL )
	{
#ifdef TB_EDITS
		/* FIXME!!!! */
#else
		gtk_image_set_from_pixbuf( GTK_IMAGE( img->image_area ), img->pixbuf2 );
		img_export_pixbuf_to_ppm( img->pixbuf2, &img->pixbuf_data, &length );
#endif
	}

	/* Draw frames until we have enough of them to fill slide duration gap. */
	if( img->export_slide_cur > img->export_slide_nr )
	{
		/* Exit still rendering and continue with next transition. */

#ifndef TB_EDITS
		/* Clear image area for next renderer */
		gtk_image_clear( GTK_IMAGE( img->image_area ) );
#endif

		/* Load next image from store. */
		if( img_prepare_pixbufs( img ) )
		{
			gchar *string;

			/* Update progress counters */
			img->export_slide++;

			/* Make dialog more informative */
			if( img->current_slide->duration == 0 )
				string = g_strdup_printf( _("Final transition export progress:") );
			else
				string = g_strdup_printf( _("Slide %d export progress:"),
										  img->export_slide );
			gtk_label_set_label( GTK_LABEL( img->export_label ), string );
			g_free( string );
			img_export_calc_slide_frames( img );

			g_free( img->pixbuf_data );
			img->pixbuf_data = NULL;
			img->export_idle_func = (GSourceFunc)img_export_transition;
			img->source_id = g_idle_add( (GSourceFunc)img_export_transition, img );
		}
		else
			img_stop_export( img );

		return( FALSE );
	}
	write( img->file_desc, img->pixbuf_data, length );

	/* Increment global frame counter and update progress bar */
	img->export_frame_cur++;
	img->export_slide_cur++;

	/* CLAMPS are needed here because of the loosy conversion when switching
	 * from floating point to integer arithmetics. */
	export_progress = CLAMP( (gdouble)img->export_slide_cur / img->export_slide_nr, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar1 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar1 ), string );

	export_progress = CLAMP( (gdouble)img->export_frame_cur / img->export_frame_nr, 0, 1 );
	snprintf( string, 10, "%.2f%%", export_progress * 100 );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( img->export_pbar2 ),
								   export_progress );
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( img->export_pbar2 ), string );

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

/* ****************************************************************************
 * Exporter functions
 *
 * These are the functions that will be called when user selects export format.
 *
 * There is exactly one function per export format. Addresses of these functions
 * are stored inside Exporter structs, which can be obtained by calling
 * img_get_exporters function.
 *
 * Each exporter function receives pointer to main img_window_struct structure,
 * from which it calculates appropriate ffmpeg export string.
 *
 * Structure, passed in as a parameter, should be treated like read-only
 * information source. Exceptions to this rule are export_fps and
 * export_cmd_line fields. Those fields MUST be set to appropriate value.
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

/* Template for exporter function. */
#if 0
static void
img_exporter_<format>( img_window_struct *img )
{
	gchar          *cmd_line;
	gchar          *format;
	const gchar    *filename;
	gchar          *aspect_ratio;
	GtkWidget      *dialog;
	GtkEntry       *entry;
	GtkWidget      *vbox;

	/* Additional options - <format> only */

	/* This function call should be the first thing exporter does, since this
	 * function will take some preventive measures. */
	dialog = img_create_export_dialog( img, _("VOB export"),
									   GTK_WINDOW( img->imagination_window ),
									   &entry, &vbox );

	/* If dialog is NULL, abort. */
	if( dialog == NULL )
		return;

	/* Add any export format specific GUI elements here */

	

	gtk_widget_show_all( dialog );

	/* Run dialog and abort if needed */
	if( gtk_dialog_run( GTK_DIALOG( dialog ) ) != GTK_RESPONSE_ACCEPT )
	{
		gtk_widget_destroy( dialog );
		img->export_is_running = 0;
		return;
	}

	/* User is serious, so we better prepare ffmpeg command line;) */
	img->export_fps = <frame rate>;
	filename = gtk_entry_get_text( entry );

	/* Any additional calculation can be placed here. */

	cmd_line = g_strdup_printf( "ffmpeg -f image2pipe -vcodec ppm -i pipe: "
								"-r %.02f -aspect %s -s %dx%d <#AUDIO#> -y "
								"-bf 2 -target %s-dvd \"%s.vob\"",
								img->export_fps, aspect_ratio,
								img->image_area->allocation.width,
								img->image_area->allocation.height,
								format, filename );
	img->export_cmd_line = cmd_line;

	/* Initiate stage 2 of export - audio processing */
	g_idle_add( (GSourceFunc)img_prepare_audio, img );

	gtk_widget_destroy( dialog );
}
#endif

static void
img_exporter_vob( img_window_struct *img )
{
	gchar          *cmd_line;
	gchar          *format;
	const gchar    *filename;
	gchar          *aspect_ratio;
	GtkWidget      *dialog;
	GtkEntry       *entry;
	GtkWidget      *vbox;

	/* Additional options - VOB only */
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *radio1, *radio2;

	/* This function call should be the first thing exporter does, since this
	 * function will take some preventive measures. */
	dialog = img_create_export_dialog( img, _("VOB export"),
									   GTK_WINDOW( img->imagination_window ),
									   &entry, &vbox );

	/* If dialog is NULL, abort. */
	if( dialog == NULL )
		return;

	/* Add any export format specific GUI elements here */
	frame = gtk_frame_new( NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 0 );

	label = gtk_label_new( _("<b>Television Format</b>") );
	gtk_label_set_use_markup( GTK_LABEL( label ), TRUE );
	gtk_frame_set_label_widget( GTK_FRAME( frame ), label );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), hbox );

	radio1 = gtk_radio_button_new_with_mnemonic( NULL, _("Normal 4:3") );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( radio1 ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), radio1, FALSE, FALSE, 0 );

	radio2 = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radio1 ), _("Widescreen 16:9") );
	gtk_box_pack_start( GTK_BOX( hbox ), radio2, FALSE, FALSE, 0 );

	gtk_widget_show_all( dialog );

	/* Run dialog and abort if needed */
	if( gtk_dialog_run( GTK_DIALOG( dialog ) ) != GTK_RESPONSE_ACCEPT )
	{
		gtk_widget_destroy( dialog );
		img->export_is_running = 0;
		return;
	}

	/* User is serious, so we better prepare ffmepg command line;) */
	format = img->image_area->allocation.height == 576 ? "pal" : "ntsc";
	img->export_fps = 30;
	filename = gtk_entry_get_text( entry );
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio1 ) ) )
		aspect_ratio = "4:3";
	else
		aspect_ratio = "16:9";

	cmd_line = g_strdup_printf( "ffmpeg -f image2pipe -vcodec ppm -i pipe: "
								"-r %.02f -aspect %s -s %dx%d <#AUDIO#> -y "
								"-bf 2 -target %s-dvd \"%s.vob\"",
								img->export_fps, aspect_ratio,
								img->image_area->allocation.width,
								img->image_area->allocation.height,
								format, filename );
	img->export_cmd_line = cmd_line;

	/* Initiate stage 2 of export - audio processing */
	g_idle_add( (GSourceFunc)img_prepare_audio, img );

	gtk_widget_destroy( dialog );
}

static void
img_exporter_ogg( img_window_struct *img )
{
	gchar          *cmd_line;
	const gchar    *filename;
	gchar          *aspect_ratio;
	GtkWidget      *dialog;
	GtkEntry       *entry;
	GtkWidget      *vbox;

	/* Additional options - OGG  only */
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *radio1, *radio2;
	GtkWidget *radios[3];
	gint       i;
	/* These values have been contributed by Jean-Pierre Redonnet.
	 * Thanks. */
	gint       qualities[] = { 512, 1024, 2048 };

	/* This function call should be the first thing exporter does, since this
	 * function will take some preventive measures. */
	dialog = img_create_export_dialog( img, _("OGG export"),
									   GTK_WINDOW( img->imagination_window ),
									   &entry, &vbox );

	/* If dialog is NULL, abort. */
	if( dialog == NULL )
		return;

	/* Add any export format specific GUI elements here */
	frame = gtk_frame_new( NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 0 );

	label = gtk_label_new( _("<b>Television Format</b>") );
	gtk_label_set_use_markup( GTK_LABEL( label ), TRUE );
	gtk_frame_set_label_widget( GTK_FRAME( frame ), label );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), hbox );

	radio1 = gtk_radio_button_new_with_mnemonic( NULL, _("Normal 4:3") );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( radio1 ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), radio1, FALSE, FALSE, 0 );

	radio2 = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radio1 ), _("Widescreen 16:9") );
	gtk_box_pack_start( GTK_BOX( hbox ), radio2, FALSE, FALSE, 0 );
	
	frame = gtk_frame_new( NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 0 );

	label = gtk_label_new( _("<b>Video Quality:</b>") );
	gtk_label_set_use_markup( GTK_LABEL( label ), TRUE );
	gtk_frame_set_label_widget( GTK_FRAME( frame ), label );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), hbox );

	radios[0] = gtk_radio_button_new_with_mnemonic( NULL, _("Low") );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[0], FALSE, FALSE, 0 );

	radios[1] = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radios[0] ), _("Medium") );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( radios[1] ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[1], FALSE, FALSE, 0 );

	radios[2] = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radios[0] ), _("High") );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[2], FALSE, FALSE, 0 );

	gtk_widget_show_all( dialog );

	/* Run dialog and abort if needed */
	if( gtk_dialog_run( GTK_DIALOG( dialog ) ) != GTK_RESPONSE_ACCEPT )
	{
		gtk_widget_destroy( dialog );
		img->export_is_running = 0;
		return;
	}

	/* User is serious, so we better prepare ffmpeg command line;) */
	img->export_fps = 30;
	filename = gtk_entry_get_text( entry );

	/* Any additional calculation can be placed here. */
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio1 ) ) )
		aspect_ratio = "4:3";
	else
		aspect_ratio = "16:9";

	for( i = 0; i < 3; i++ )
	{
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radios[i] ) ) )
			break;
	}

	cmd_line = g_strdup_printf( "ffmpeg -f image2pipe -vcodec ppm -i pipe: "
								"-r %.02f -aspect %s -s %dx%d <#AUDIO#> "
								"-vcodec libtheora -b %dk -acodec libvorbis "
								"-f ogg -y \"%s.ogv\"",
								img->export_fps, aspect_ratio,
								img->image_area->allocation.width,
								img->image_area->allocation.height,
								qualities[i], filename );
	img->export_cmd_line = cmd_line;

	/* Initiate stage 2 of export - audio processing */
	g_idle_add( (GSourceFunc)img_prepare_audio, img );

	gtk_widget_destroy( dialog );
}

static void
img_exporter_flv( img_window_struct *img )
{
	gchar          *cmd_line;
	const gchar    *filename;
	GtkWidget      *dialog;
	GtkEntry       *entry;
	GtkWidget      *vbox;

	/* Additional options - OGG  only */
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *radios[3];
	gint       i;
	/* These values have been contributed by Jean-Pierre Redonnet.
	 * Thanks. */
	gint       qualities[] = { 192, 384, 768 };

	/* This function call should be the first thing exporter does, since this
	 * function will take some preventive measures. */
	dialog = img_create_export_dialog( img, _("FLV export"),
									   GTK_WINDOW( img->imagination_window ),
									   &entry, &vbox );

	/* If dialog is NULL, abort. */
	if( dialog == NULL )
		return;

	/* Add any export format specific GUI elements here */
	frame = gtk_frame_new( NULL );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, FALSE, 0 );

	label = gtk_label_new( _("<b>Video Quality:</b>") );
	gtk_label_set_use_markup( GTK_LABEL( label ), TRUE );
	gtk_frame_set_label_widget( GTK_FRAME( frame ), label );

	hbox = gtk_hbox_new( TRUE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), hbox );

	radios[0] = gtk_radio_button_new_with_mnemonic( NULL, _("Low") );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[0], FALSE, FALSE, 0 );

	radios[1] = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radios[0] ), _("Medium") );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( radios[1] ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[1], FALSE, FALSE, 0 );

	radios[2] = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON( radios[0] ), _("High") );
	gtk_box_pack_start( GTK_BOX( hbox ), radios[2], FALSE, FALSE, 0 );

	gtk_widget_show_all( dialog );

	/* Run dialog and abort if needed */
	if( gtk_dialog_run( GTK_DIALOG( dialog ) ) != GTK_RESPONSE_ACCEPT )
	{
		gtk_widget_destroy( dialog );
		img->export_is_running = 0;
		return;
	}

	/* User is serious, so we better prepare ffmpeg command line;) */
	img->export_fps = 30;
	filename = gtk_entry_get_text( entry );

	/* Any additional calculation can be placed here. */
	for( i = 0; i < 3; i++ )
	{
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radios[i] ) ) )
			break;
	}

	cmd_line = g_strdup_printf( "ffmpeg -f image2pipe -vcodec ppm -i pipe: "
								"-r %.02f -b %dk -s 320x240 <#AUDIO#> "
								"-f flv -vcodec flv -acodec libmp3lame "
								"-ab 56000 -ar 22050 -ac 1 -y \"%s.flv\"",
								img->export_fps, qualities[i], filename );
	img->export_cmd_line = cmd_line;

	/* Initiate stage 2 of export - audio processing */
	g_idle_add( (GSourceFunc)img_prepare_audio, img );

	gtk_widget_destroy( dialog );
}

/* ****************************************************************************
 * End exporters
 * ************************************************************************* */
