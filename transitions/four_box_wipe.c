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
	*group = "Four Box Wipe";

	*trans = g_new( gchar *, 7 );
	(*trans)[i++] = "Corners In";
	(*trans)[i++] = "img_corners_in";
	(*trans)[i++] = GINT_TO_POINTER( 13 );
	(*trans)[i++] = "Corners Out";
	(*trans)[i++] = "img_corners_out";
	(*trans)[i++] = GINT_TO_POINTER( 14 );
	(*trans)[i++] = NULL;
}

void
img_corners_in( GdkDrawable *window,
                GdkPixbuf   *image_from,
                GdkPixbuf   *image_to,
                gdouble      progress,
                gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void
img_corners_out( GdkDrawable *window,
                 GdkPixbuf   *image_from,
                 GdkPixbuf   *image_to,
                 gdouble      progress,
                 gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2 );
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
	gint             width, height;
	gint             w, h, x, y;

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

	gdk_cairo_set_source_pixbuf( cr, image_to, 0, 0 );
	w = width  * progress / 2;
	h = height * progress / 2;
	switch( type )
	{
		case 1:
			cairo_rectangle( cr, 0, 0, w, h );
			cairo_rectangle( cr, width - w, 0, w, h );
			cairo_rectangle( cr, 0, height - h , w, h );
			cairo_rectangle( cr, width - w, height - h, w, h );
			break;
		case 2:
			x = ( ( width  / 2 ) - w ) / 2;
			y = ( ( height / 2 ) - h ) / 2;
			cairo_rectangle( cr, x, y, w, h );
			cairo_rectangle( cr, width - w - x, y, w, h );
			cairo_rectangle( cr, x, height - h - y, w, h );
			cairo_rectangle( cr, width - w - x, height - h - y, w, h );
			break;
	}
	cairo_clip(cr );
	cairo_paint(cr);

	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
