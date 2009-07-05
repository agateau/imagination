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
				   gdouble      initial_rotation,
				   gint         direction );

/* Plug-in API */
void
img_get_plugin_info( gchar  **group,
					 gchar ***trans )
{
	gint i = 0;
	*group = "Clock Wipe";

	*trans = g_new( gchar *, 25 );
	(*trans)[i++] = "Clockwise Twelve";
	(*trans)[i++] = "img_ctw";
	(*trans)[i++] = GINT_TO_POINTER( 40 );
	(*trans)[i++] = "Counter Clockwise Twelve";
	(*trans)[i++] = "img_cctw";
	(*trans)[i++] = GINT_TO_POINTER( 41 );
	(*trans)[i++] = "Clockwise Three";
	(*trans)[i++] = "img_cth";
	(*trans)[i++] = GINT_TO_POINTER( 42 );
	(*trans)[i++] = "Counter Clockwise Three";
	(*trans)[i++] = "img_ccth";
	(*trans)[i++] = GINT_TO_POINTER( 43 );
	(*trans)[i++] = "Clockwise Six";
	(*trans)[i++] = "img_csi";
	(*trans)[i++] = GINT_TO_POINTER( 44 );
	(*trans)[i++] = "Counter Clockwise Six";
	(*trans)[i++] = "img_ccsi";
	(*trans)[i++] = GINT_TO_POINTER( 45 );
	(*trans)[i++] = "Clockwise Nine";
	(*trans)[i++] = "img_cni";
	(*trans)[i++] = GINT_TO_POINTER( 46 );
	(*trans)[i++] = "Counter Clockwise Nine";
	(*trans)[i++] = "img_ccni";
	(*trans)[i++] = GINT_TO_POINTER( 47 );
	(*trans)[i++] = NULL;
}

void
img_ctw( GdkDrawable *window,
         GdkPixbuf   *image_from,
         GdkPixbuf   *image_to,
         gdouble      progress,
         gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3, 1 );
}

void
img_cth( GdkDrawable *window,
         GdkPixbuf   *image_from,
         GdkPixbuf   *image_to,
         gdouble      progress,
         gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 0, 1 );
}

void
img_csi( GdkDrawable *window,
         GdkPixbuf   *image_from,
         GdkPixbuf   *image_to,
         gdouble      progress,
         gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1, 1 );
}

void
img_cni( GdkDrawable *window,
         GdkPixbuf   *image_from,
         GdkPixbuf   *image_to,
         gdouble      progress,
         gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2, 1 );
}

void
img_cctw( GdkDrawable *window,
          GdkPixbuf   *image_from,
          GdkPixbuf   *image_to,
          gdouble      progress,
          gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3, -1 );
}

void
img_ccth( GdkDrawable *window,
          GdkPixbuf   *image_from,
          GdkPixbuf   *image_to,
          gdouble      progress,
          gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 0, -1 );
}

void
img_ccsi( GdkDrawable *window,
          GdkPixbuf   *image_from,
          GdkPixbuf   *image_to,
          gdouble      progress,
          gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1, -1 );
}

void
img_ccni( GdkDrawable *window,
          GdkPixbuf   *image_from,
          GdkPixbuf   *image_to,
          gdouble      progress,
          gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2, -1 );
}


/* Local functions definitions */
static void
transition_render( GdkDrawable *window,
				   GdkPixbuf   *image_from,
				   GdkPixbuf   *image_to,
				   gdouble      progress,
				   gint         file_desc,
				   gdouble      initial_rotation,
				   gint         direction )
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	gint             width, height;
	gdouble          begin, end;
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

	begin = initial_rotation / 2 * G_PI;
	end   = begin + direction * progress * 2 * G_PI;
	if( begin < end )
		cairo_arc( cr, width / 2, height / 2, diag, begin, end );
	else
		cairo_arc( cr, width / 2, height / 2, diag, end, begin );

	cairo_close_path( cr );
	cairo_fill( cr );

	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
