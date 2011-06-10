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

/* Define video formats */
struct aspect_ratio aspect_ratio_list[] = {
    {"4:3", "4:3"},
    {"16:9", "16:9"},
    {NULL}
};

struct video_size VOB_size_list[] = {
    {"720 x 480 NTSC", 720, 480},
    {"720 x 576 PAL", 720, 576},
    {"1280 x 720 HD", 1280, 720},
    {"1920 x 1080 HD", 1920, 1080},
    {NULL}
};

struct video_fps VOB_fps_list[] = {
    {"30 (NTSC)", "30000/1001 -target ntsc-dvd", 30000/1001},
    {"25 (PAL)", "25 -target pal-dvd", 25},
    {NULL}
};

gchar *VOB_extensions[] = {
    ".vob",
    ".mpg",
    NULL
};

struct video_size OGV_size_list[] = {
    {"320 x 240 4:3", 320, 240},
    {"400 x 300", 400, 300},
    {"512 x 384", 512, 384},
    {"640 x 480", 640, 480},
    {"800 x 600", 800, 600},
    {"320 x 180 16:9", 320, 180},
    {"400 x 225", 400, 225},
    {"512 x 288", 512, 288},
    {"640 x 360", 640, 360},
    {"854 x 480", 854, 480},
    {NULL}
};

/* These values have been contributed by Jean-Pierre Redonnet. */
struct video_bitrate OGV_bitrate_list[] = {
    {"512 kbps (low)", 512*1024},
    {"1024 kbps (medium)", 1024*1024},
    {"2048 kbps (high)", 2048*1024},
    {NULL}
};

struct video_fps OGV_fps_list[] = {
    {"30", "30", 30},
    {NULL}
};

gchar *OGV_extensions[] = {
    ".ogv",
    NULL
};

struct video_size FLV_size_list[] = {
    {"320 x 240 4:3", 320, 240},
    {"400 x 300", 400, 300},
    {"512 x 384", 512, 384},
    {"640 x 480", 640, 480},
    {"800 x 600", 800, 600},
    {"320 x 180 16:9", 320, 180},
    {"400 x 225", 400, 225},
    {"512 x 288", 512, 288},
    {"640 x 360", 640, 360},
    {"854 x 480", 854, 480},
    {NULL}
};

struct video_bitrate FLV_bitrate_list[] = {
    {"384 kbps (low)", 384*1024},
    {"768 kbps (medium)", 768*1024},
    {"1536 kbps (high)", 1536*1024},
    {NULL}
};

gchar *FLV_extensions[] = {
    ".flv",
    NULL
};

struct video_size x3GP_size_list[] = {
    {"128 x 96", 128, 96},
    {"176 x 144", 176, 144},
    {"352 x 288", 352, 288},
    {"704 x 576", 704, 576},
    {"1408 x 1152", 1408, 1153},
    {NULL}
};

struct video_fps x3GP_fps_list[] = {
    {"25", "25", 25},
    {NULL}
};

gchar *x3GP_extensions[] = {
    ".3gp",
    NULL
};

struct video_format video_format_list[] = {
    /* name, config_name, video_format, ffmpeg_option, sizelist,
                                aspect_ratio_list, bitratelist */
    {"VOB (DVD Video)", "VOB", 'V',
        "-loglevel debug "
        "-bf 2 " /* use 2 B-Frames */
        /* target is set with aspect ratio: pal-dvd or ntsc-dvd */
        , VOB_size_list, aspect_ratio_list, NULL, VOB_fps_list,
        VOB_extensions
    }, /* FIXME Do add _() around every .name for i18n */

    {"OGV (Theora Vorbis)", "OGV", 'O',
        "-f ogg "
        "-vcodec libtheora -acodec libvorbis"
        , OGV_size_list, aspect_ratio_list, OGV_bitrate_list, OGV_fps_list,
        OGV_extensions
    },
        
    {"FLV (Flash Video)", "FLV", 'F',
        "-f flv -vcodec flv -acodec libmp3lame"
        "-ab 56000"     /* audio bitrate */
        "-ar 22050"     /* audio sampling frequency*/
        "-ac 1 "        /* number of audio channels */
        , FLV_size_list, aspect_ratio_list, FLV_bitrate_list, OGV_fps_list,
        FLV_extensions
    },
        
    {"3GP (Mobile Phones)", "3GP", '3',
        "-f 3gp -vcodec h263 -acodec libfaac"
        "-b 192k "                  /* bitrate */
        "-ab 32k -ar 8000 -ac 1"    /* audio bitrate, sampling frequency and number of channels */
        , x3GP_size_list, NULL, NULL, x3GP_fps_list,
        x3GP_extensions
    },
    {NULL}
};

/* ****************************************************************************
 * Local declarations
 * ************************************************************************* */
static void
img_update_thumbs( img_window_struct *img );

