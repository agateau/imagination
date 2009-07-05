/*
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

#include "export_to_ppm.h"
#include <math.h>
#include <gdk/gdk.h>

/* Local functions declarations */
static void
transition_render( GdkDrawable *window,
				   GdkPixbuf   *image_from,
				   GdkPixbuf   *image_to,
				   gdouble      progress,
				   gint         file_desc,
				   gint         type );

/* Plug-in API */
void
img_get_plugin_info( gchar  **group,
					 gchar ***trans )
{
	gint i = 0;
	*group = "Barn Door Wipe";

	*trans = g_new( gchar *, 13 );
	(*trans)[i++] = "Vertical";
	(*trans)[i++] = "img_vertical";
	(*trans)[i++] = GINT_TO_POINTER( 15 );
	(*trans)[i++] = "Horizontal";
	(*trans)[i++] = "img_horizontal";
	(*trans)[i++] = GINT_TO_POINTER( 16 );
	(*trans)[i++] = "Diagonal Bottom Left";
	(*trans)[i++] = "img_diagonal_bottom_left";
	(*trans)[i++] = GINT_TO_POINTER( 17 );
	(*trans)[i++] = "Diagonal Top Left";
	(*trans)[i++] = "img_diagonal_top_left";
	(*trans)[i++] = GINT_TO_POINTER( 18 );
	(*trans)[i++] = NULL;
}

void
img_vertical( GdkDrawable *window,
              GdkPixbuf   *image_from,
              GdkPixbuf   *image_to,
              gdouble      progress,
              gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void
img_horizontal( GdkDrawable *window,
                GdkPixbuf   *image_from,
                GdkPixbuf   *image_to,
                gdouble      progress,
                gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2 );
}

void
img_diagonal_bottom_left( GdkDrawable *window,
                          GdkPixbuf   *image_from,
                          GdkPixbuf   *image_to,
                          gdouble      progress,
                          gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3 );
}

void
img_diagonal_top_left( GdkDrawable *window,
                       GdkPixbuf   *image_from,
                       GdkPixbuf   *image_to,
                       gdouble      progress,
                       gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 4 );
}

/* Local functions definitions */
static void
transition_render( GdkDrawable *window,
				   GdkPixbuf   *image_from,
				   GdkPixbuf   *image_to,
				   gdouble      progress,
				   gint         file_desc,
				   gint         type )
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	gint             width, height, dim;
	gdouble          diag;

	gdk_drawable_get_size( window, &width, &height );

	if( file_desc < 0 )
	{
		cr = gdk_cairo_create( window );
	}
	else
	{
		surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24,
											  width, height );
		cr = cairo_create( surface );
	}

	gdk_cairo_set_source_pixbuf( cr, image_from, 0, 0 );
	cairo_paint( cr );

	diag = sqrt( ( width * width ) + ( height * height ) );
	gdk_cairo_set_source_pixbuf( cr, image_to, 0, 0 );
	cairo_move_to( cr, width / 2 , height / 2 );
	switch( type )
	{
		case 1:
			dim = width;
			break;
		case 2:
			dim = height;
			cairo_rotate( cr, G_PI / 2 );
			break;
		case 3:
			dim = diag;
			cairo_rotate( cr, atan2( width, height ) );
			break;
		case 4:
			dim = diag;
			cairo_rotate( cr, atan2( width, -height ) );
			break;
	}
	cairo_rel_move_to( cr, ( dim * progress ) / 2, 0 );
	cairo_rel_line_to( cr, 0, - diag / 2 );
	cairo_rel_line_to( cr, - dim * progress, 0 );
	cairo_rel_line_to( cr, 0, diag );
	cairo_rel_line_to( cr, dim * progress, 0 );
	cairo_close_path( cr );
	cairo_clip( cr );
	cairo_paint(cr);

	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
