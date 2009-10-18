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

#include "support.h"
#include <glib/gstdio.h>

static gboolean img_plugin_is_loaded(img_window_struct *, GModule *);

GtkWidget *img_load_icon(gchar *filename, GtkIconSize size)
{
    GtkWidget *file_image;
	gchar *path;
	GdkPixbuf *file_pixbuf = NULL;

	path = g_strconcat(DATADIR, "/imagination/pixmaps/",filename,NULL);
	file_pixbuf = gdk_pixbuf_new_from_file(path,NULL);
	g_free (path);

	if (file_pixbuf == NULL)
		file_image = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, size);
    else
	{
		file_image = gtk_image_new_from_pixbuf(file_pixbuf);
	    g_object_unref (file_pixbuf);
	}
    return file_image;
}

gchar *img_convert_seconds_to_time(gint total_secs)
{
	gint h, m, s;

	h =  total_secs / 3600;
	m = (total_secs % 3600) / 60;
	s =  total_secs - (h * 3600) - (m * 60);
	return g_strdup_printf("%02d:%02d:%02d", h, m, s);
}

GtkWidget *_gtk_combo_box_new_text(gboolean pointer)
{
	GtkWidget *combo_box;
	GtkCellRenderer *cell;
	GtkListStore *list;
	GtkTreeStore *tree;
	GtkTreeModel *model;

	if (pointer)
	{
		tree = gtk_tree_store_new (4, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
		model = GTK_TREE_MODEL( tree );

		combo_box = gtk_combo_box_new_with_model (model);
		g_object_unref (G_OBJECT( model ));
		cell = gtk_cell_renderer_pixbuf_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, FALSE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell, "pixbuf", 0, NULL);
	}
	else
	{
		list = gtk_list_store_new (1, G_TYPE_STRING);
		model = GTK_TREE_MODEL( list );
		
		combo_box = gtk_combo_box_new_with_model (model);
		g_object_unref (G_OBJECT( model ));
	}

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell, "text", pointer ? 1 : 0, NULL);
	g_object_set(cell, "ypad", (guint)0, NULL);
	return combo_box;
}

void img_set_statusbar_message(img_window_struct *img_struct, gint selected)
{
	gchar *message = NULL;
	gchar *total_slides = NULL;

	if (img_struct->slides_nr == 0)
	{
		message = g_strdup_printf(
				ngettext( "Welcome to Imagination - %d transition loaded.",
						  "Welcome to Imagination - %d transitions loaded.",
						  img_struct->nr_transitions_loaded ),
				img_struct->nr_transitions_loaded );
		gtk_statusbar_push( GTK_STATUSBAR( img_struct->statusbar ),
							img_struct->context_id, message );
		g_free( message );
		gtk_label_set_text( GTK_LABEL( img_struct->total_slide_number_label ), NULL );
	}
	else if (selected)
	{
		message = g_strdup_printf( ngettext( "%d slide selected",
											 "%d slides selected",
											 selected ), selected);
		gtk_statusbar_push( GTK_STATUSBAR( img_struct->statusbar ),
							img_struct->context_id, message );
		g_free( message );
	}
	else
	{
		total_slides = g_strdup_printf("%d",img_struct->slides_nr);
		gtk_label_set_text(GTK_LABEL(img_struct->total_slide_number_label),total_slides);
		message = g_strdup_printf( ngettext( "%d slide loaded %s",
											 "%d slides loaded %s",
											 img_struct->slides_nr ),
								   img_struct->slides_nr,
								   _(" - Use the CTRL key to select/unselect "
									 "or SHIFT for multiple select") );
		gtk_statusbar_push( GTK_STATUSBAR( img_struct->statusbar ),
							img_struct->context_id, message );
		g_free(total_slides);
		g_free(message);
	}
}

