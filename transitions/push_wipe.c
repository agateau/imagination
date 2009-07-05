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

	*group = "Push Wipe";

	*trans = g_new( gchar *, 13 );
	(*trans)[i++] = "From Left";
	(*trans)[i++] = "img_from_left";
	(*trans)[i++] = GINT_TO_POINTER( 26 );
	(*trans)[i++] = "From Right";
	(*trans)[i++] = "img_from_right";
	(*trans)[i++] = GINT_TO_POINTER( 27 );
	(*trans)[i++] = "From Top";
	(*trans)[i++] = "img_from_top";
	(*trans)[i++] = GINT_TO_POINTER( 28 );
	(*trans)[i++] = "From Bottom";
	(*trans)[i++] = "img_from_bottom";
	(*trans)[i++] = GINT_TO_POINTER( 29 );
	(*trans)[i++] = NULL;
}

void img_from_left( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void img_from_right( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2 );
}

void img_from_top( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3 );
}

void img_from_bottom( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
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
				   gint         direction )
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	gint            width, height;

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

	switch (direction)
	{
		case 1:
		gdk_cairo_set_source_pixbuf(cr,image_to,-width * (1- progress),0);
		break;
		
		case 2:
		gdk_cairo_set_source_pixbuf(cr,image_from,-width * progress,0);
		break;
		
		case 3:
		gdk_cairo_set_source_pixbuf(cr,image_from,0,height * progress);
		break;
		
		case 4:
		gdk_cairo_set_source_pixbuf(cr,image_from,0,-height * progress);
		break;
	}
	cairo_paint( cr );

	switch (direction)
	{
		case 1:
		gdk_cairo_set_source_pixbuf(cr,image_from,width * progress,0);
		break;
		
		case 2:
		gdk_cairo_set_source_pixbuf(cr,image_to,width * (1 - progress),0);
		break;
		
		case 3:
		gdk_cairo_set_source_pixbuf(cr,image_to,0,-height * (1 - progress));
		break;
		
		case 4:
		gdk_cairo_set_source_pixbuf(cr,image_to,0,height * (1 - progress));
		break;
	}
   	cairo_paint(cr);
	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