static void
img_update_current_slide( img_window_struct *img );

static void img_video_format_changed (GtkComboBox *combo, img_window_struct *img);


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
	GtkWidget *ex_vbox;
	GtkWidget *ex_hbox;
	GtkWidget *frame3;
	GtkWidget *label_frame3;
	GtkWidget *alignment_frame3;
	GtkWidget *distort_button;
	GtkWidget *bg_button;
	GtkWidget *bg_label;
	GdkColor   color;
	GtkWidget *label;
	gint       response;
    gint       i;
	gchar     *string;
    GtkTreeIter   iter;
    GtkListStore *store;
    GtkWidget   *table;

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

	label = gtk_label_new (_("<b>Slideshow Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (main_frame), label);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

	alignment_main_frame = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (main_frame), alignment_main_frame);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_main_frame), 5, 15, 10, 10);

    vbox_frame1 = gtk_vbox_new( FALSE, 10 );
    gtk_container_add( GTK_CONTAINER( alignment_main_frame ), vbox_frame1 );

    table = gtk_table_new(5, 2, FALSE);
    gtk_box_pack_start(GTK_BOX (vbox_frame1), table, TRUE, TRUE, 10);

    /* Video Format */
    label = gtk_label_new (_("Video Format:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);

    img->video_format_combo = _gtk_combo_box_new_text(FALSE);
    gtk_table_attach(GTK_TABLE(table), img->video_format_combo, 1, 2, 0, 1,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 2);
    store = GTK_LIST_STORE( gtk_combo_box_get_model(GTK_COMBO_BOX( img->video_format_combo ) ) );
    i = 0;
    while (video_format_list[i].name != NULL) {
        gtk_list_store_append( store, &iter );
        gtk_list_store_set( store, &iter, 0, video_format_list[i].name, -1 );
        i++;
	}

	/* Video Size */
    label = gtk_label_new (_("Video Size:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);

    img->video_size_combo = _gtk_combo_box_new_text(FALSE);
    gtk_table_attach(GTK_TABLE(table), img->video_size_combo, 1, 2, 1, 2,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 2);

    /* FPS */
    label = gtk_label_new( _("Frames per Second (FPS):") );
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);

    img->fps_combo = _gtk_combo_box_new_text(FALSE);
    gtk_table_attach(GTK_TABLE(table), img->fps_combo, 1, 2, 2, 3,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 2);

    /* Aspect Ratio */
    label = gtk_label_new( _("Aspect Ratio:") );
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);

    img->aspect_ratio_combo = _gtk_combo_box_new_text(FALSE);
    gtk_table_attach(GTK_TABLE(table), img->aspect_ratio_combo, 1, 2, 3, 4,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 2);

    /* Bitrate */
    label = gtk_label_new( _("Bitrate:") );
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);

    img->bitrate_combo = _gtk_combo_box_new_text(FALSE);
    gtk_table_attach(GTK_TABLE(table), img->bitrate_combo, 1, 2, 4, 5,
                              GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 2);

    /* Fill combobox depending on selected video format */
    g_signal_connect (G_OBJECT (img->video_format_combo), "changed", G_CALLBACK (img_video_format_changed),img);

	/* Advanced Settings */

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
	img_set_format_options(img);

	response = gtk_dialog_run(GTK_DIALOG(dialog1));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		gboolean dist = img->distort_images;
		GdkColor new;
		gboolean c_dist,
				 c_color;

		/* Get distorsion settings */
		img->distort_images = 
			gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( distort_button ) );
		c_dist = ( dist ? ! img->distort_images : img->distort_images );

		/* Get format settings */
		img_get_format_options(img);

		img->video_ratio = (gdouble)img->video_size[0] / img->video_size[1];

		/* Get color settings */
		gtk_color_button_get_color( GTK_COLOR_BUTTON( bg_button ), &new );
		img->background_color[0] = (gdouble)new.red   / 0xffff;
		img->background_color[1] = (gdouble)new.green / 0xffff;
		img->background_color[2] = (gdouble)new.blue  / 0xffff;
		c_color = ( color.red   != new.red   ) ||
				  ( color.green != new.green ) ||
				  ( color.blue  != new.blue  );

		/* Update display properly */
		if( c_dist || c_color )
		{
			/* Update thumbnails */
			img_update_thumbs( img );

			/* Update display of currently selected image */
			img_update_current_slide( img );

			/* Set indicator that project should be saved */
			img_set_project_mod_state( img, TRUE );
		}
		
		/* Adjust zoom level */
        img_zoom_fit(NULL, img);
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