void img_load_available_transitions(img_window_struct *img)
{
	GDir          *dir;
	const gchar   *transition_name;
	gchar         *fname = NULL, *name, *filename;
	gchar        **trans, **bak;
	GModule       *module;
	GdkPixbuf     *pixbuf;
	GtkTreeIter    piter, citer;
	GtkTreeStore  *model;
	gpointer       address;
	gchar         *search_paths[3], **path;
	void (*plugin_set_name)(gchar **, gchar ***);

	model = GTK_TREE_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(img->transition_type)));
	
	/* Fill the combo box with no transition */
	gtk_tree_store_append(model, &piter, NULL);
	gtk_tree_store_set(model, &piter, 0, NULL, 1, _("None"), 2, NULL, 3, -1, -1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(img->transition_type), 0);

	/* Create NULL terminated array of paths that we'll be looking at */
#if PLUGINS_INSTALLED
	search_paths[0] = g_strconcat(PACKAGE_LIB_DIR,"/imagination",NULL);
#else
	search_paths[0] = g_strdup("./transitions");
#endif
	search_paths[1] = g_strconcat( g_get_home_dir(), "/.imagination/plugins", NULL );
	search_paths[2] = NULL;

	/* Search all paths listed in array */
	for( path = search_paths; *path; path++ )
	{
		dir = g_dir_open( *path, 0, NULL );
		if( dir == NULL )
		{
			g_free( *path );
			continue;
		}
		
		while( TRUE )
		{
			transition_name = g_dir_read_name( dir );
			if ( transition_name == NULL )
				break;
			
			fname = g_build_filename( *path, transition_name, NULL );
			module = g_module_open( fname, G_MODULE_BIND_LOCAL );
			if( module && img_plugin_is_loaded(img, module) == FALSE )
			{
				/* Obtain the name from the plugin function */
				g_module_symbol( module, "img_get_plugin_info",
								 (void *)&plugin_set_name);
				plugin_set_name( &name, &trans );
				
				/* Add group name to the store */
				gtk_tree_store_append( model, &piter, NULL );
				gtk_tree_store_set( model, &piter, 0, NULL, 1, name, 3, 0, -1 );
				img->plugin_list = g_slist_append(img->plugin_list, module);
				
				/* Add transitions */
				for( bak = trans; *trans; trans += 3 )
				{
#if PLUGINS_INSTALLED
					filename =
						g_strdup_printf( "%s/imagination/pixmaps/imagination-%d.png",
										 DATADIR, GPOINTER_TO_INT( trans[2] ) );
#else /* PLUGINS_INSTALLED */
					filename =
						g_strdup_printf( "./pixmaps/imagination-%d.png",
										 GPOINTER_TO_INT( trans[2] ) );
#endif /* ! PLUGINS_INSTALLED */

					pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

					/* Local plugins will fail to load images from system
					 * folder, so we'll try to load the from home folder. */
					if( ! pixbuf )
					{
						g_free( filename );
						filename =
							g_strdup_printf( "%s/.imagination/pixmaps/imagination-%d.png",
											 g_get_home_dir(),
											 GPOINTER_TO_INT( trans[2] ) );
						pixbuf = gdk_pixbuf_new_from_file( filename, NULL );
					}
					g_free( filename );
					g_module_symbol( module, trans[1], &address );
					gtk_tree_store_append( model, &citer, &piter );
					gtk_tree_store_set( model, &citer, 0, pixbuf,
													   1, trans[0],
													   2, address,
													   3, GPOINTER_TO_INT( trans[2] ),
													   -1 );
					img->nr_transitions_loaded++;
				}
				g_free( bak );
			}
			g_free( fname );
		}
		g_free( *path );
		g_dir_close( dir );
	}
}

static gboolean img_plugin_is_loaded(img_window_struct *img, GModule *module)
{
	return (g_slist_find(img->plugin_list,module) != NULL);
}

void img_show_file_chooser(SexyIconEntry *entry, SexyIconEntryPosition icon_pos,int button,img_window_struct *img)
{
	GtkWidget *file_selector;
	gchar *dest_dir;
	gint response;

	file_selector = gtk_file_chooser_dialog_new (_("Please choose the slideshow project filename"),
							GTK_WINDOW (img->imagination_window),
							GTK_FILE_CHOOSER_ACTION_SAVE,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_CANCEL,
							GTK_STOCK_SAVE,
							GTK_RESPONSE_ACCEPT,
							NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (file_selector),TRUE);
	response = gtk_dialog_run (GTK_DIALOG(file_selector));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		dest_dir = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (file_selector));
		gtk_entry_set_text(GTK_ENTRY(entry),dest_dir);
		g_free(dest_dir);
	}
	gtk_widget_destroy(file_selector);
}

