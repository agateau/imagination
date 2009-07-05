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

	*group = "Wave";

	*trans = g_new( gchar *, 25 );
	(*trans)[i++] = "Wave Down";
	(*trans)[i++] = "img_down";
	(*trans)[i++] = GINT_TO_POINTER( 48 );
	(*trans)[i++] = "Wave Up";
	(*trans)[i++] = "img_up";
	(*trans)[i++] = GINT_TO_POINTER( 49 );
	(*trans)[i++] = "Wave Left";
	(*trans)[i++] = "img_left";
	(*trans)[i++] = GINT_TO_POINTER( 50 );
	(*trans)[i++] = "Wave Right";
	(*trans)[i++] = "img_right";
	(*trans)[i++] = GINT_TO_POINTER( 51 );
	(*trans)[i++] = "Wave Left Down";
	(*trans)[i++] = "img_left_down";
	(*trans)[i++] = GINT_TO_POINTER( 52 );
	(*trans)[i++] = "Wave Left Up";
	(*trans)[i++] = "img_left_up";
	(*trans)[i++] = GINT_TO_POINTER( 53 );
	(*trans)[i++] = "Wave Right Up";
	(*trans)[i++] = "img_right_up";
	(*trans)[i++] = GINT_TO_POINTER( 54 );
	(*trans)[i++] = "Wave Right Down";
	(*trans)[i++] = "img_right_down";
	(*trans)[i++] = GINT_TO_POINTER( 55 );
	(*trans)[i++] = NULL;
}

void
img_down( GdkDrawable *window,
		  GdkPixbuf   *image_from, 
		  GdkPixbuf   *image_to,
		  gdouble      progress,
		  gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void
img_up( GdkDrawable *window,
		 GdkPixbuf   *image_from, 
		 GdkPixbuf   *image_to,
		 gdouble      progress,
		 gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 2 );
}


void
img_right( GdkDrawable *window,
			GdkPixbuf   *image_from, 
			GdkPixbuf   *image_to,
			gdouble      progress,
			gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 3 );
}

void
img_left( GdkDrawable *window,
		   GdkPixbuf   *image_from, 
		   GdkPixbuf   *image_to,
		   gdouble      progress,
		   gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 4 );
}

void
img_right_down( GdkDrawable *window,
			GdkPixbuf   *image_from, 
			GdkPixbuf   *image_to,
			gdouble      progress,
			gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 5 );
}

void
img_right_up( GdkDrawable *window,
			GdkPixbuf   *image_from, 
			GdkPixbuf   *image_to,
			gdouble      progress,
			gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 6 );
}

void
img_left_up( GdkDrawable *window,
			GdkPixbuf   *image_from, 
			GdkPixbuf   *image_to,
			gdouble      progress,
			gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 7 );
}

void
img_left_down( GdkDrawable *window,
			GdkPixbuf   *image_from, 
			GdkPixbuf   *image_to,
			gdouble      progress,
			gint         file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 8 );
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
	cairo_pattern_t *pattern;
	gint			width, height;
	gdouble 		wave_factor = 1.07;	// 50px
	gdouble 		stop;

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
		case 1:	/* down */
		pattern = cairo_pattern_create_linear( width / 2, 0, width / 2, height );
		break;

		case 2: /* up */
		pattern = cairo_pattern_create_linear( width / 2, height, width / 2, 0 );
		break;

		case 3: /* left */
		pattern = cairo_pattern_create_linear( width, height / 2, 0, height / 2 );
		break;

		case 4: /* right */
		pattern = cairo_pattern_create_linear( 0, height / 2, width, height / 2 );
		break;
		
		case 5: /* left down */
		pattern = cairo_pattern_create_linear( width, 0, 0, height );
		break;
		
		case 6: /* left up */
		pattern = cairo_pattern_create_linear( width, height, 0, 0 );
		break;
		
		case 7: /* right up */
		pattern = cairo_pattern_create_linear( 0, height , width, 0 );
		break;
		
		case 8: /* right down */
		pattern = cairo_pattern_create_linear( 0, 0, width, height );
		break;
	}
	/* Add color stops */
	cairo_pattern_add_color_stop_rgba(pattern, (1 - wave_factor) / 2, 0, 0, 0, 1 );	// start

	stop = progress * wave_factor - ( wave_factor - 1 );
	cairo_pattern_add_color_stop_rgba(pattern, stop, 0, 0, 0, 1 );

	stop = progress * wave_factor;
	cairo_pattern_add_color_stop_rgba(pattern, stop, 0, 0, 0, 0 );

	cairo_pattern_add_color_stop_rgba(pattern, wave_factor, 0, 0, 0, 0 ); // stop

	cairo_mask(cr, pattern);
	cairo_destroy(cr);
	cairo_pattern_destroy(pattern);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}
