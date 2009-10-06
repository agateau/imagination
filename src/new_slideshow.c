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

#include "new_slideshow.h"

/* ****************************************************************************
 * Local declarations
 * ************************************************************************* */
static void
img_update_thumbs( img_window_struct *img );

static void
img_update_current_slide( img_window_struct *img );

/* ****************************************************************************
 * Public API
 * ************************************************************************* */
void img_new_slideshow_settings_dialog(img_window_struct *img, gboolean flag)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *vbox1;
	GtkWidget *main_frame;
	GtkWidget *alignment_main_frame;
	GtkWidget *vbox_frame1;
	GtkWidget *hbox_slideshow_options;
	GtkWidget *frame1;
	GtkWidget *label_frame1;
	GtkWidget *alignment_frame1;
	GtkWidget *ex_vbox;
	GtkWidget *ex_hbox;
	GtkWidget *frame3;
	GtkWidget *label_frame3;
	GtkWidget *alignment_frame3;
	GtkWidget *distort_button;
	GtkWidget *bg_button;
	GtkWidget *bg_label;
	GdkColor   color;
	GtkWidget *vbox_video_format;
	GtkWidget *pal,*ntsc;
	GtkWidget *label1;
	gint       response;
	gchar     *string;

	/* Display propert title depending on the callback that is calling this function. */
	string = ( flag ? _("Project properties") : _("Create a new slideshow") );
	dialog1 = gtk_dialog_new_with_buttons( string,
										GTK_WINDOW(img->imagination_window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);

	gtk_button_box_set_layout (GTK_BUTTON_BOX (GTK_DIALOG (dialog1)->action_area), GTK_BUTTONBOX_SPREAD);
	gtk_window_set_default_size(GTK_WINDOW(dialog1),520,-1);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

	dialog_vbox1 = gtk_dialog_get_content_area( GTK_DIALOG( dialog1 ) );
	vbox1 = gtk_vbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

	main_frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox1), main_frame, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (main_frame), GTK_SHADOW_IN);

	label1 = gtk_label_new (_("<b>Slideshow Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (main_frame), label1);
	gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);

	alignment_main_frame = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (main_frame), alignment_main_frame);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_main_frame), 5, 15, 10, 10);

	vbox_frame1 = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( alignment_main_frame ), vbox_frame1 );

	hbox_slideshow_options = gtk_hbox_new(TRUE, 10);
	gtk_box_pack_start(GTK_BOX (vbox_frame1), hbox_slideshow_options, TRUE, TRUE, 10);

	frame1 = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (hbox_slideshow_options), frame1, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);

	alignment_frame1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame1), alignment_frame1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_frame1), 5, 5, 5, 5);

	vbox_video_format = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (alignment_frame1), vbox_video_format);

	pal = gtk_radio_button_new_with_mnemonic (NULL, "PAL 720 x 576");
	gtk_box_pack_start (GTK_BOX (vbox_video_format), pal, TRUE, TRUE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pal), TRUE);

	ntsc = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (pal), "NTSC 720 x 480");
	gtk_box_pack_start (GTK_BOX (vbox_video_format), ntsc, TRUE, TRUE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pal), TRUE);

	label_frame1 = gtk_label_new (_("<b>Video Format</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame1), label_frame1);
	gtk_label_set_use_markup (GTK_LABEL (label_frame1), TRUE);

	frame3 = gtk_frame_new( NULL );
	gtk_box_pack_start (GTK_BOX (vbox_frame1), frame3, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_IN);

	alignment_frame3 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame3), alignment_frame3);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_frame3), 5, 5, 5, 5);

	label_frame3 = gtk_label_new (_("<b>Advanced Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame3), label_frame3);
	gtk_label_set_use_markup (GTK_LABEL (label_frame3), TRUE);

	ex_vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( alignment_frame3 ), ex_vbox );

	distort_button = gtk_check_button_new_with_label( _("Rescale images to fit desired aspect ratio") );
	gtk_box_pack_start( GTK_BOX( ex_vbox ), distort_button, FALSE, FALSE, 0 );

	ex_hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( ex_vbox ), ex_hbox, FALSE, FALSE, 0 );

	bg_label = gtk_label_new( _("Select background color:") );
	gtk_box_pack_start( GTK_BOX( ex_hbox ), bg_label, FALSE, FALSE, 0 );

	color.red   = img->background_color[0] * 0xffff;
	color.green = img->background_color[1] * 0xffff;
	color.blue  = img->background_color[2] * 0xffff;
	bg_button = gtk_color_button_new_with_color( &color );
	gtk_box_pack_start( GTK_BOX( ex_hbox ), bg_button, FALSE, FALSE, 0 );

	gtk_widget_show_all(dialog_vbox1);

	/* Set parameters */
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( distort_button ), img->distort_images );
	if (img->video_size[1] == 480)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ntsc), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pal), TRUE);

	response = gtk_dialog_run(GTK_DIALOG(dialog1));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		gboolean dist = img->distort_images;
		gint     size = img->video_size[1];
		GdkColor new;
		gboolean c_dist,
				 c_size,
				 c_color;

		/* Get distorsion settings */
		img->distort_images = 
			gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( distort_button ) );
		c_dist = ( dist ? ! img->distort_images : img->distort_images );

		/* Get format settings */
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pal ) ) )
			img->video_size[1] = 576;
		else
			img->video_size[1] = 480;
		img->video_ratio = (gdouble)img->video_size[0] / img->video_size[1];
		c_size = ( size != img->video_size[1] );

		/* Get color settings */
		gtk_color_button_get_color( GTK_COLOR_BUTTON( bg_button ), &new );
		img->background_color[0] = (gdouble)new.red   / 0xffff;
		img->background_color[1] = (gdouble)new.green / 0xffff;
		img->background_color[2] = (gdouble)new.blue  / 0xffff;
		c_color = ( color.red   != new.red   ) ||
				  ( color.green != new.green ) ||
				  ( color.blue  != new.blue  );

		/* Update display properly */
		if( c_dist || c_size || c_color )
		{
			/* Update thumbnails */
			img_update_thumbs( img );

			/* Update display of currently selected image */
			img_update_current_slide( img );

			/* Set indicator that project should be saved */
			img_set_project_mod_state( img, TRUE );

			/* Resize image area */
			if( c_size )
				gtk_widget_set_size_request(
							img->image_area,
							img->video_size[0] * img->image_area_zoom,
							img->video_size[1] * img->image_area_zoom );
		}
	}

	/* Destroy dialog */
	gtk_widget_destroy(dialog1);
}