slide_struct *
img_create_new_slide( void )
{
	slide_struct    *slide = NULL;

	slide = g_slice_new0( slide_struct );
	if( slide )
	{
		/* Still part */
		slide->duration = 1;

		/* Transition */
		slide->path = g_strdup( "0" );
		slide->transition_id = -1;
		slide->speed = NORMAL;

		/* Ken Burns */
		slide->cur_point = -1;

		/* Subtitles */
		slide->anim_duration = 1;
		slide->position = IMG_SUB_POS_MIDDLE_CENTER;
		slide->placing = IMG_REL_PLACING_EXPORTED_VIDEO;
		slide->font_desc = pango_font_description_from_string( "Sans 12" );
		slide->font_color[0] = 0; /* R */
		slide->font_color[1] = 0; /* G */
		slide->font_color[2] = 0; /* B */
		slide->font_color[3] = 1; /* A */
	}

	return( slide );
}

void
img_set_slide_file_info( slide_struct *slide,
						 const gchar  *filename )
{
	GdkPixbufFormat *format;
	gint             width,
					 height;

	format = gdk_pixbuf_get_file_info( filename, &width, &height );

	slide->o_filename = g_strdup( filename );
	slide->r_filename = g_strdup( filename );
	slide->angle = 0;
	
	slide->resolution = g_strdup_printf( "%d x %d", width, height );
	slide->type = gdk_pixbuf_format_get_name( format );
}

void
img_set_slide_gradient_info( slide_struct *slide,
							 gint          gradient,
							 gdouble      *start_color,
							 gdouble      *stop_color,
							 gdouble      *start_point,
							 gdouble      *stop_point )
{
	gint i;

	slide->gradient = gradient;
	for( i = 0; i < 3; i++ )
	{
		slide->g_start_color[i] = start_color[i];
		slide->g_stop_color[i]  = stop_color[i];
	}
	for( i = 0; i < 2; i++ )
	{
		slide->g_start_point[i] = start_point[i];
		slide->g_stop_point[i]  = stop_point[i];
	}
}

void
img_set_slide_still_info( slide_struct      *slide,
						  gint               duration,
						  img_window_struct *img )
{
	if( slide->duration != duration )
	{
		slide->duration = duration;

		if( ! img->total_dur_id )
			img->total_dur_id =
				g_idle_add( (GSourceFunc)img_set_total_slideshow_duration, img );
	}
}

void
img_set_slide_transition_info( slide_struct      *slide,
							   GtkListStore      *store,
							   GtkTreeIter       *iter,
							   GdkPixbuf         *pix,
							   const gchar       *path,
							   gint               transition_id,
							   ImgRender          render,
							   guint              speed,
							   img_window_struct *img )
{
	/* Set transition render. */
	if( path && ( slide->transition_id != transition_id ) )
	{
		if( slide->path )
			g_free( slide->path );

		slide->path = g_strdup( path );
		slide->transition_id = transition_id;
		slide->render = render;

		gtk_list_store_set( store, iter, 2, pix, -1 );
	}

	if( speed && ( slide->speed != speed ) )
	{
		slide->speed = speed;

		if( ! img->total_dur_id )
			img->total_dur_id =
				g_idle_add( (GSourceFunc)img_set_total_slideshow_duration, img );
	}
}

void
img_set_slide_ken_burns_info( slide_struct *slide,
							  gint          cur_point,
							  gsize         length,
							  gdouble      *points )
{
	ImgStopPoint *point;
	gint          i,
				  full;

	if( slide->no_points )
	{
		g_list_free( slide->points );
		slide->no_points = 0;
	}

	for( i = 0; i < length; i += 4 )
	{
		/* Create new point */
		point = g_slice_new( ImgStopPoint );
		point->time = (gint)( points[0 + i] + 0.5 );
		point->offx = points[1 + i];
		point->offy = points[2 + i];
		point->zoom = points[3 + i];
		
		/* Append it to the list */
		slide->points = g_list_append( slide->points, point );
		slide->no_points++;
	}

	slide->cur_point = CLAMP( cur_point, -1, slide->no_points - 1 );

	full = img_calc_slide_duration_points( slide->points,
										   slide->no_points );
	if( full )
		slide->duration = full;
}