static void img_video_format_changed (GtkComboBox *combo, img_window_struct *img)
{
	GtkTreeIter   iter;
	GtkListStore *store;
    gint          video_format, i;

	video_format = gtk_combo_box_get_active(combo);
    
    /* Video size */
    store = GTK_LIST_STORE( gtk_combo_box_get_model(GTK_COMBO_BOX( img->video_size_combo ) ) );
    gtk_list_store_clear(store);
    i = 0;
    while (video_format_list[video_format].sizelist[i].name != NULL) {
        gtk_list_store_append (store, &iter );
		gtk_list_store_set (store, &iter, 0,
                            video_format_list[video_format].sizelist[i].name, -1 );
        i++;
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(img->video_size_combo), 0);

    /* Aspect Ratio */
    if (video_format_list[video_format].aspect_ratio_list == NULL)
    {
        gtk_widget_set_sensitive(img->aspect_ratio_combo, FALSE);
        gtk_combo_box_set_active(GTK_COMBO_BOX(img->aspect_ratio_combo), -1);
    }
    else
    {
        gtk_widget_set_sensitive(img->aspect_ratio_combo, TRUE);
        store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(img->aspect_ratio_combo)));
        gtk_list_store_clear(store);
        i = 0;
        while (video_format_list[video_format].aspect_ratio_list[i].name != NULL) {
            gtk_list_store_append( store, &iter );
            gtk_list_store_set( store, &iter, 0,
                          video_format_list[video_format].aspect_ratio_list[i].name, -1 );
            i++;
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(img->aspect_ratio_combo), 0);
    }
    
    /* FPS */
    store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(img->fps_combo)));
    gtk_list_store_clear(store);
    i = 0;
    while (video_format_list[video_format].fps_list[i].name != NULL) {
        gtk_list_store_append( store, &iter );
        gtk_list_store_set( store, &iter, 0, video_format_list[video_format].fps_list[i].name, -1 );
        i++;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(img->fps_combo),0);


    /* Bitrate */
    if (video_format_list[video_format].bitratelist == NULL)
    {
        gtk_widget_set_sensitive(img->bitrate_combo, FALSE);        gtk_combo_box_set_active(GTK_COMBO_BOX(img->bitrate_combo),-1);
    }
    else
    {
        gtk_widget_set_sensitive(img->bitrate_combo, TRUE);
        store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(img->bitrate_combo)));
        gtk_list_store_clear(store);
        i = 0;
        while (video_format_list[video_format].bitratelist[i].name != NULL) {
            gtk_list_store_append( store, &iter );
            gtk_list_store_set( store, &iter, 0, video_format_list[video_format].bitratelist[i].name, -1 );
            i++;
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(img->bitrate_combo),0);
    }


}

void img_get_format_options(img_window_struct *img)
{
    gint    video_format_index, video_size_index;

    video_format_index = gtk_combo_box_get_active(GTK_COMBO_BOX(img->video_format_combo));
    video_size_index = gtk_combo_box_get_active(GTK_COMBO_BOX(img->video_size_combo));

    img->video_format_index = video_format_index;
    img->video_size[0] = video_format_list[video_format_index].sizelist[video_size_index].x;
    img->video_size[1] = video_format_list[video_format_index].sizelist[video_size_index].y;
    img->aspect_ratio_index = gtk_combo_box_get_active(GTK_COMBO_BOX(img->aspect_ratio_combo));
    img->bitrate_index = gtk_combo_box_get_active(GTK_COMBO_BOX(img->bitrate_combo));
    img->fps_index = gtk_combo_box_get_active(GTK_COMBO_BOX(img->fps_combo));
    img->export_fps = video_format_list[video_format_index].fps_list[img->fps_index].value;
}

void img_set_format_options(img_window_struct *img)
{
    gint i;
    struct video_size *size_list;

    /* Video format */
    gtk_combo_box_set_active(GTK_COMBO_BOX(img->video_format_combo), img->video_format_index);

    /* size list */
    size_list = video_format_list[img->video_format_index].sizelist;
    i = 0;
    while (size_list[i].name != NULL)
    {
        if (img->video_size[0] == size_list[i].x
            && img->video_size[1] == size_list[i].y)
        {
            gtk_combo_box_set_active(GTK_COMBO_BOX(img->video_size_combo), i);
            break;
        }
        i++;
    }
    if (size_list[i].name == NULL) {
        img_message(img, FALSE, "No video size found\n"); /* FIXME - add a custom Size option */
    }

    /* Aspect Ratio */
    if (video_format_list[img->video_format_index].aspect_ratio_list != NULL)
        gtk_combo_box_set_active(GTK_COMBO_BOX(img->aspect_ratio_combo), img->aspect_ratio_index);

    /* Bitrate */
    if (video_format_list[img->video_format_index].bitratelist != NULL)
        gtk_combo_box_set_active(GTK_COMBO_BOX(img->bitrate_combo), img->bitrate_index);
    
    /* FPS */
    gtk_combo_box_set_active(GTK_COMBO_BOX(img->fps_combo), img->fps_index);
}

