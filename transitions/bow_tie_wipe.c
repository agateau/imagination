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
				   gint         direction );

/* Plug-in API */
void
img_get_plugin_info( gchar  **group,
					 gchar ***trans )
{
	gint i = 0;

	*group = "Bow Tie Wipe";

	*trans = g_new( gchar *, 7 );
	(*trans)[i++] = "Vertical";
	(*trans)[i++] = "img_vertical";
	(*trans)[i++] = GINT_TO_POINTER( 36 );
	(*trans)[i++] = "Horizontal";
	(*trans)[i++] = "img_horizontal";
	(*trans)[i++] = GINT_TO_POINTER( 37 );
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


/* Local functions definitions */
static void
transition_render( GdkDrawable *window,
				   GdkPixbuf   *image_from,
				   GdkPixbuf   *image_to,
				   gdouble      progress,
				   gint         file_desc,
				   gint         direction )
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	gint             width, height;
	gint             tmp;

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

	switch( direction )
	{
		case 1:	/* vertical */
			tmp = width / 2;
			cairo_move_to( cr, tmp * ( 1 - 2 * progress ), 0 );
			cairo_line_to( cr, tmp, height * progress );
			cairo_line_to( cr, tmp * ( 1 + 2 * progress ), 0 );
			cairo_close_path( cr );
			cairo_fill( cr );

			cairo_move_to( cr, tmp * ( 1 - 2 * progress ), height );
			cairo_line_to( cr, tmp, height * ( 1 - progress ) );
			cairo_line_to( cr, tmp * ( 1 + 2 * progress ), height );
			cairo_close_path( cr );
			cairo_fill( cr );
			break;

		case 2:	/* horizontal */
			tmp = height / 2;
			cairo_move_to( cr, 0, tmp * ( 1 - 2 * progress ) );
			cairo_line_to( cr, width * progress, tmp );
			cairo_line_to( cr, 0, tmp * ( 1 + 2 * progress ) );
			cairo_close_path( cr );
			cairo_fill( cr );

			cairo_move_to( cr, width, tmp * ( 1 - 2 * progress ) );
			cairo_line_to( cr, width * ( 1 - progress ), tmp );
			cairo_line_to( cr, width, tmp * ( 1 + 2 * progress ) );
			cairo_close_path( cr );
			cairo_fill( cr );
			break;
	}

	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