void
img_set_slide_text_info( slide_struct      *slide,
						 GtkListStore      *store,
						 GtkTreeIter       *iter,
						 const gchar       *subtitle,
						 gint	            anim_id,
						 gint               anim_duration,
						 gint               position,
						 gint               placing,
						 const gchar       *font_desc,
						 gdouble           *font_color,
						 img_window_struct *img )
{
	/* Set the slide text info parameters */
	if( store && iter )
	{
		gboolean flag;

		if( slide->subtitle )
			g_free( slide->subtitle );
		slide->subtitle = g_strdup( subtitle );

		flag = ( subtitle ? TRUE : FALSE );
		gtk_list_store_set( store, iter, 3, flag, -1 );
	}

	if( ( anim_id > -1 ) && ( anim_id != slide->anim_id ) )
	{
		GtkTreeModel *model;
		gchar        *path;
		GtkTreeIter   iter;

		path = g_strdup_printf( "%d", anim_id );
		model = gtk_combo_box_get_model( GTK_COMBO_BOX( img->sub_anim ) );
		gtk_tree_model_get_iter_from_string( model, &iter, path );
		g_free( path );

		slide->anim_id = anim_id;
		gtk_tree_model_get( model, &iter, 1, &slide->anim, -1 );

		/* Sync timings */
		img_sync_timings( slide, img );
	}

	if( ( anim_duration > 0 ) && ( anim_duration != slide->anim_duration ) )
	{
		slide->anim_duration = anim_duration;

		/* Synchronize timings */
		img_sync_timings( slide, img );
	}

	if( ( position > -1 ) && ( position != slide->position ) )
		slide->position = position;

	if( ( placing > -1 ) && ( placing != slide->placing ) )
		slide->placing = placing;

	if( font_desc )
	{
		if( slide->font_desc )
			pango_font_description_free( slide->font_desc );
		slide->font_desc = pango_font_description_from_string( font_desc );
	}

	if( font_color )
	{
		slide->font_color[0] = font_color[0];
		slide->font_color[1] = font_color[1];
		slide->font_color[2] = font_color[2];
		slide->font_color[3] = font_color[3];
	}
}								

void
img_free_slide_struct( slide_struct *entry )
{
	GList *tmp;

	if( entry->angle != ANGLE_0 )
		g_unlink( entry->r_filename );
	g_free(entry->o_filename);
	g_free(entry->r_filename);
	g_free(entry->resolution);
	g_free(entry->type);
	
	/* Free stop point list */
	for( tmp = entry->points; tmp; tmp = g_list_next( tmp ) )
		g_slice_free( ImgStopPoint, tmp->data );
	g_list_free( entry->points );

	g_slice_free( slide_struct, entry );
}

gboolean
img_set_total_slideshow_duration( img_window_struct *img )
{
	gchar        *time;
	GtkTreeIter   iter;
	slide_struct *entry;
	GtkTreeModel *model;

	img->total_secs = 0;

	model = GTK_TREE_MODEL( img->thumbnail_model );
	if( gtk_tree_model_get_iter_first( model, &iter ) )
	{
		do
		{
			gtk_tree_model_get( model, &iter, 1, &entry, -1 );
			img->total_secs += entry->duration;
			
			if(entry->render)
				img->total_secs += entry->speed;
		}
		while( gtk_tree_model_iter_next( model, &iter ) );

		/* Add time of last pseudo slide */
		if( img->final_transition.render )
			img->total_secs += img->final_transition.speed;
	}
		
	time = img_convert_seconds_to_time(img->total_secs);
	gtk_label_set_text(GTK_LABEL (img->total_time_data),time);
	g_free(time);

	/* This is here only to be able to add this to idle source. */
	img->total_dur_id = 0;
	return( FALSE );
}