/* ****************************************************************************
 * Local definitions
 * ************************************************************************* */
static void
img_update_thumbs( img_window_struct *img )
{
	gboolean      next;
	GtkTreeIter   iter;
	GtkListStore *store = img->thumbnail_model;
	GtkTreeModel *model = GTK_TREE_MODEL( store );

	for( next = gtk_tree_model_get_iter_first( model, &iter );
		 next;
		 next = gtk_tree_model_iter_next( model, &iter ) )
	{
		slide_struct *slide;
		GdkPixbuf    *pix;

		gtk_tree_model_get( model, &iter, 1, &slide, -1 );
		if( img_scale_image( slide->r_filename, img->video_ratio, 88, 0,
							 img->distort_images, img->background_color,
							 &pix, NULL ) )
		{
			gtk_list_store_set( store, &iter, 0, pix, -1 );
			g_object_unref( G_OBJECT( pix ) );
		}
	}
}

static void
img_update_current_slide( img_window_struct *img )
{
	if( ! img->current_slide )
		return;

	cairo_surface_destroy( img->current_image );
	img_scale_image( img->current_slide->r_filename, img->video_ratio,
					 0, img->video_size[1], img->distort_images,
					 img->background_color, NULL, &img->current_image );
	gtk_widget_queue_draw( img->image_area );
}

