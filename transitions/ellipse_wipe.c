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

#include "export_to_ppm.h"
#include <gdk/gdk.h>
#include <math.h>

#define AB 0.5				/* This controls the deformation */
#define BA ( 1 / ( AB ) )	/* Inverse value for vertical effects */

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

	*group = "Ellipse Wipe";

	*trans = g_new( gchar *, 19 );
	(*trans)[i++] = "Circle In";
	(*trans)[i++] = "img_circle_in";
	(*trans)[i++] = GINT_TO_POINTER( 20 );
	(*trans)[i++] = "Circle Out";
	(*trans)[i++] = "img_circle_out";
	(*trans)[i++] = GINT_TO_POINTER( 21 );
	(*trans)[i++] = "Horizontal In";
	(*trans)[i++] = "img_horizontal_in";
	(*trans)[i++] = GINT_TO_POINTER( 22 );
	(*trans)[i++] = "Horizontal Out";
	(*trans)[i++] = "img_horizontal_out";
	(*trans)[i++] = GINT_TO_POINTER( 23 );
	(*trans)[i++] = "Vertical In";
	(*trans)[i++] = "img_vertical_in";
	(*trans)[i++] = GINT_TO_POINTER( 24 );
	(*trans)[i++] = "Vertical Out";
	(*trans)[i++] = "img_vertical_out";
	(*trans)[i++] = GINT_TO_POINTER( 25 );
	(*trans)[i++] = NULL;
}

void img_circle_in( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void img_circle_out( GdkDrawable *window, GdkPixbuf *image_to, GdkPixbuf *image_from, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2 );
}

void img_horizontal_in( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3 );
}

void img_horizontal_out( GdkDrawable *window, GdkPixbuf *image_to, GdkPixbuf *image_from, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 4 );
}

void img_vertical_in( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 5 );
}

void img_vertical_out( GdkDrawable *window, GdkPixbuf *image_to, GdkPixbuf *image_from, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 6 );
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
	gint            width, height;
	gdouble			radius = 470, j;

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
	j = (gdouble)height / width;
	switch (direction)
	{
		case 1:
		cairo_arc(cr, width / 2.0, height / 2.0, radius * progress, 0, 2 * G_PI);
		break;
		
		case 2:
		cairo_arc(cr, width / 2.0, height / 2.0, radius * (1 - progress), 0, 2 * G_PI);
		break;
		
		case 3:
		radius = width / ( 2 * AB ) * sqrt( ( AB * AB ) + ( j * j ) );
		cairo_save(cr);
    	cairo_translate(cr,width / 2.0, height / 2.0);
    	cairo_scale(cr, 1, AB);
    	cairo_arc (cr, 0, 0, radius * progress, 0, 2 * G_PI);
    	cairo_restore(cr);
		break;
		
		case 4:
		radius = width / ( 2 * AB ) * sqrt( ( AB * AB ) + ( j * j ) );
		cairo_save(cr);
    	cairo_translate(cr,width / 2.0, height / 2.0);
    	cairo_scale(cr, 1, AB);
    	cairo_arc (cr, 0, 0, radius * (1 - progress), 0, 2 * G_PI);
    	cairo_restore(cr);
		break;
		
		case 5:
		radius = width / ( 2 * BA ) * sqrt( ( BA * BA ) + ( j * j ) );
		cairo_save(cr);
    	cairo_translate(cr,width / 2.0, height / 2.0);
    	cairo_scale(cr, 1, BA);
    	cairo_arc (cr, 0, 0, radius * progress, 0, 2 * G_PI);
    	cairo_restore(cr);
		break;
		
		case 6:
		radius = width / ( 2 * BA ) * sqrt( ( BA * BA ) + ( j * j ) );
		cairo_save(cr);
    	cairo_translate(cr,width / 2.0, height / 2.0);
    	cairo_scale(cr, 1, BA );
    	cairo_arc (cr, 0, 0, radius * (1 - progress), 0, 2 * G_PI);
    	cairo_restore(cr);
		break;
	}
	cairo_clip(cr);
   	cairo_paint(cr);

	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