gint
img_calc_slide_duration_points( GList *list,
								gint   length )
{
	GList        *tmp;
	gint          i, duration = 0;
	ImgStopPoint *point;

	/* If we have no points, return 0 */
	if( length == 0 )
		return( 0 );

	/* Calculate length */
	for( tmp = list, i = 0; i < length; tmp = g_list_next( tmp ), i++ )
	{
		point = (ImgStopPoint *)tmp->data;
		duration += point->time;
	}

	return( duration );
}

/*
 * img_scale_image:
 *
 * This function should be called for all image loading needs. It'll properly
 * scale and trim loaded images, add borders if needed and return surface or
 * pixbuf of requested size.
 *
 * If one of the size requests is 0, the other one will be calculated from
 * first one with aspect ratio calculation. If both dimensions are 0, image
 * will be loaded from disk at original size (this is mainly used for export,
 * when we want to have images at their best quality).
 *
 * Return value: TRUE if image loading succeded, FALSE otherwise.
 */
gboolean
img_scale_image( const gchar      *filename,
				 gdouble           ratio,
				 gint              width,
				 gint              height,
				 gboolean          distort,
				 gdouble          *color,
				 GdkPixbuf       **pixbuf,
				 cairo_surface_t **surface )
{
	GdkPixbuf *loader;             /* Pixbuf used for loading */
	gint       i_width, i_height;  /* Image dimensions */
	gint       offset_x, offset_y; /* Offset values for borders */
	gdouble    i_ratio;            /* Export and image aspect ratios */
	gdouble    skew;               /* Transformation between ratio and
											 i_ratio */
	gboolean   transform = FALSE;  /* Flag that controls scalling */

	/* MAximal distortion values */
	gdouble max_stretch = 0.1280;
	gdouble max_crop    = 0.8500;

	/* Borderline skew values */
	gdouble max_skew = ( 1 + max_stretch ) / max_crop;
	gdouble min_skew = ( 1 - max_stretch ) * max_crop;

	/* Obtain information about image being loaded */
	if( ! gdk_pixbuf_get_file_info( filename, &i_width, &i_height ) )
		return( FALSE );

	/* How distorted images would be if we scaled them */
	i_ratio = (gdouble)i_width / i_height;
	skew = ratio / i_ratio;

	/* Calculationg surface dimensions.
	 *
	 * In order to be as flexible as possible, this function can load images at
	 * various sizes, but at aspect ration that matches the aspect ratio of main
	 * preview area. How size is determined? If width argument is not -1, this
	 * is taken as a reference dimension from which height is calculated (if
	 * height argument also present, it's ignored). If width argument is -1,
	 * height is taken as a reference dimension. If both width and height are
	 * -1, surface dimensions are calculated to to fit original image.
	 */
	if( width > 0 )
	{
		/* Calculate height according to width */
		height = width / ratio;
	}
	else if( height > 0 )
	{
		/* Calculate width from height */
		width = height * ratio;
	}
	else
	{
		/* Load image at maximum quality
		 *
		 * If the user doesn't want to have distorted images, we create slightly
		 * bigger surface that will hold borders too.
		 *
		 * If images should be distorted, we first check if we're able to fit
		 * image without distorting it too much. If images would be largely
		 * distorted, we simply load them undistorted.
		 *
		 * If we came all the way to  here, then we're able to distort image.
		 */
		if( ( ! distort )       || /* Don't distort */
			( skew > max_skew ) || /* Image is too wide */
			( skew < min_skew )  ) /* Image is too tall */
		{
			/* User doesn't want images to be distorted or distortion would be
			 * too intrusive. */
			if( ratio < i_ratio )
			{
				/* Borders will be added on left and right */
				width = i_width;
				height = width / ratio;
			}
			else
			{
				/* Borders will be added on top and bottom */
				height = i_height;
				width = height * ratio;
			}
		}
		else
		{
			/* User wants images to be distorted and we're able to do it
			 * without ruining images. */
			if( ratio < i_ratio )
			{
				/* Image will be distorted horizontally */
				height = i_height;
				width = height * ratio;
			}
			else
			{
				/* Image will be distorted vertically */
				width = i_width;
				height = width / ratio;
			}
		}
	}

	/* Will image be disotrted?
	 *
	 * Conditions:
	 *  - user allows us to do it
	 *  - skew is in sensible range
	 *  - image is not smaller than exported wideo size
	 */
	transform = distort && skew < max_skew && skew > min_skew &&
				( i_width >= width || i_height >= height );

	/* Load image into pixbuf at proper size */
	if( transform )
	{
		gint lw, lh;

		/* Images will be loaded at slightly modified dimensions */
		if( ratio < i_ratio )
		{
			/* Horizontal scaling */
			lw = (gdouble)width / ( skew + 1 ) * 2;
			lh = height;
		}
		else
		{
			/* Vertical scaling */
			lw = width;
			lh = (gdouble)height * ( skew + 1 ) / 2;
		}
		loader = gdk_pixbuf_new_from_file_at_scale( filename, lw, lh,
													FALSE, NULL );
	}
	else
	{
		/* Simply load image into pixbuf at size */
		loader = gdk_pixbuf_new_from_file_at_size( filename, width,
												   height, NULL );
	}
	if( ! loader )
		return( FALSE );

	i_width  = gdk_pixbuf_get_width( loader );
	i_height = gdk_pixbuf_get_height( loader );

	/* Calculate offsets */
	offset_x = ( width - i_width ) / 2;   /* CAN BE NEGATIVE!!! */
	offset_y = ( height - i_height ) / 2; /* CAN BE NEGATIVE!!! */

	/* Prepare output
	 *
	 * We can give two different output formats: cairo_surface_t and GdkPixbuf.
	 */
	if( pixbuf )
	{
		/* Create new pixbuf with loaded image */
		GdkPixbuf *tmp_pix;   /* Pixbuf used for loading */
		guint32    tmp_color; /* Background color */

		tmp_color = ( (gint)( color[0] * 0xff ) << 24 ) |
					( (gint)( color[1] * 0xff ) << 16 ) |
					( (gint)( color[2] * 0xff ) <<  8 );
		tmp_pix = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, width, height );
		gdk_pixbuf_fill( tmp_pix, tmp_color );
		gdk_pixbuf_composite( loader, tmp_pix,
							  MAX( 0, offset_x ), MAX( 0, offset_y ),
							  MIN( i_width, width ), MIN( i_height, height ),
							  offset_x, offset_y, 1, 1,
							  GDK_INTERP_BILINEAR, 255 );

		*pixbuf = tmp_pix;
	}
	if( surface )
	{
		/* Paint surface with loaded image
		 * 
		 * If image cannot be scalled, transform is FALSE. In this case, just
		 * borders are added. If transform is not 0, than scale image before
		 * painting it. */
		cairo_t         *cr;       /* Cairo, used to transform image */
		cairo_surface_t *tmp_surf; /* Surface to draw on */

		/* Create image surface with proper dimensions */
		tmp_surf = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
											   width, height );

		cr = cairo_create( tmp_surf );
		
		if( ! transform )
		{
			/* Fill with background color */
			cairo_set_source_rgb( cr, color[0], color[1], color[2] );
			cairo_paint( cr );
		}
		
		/* Paint image */
		gdk_cairo_set_source_pixbuf( cr, loader, offset_x, offset_y );
		cairo_paint( cr );
		
		cairo_destroy( cr );

		/* Return surface */
		*surface = tmp_surf;
	}

	/* Free temporary pixbuf */
	g_object_unref( G_OBJECT( loader ) );

	return( TRUE );
}

void
img_set_project_mod_state( img_window_struct *img,
						   gboolean           modified )
{
	if( ( img->project_is_modified ? modified : ! modified ) )
		return;

	img->project_is_modified = modified;

	/* FIXME: Do any updates here (add "*" to window title, ...). */
}

void
img_sync_timings( slide_struct      *slide,
				  img_window_struct *img )
{
	/* If times are already synchronized, return */
	if( slide->duration >= slide->anim_duration )
		return;

	/* Do the right thing;) */
	if( slide->no_points )
	{
		gint          diff;
		ImgStopPoint *point;

		/* Calculate difference that we need to accomodate */
		diff = slide->anim_duration - slide->duration;

		/* Elongate last point */
		point = (ImgStopPoint *)g_list_last( slide->points )->data;
		point->time += diff;
		
		/* Update Ken Burns display */
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( img->ken_duration ),
								   point->time );
	}

	/* Update display */
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( img->duration ),
							   slide->anim_duration );
}

void img_select_nth_slide(img_window_struct *img, gint slide_to_select)
{
	GtkTreePath *path;

	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->active_icon));
	path = gtk_tree_path_new_from_indices(slide_to_select, -1);
	gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->active_icon), path, NULL, FALSE);
	gtk_icon_view_select_path (GTK_ICON_VIEW (img->active_icon), path);
	gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->active_icon), path, FALSE, 0, 0);
	gtk_tree_path_free (path);
}

GdkPixbuf *
img_convert_surface_to_pixbuf( cairo_surface_t *surface )
{
	GdkPixbuf *pixbuf;
	gint       w, h, ss, sp, row, col;
	guchar    *data_s, *data_p;

	/* Information about surface */
	w = cairo_image_surface_get_width( surface );
	h = cairo_image_surface_get_height( surface );
	ss = cairo_image_surface_get_stride( surface );
	data_s = cairo_image_surface_get_data( surface );

	/* Create new pixbuf according to upper specs */
	pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, w, h );

	/* Get info about new pixbuf */
	sp = gdk_pixbuf_get_rowstride( pixbuf );
	data_p = gdk_pixbuf_get_pixels( pixbuf );

	/* Copy pixels */
	for( row = 0; row < h; row++ )
	{
		for( col = 0; col < w; col++ )
		{
			gint index_s, index_p;

			index_s = row * ss + col * 4;
			index_p = row * sp + col * 3;

			data_p[index_p + 0] = data_s[index_s + 2];
			data_p[index_p + 1] = data_s[index_s + 1];
			data_p[index_p + 2] = data_s[index_s + 0];
		}
	}

	return( pixbuf );
}

gboolean
img_scale_gradient( gint              gradient,
					gdouble          *p_start,
					gdouble          *p_stop,
					gdouble          *c_start,
					gdouble          *c_stop,
					gint              width,
					gint              height,
					GdkPixbuf       **pixbuf,
					cairo_surface_t **surface )
{
	cairo_surface_t *sf;
	cairo_t         *cr;
	cairo_pattern_t *pat;
	gdouble          diffx, diffy, radius;

	sf = cairo_image_surface_create( CAIRO_FORMAT_RGB24, width, height );
	cr = cairo_create( sf );

	switch( gradient )
	{
		case 0: /* Solid color */
			cairo_set_source_rgb( cr, c_start[0], c_start[1], c_start[2] );
			cairo_paint( cr );
			break;

		case 1: /* Linear gradient */
			pat = cairo_pattern_create_linear( p_start[0] * width,
											   p_start[1] * height,
											   p_stop[0] * width,
											   p_stop[1] * height );
			cairo_pattern_add_color_stop_rgb( pat, 0, c_start[0],
											  c_start[1], c_start[2] );
			cairo_pattern_add_color_stop_rgb( pat, 1, c_stop[0],
											  c_stop[1], c_stop[2] );
			cairo_set_source( cr, pat );
			cairo_paint( cr );
			cairo_pattern_destroy( pat );
			break;

		case 2: /* Radial gradient */
			diffx = ABS( p_start[0] - p_stop[0] ) * width;
			diffy = ABS( p_start[1] - p_stop[1] ) * height;
			radius = sqrt( pow( diffx, 2 ) + pow( diffy, 2 ) );

			pat = cairo_pattern_create_radial( p_start[0] * width,
											   p_start[1] * height, 0,
											   p_start[0] * width,
											   p_start[1] * height, radius );
			cairo_pattern_add_color_stop_rgb( pat, 0, c_start[0],
											  c_start[1], c_start[2] );
			cairo_pattern_add_color_stop_rgb( pat, 1, c_stop[0],
											  c_stop[1], c_stop[2] );
			cairo_set_source( cr, pat );
			cairo_paint( cr );
			cairo_pattern_destroy( pat );
			break;
	}
	cairo_destroy( cr );

	if( surface )
		*surface = sf;
	else
	{
		*pixbuf = img_convert_surface_to_pixbuf( sf );
		cairo_surface_destroy( sf );
	}

	return( TRUE );
}

