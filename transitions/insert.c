/*
 *  Copyright (c) 2009 Jean-Pierre Redonnet <inphilly@gmail.com>
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
					 
static void
transition2_render( GdkDrawable *window,
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

	*group = "Insert";

	*trans = g_new( gchar *, 22 );
	(*trans)[i++] = "Clockwise";
	(*trans)[i++] = "img_spin_insert";
	(*trans)[i++] = GINT_TO_POINTER( 60 );
	(*trans)[i++] = "Counter Clockwise";
	(*trans)[i++] = "img_spininv_insert";
	(*trans)[i++] = GINT_TO_POINTER( 61 );
	(*trans)[i++] = "From Center";
	(*trans)[i++] = "img_simple_insert";
	(*trans)[i++] = GINT_TO_POINTER( 62 );
	(*trans)[i++] = "From Top Left";
	(*trans)[i++] = "img_upleft_insert";
	(*trans)[i++] = GINT_TO_POINTER( 63 );
	(*trans)[i++] = "From Top Right";
	(*trans)[i++] = "img_upright_insert";
	(*trans)[i++] = GINT_TO_POINTER( 64 );
	(*trans)[i++] = "From Bottom Left";
	(*trans)[i++] = "img_downleft_insert";
	(*trans)[i++] = GINT_TO_POINTER( 65 );
	(*trans)[i++] = "From Bottom Right";
	(*trans)[i++] = "img_downright_insert";
	(*trans)[i++] = GINT_TO_POINTER( 66 );
	(*trans)[i++] = NULL;
	
}

void img_spin_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 1 );
}

void img_spininv_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, -1 );
}

void img_simple_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition_render( window, image_from, image_to, progress, file_desc, 0 );
}

void img_upleft_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition2_render( window, image_from, image_to, progress, file_desc, 0 );
}

void img_upright_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition2_render( window, image_from, image_to, progress, file_desc, 1 );
}

void img_downleft_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition2_render( window, image_from, image_to, progress, file_desc, 2 );
}

void img_downright_insert( GdkDrawable *window, GdkPixbuf *image_from, GdkPixbuf *image_to, gdouble progress, gint file_desc )
{
	transition2_render( window, image_from, image_to, progress, file_desc, 3 );
}

/* Local functions definitions */
/*Rotation & insertion*/
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
	

	cairo_translate (cr, 0.5*width, 0.5*height);
	if (direction==1) 
		cairo_rotate (cr, 2*G_PI*progress);
	else if (direction==-1) 
		cairo_rotate (cr, 2 * G_PI * ( 1 - progress ));	

	cairo_scale  (cr, progress, progress);
	cairo_translate (cr, -0.5*width, -0.5*height);
	
	gdk_cairo_set_source_pixbuf( cr, image_to, 0, 0 );
	cairo_paint( cr );
	
	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}

/* Linear translation & insertion*/
static void
transition2_render( GdkDrawable *window,
				   GdkPixbuf   *image_from,
				   GdkPixbuf   *image_to,
				   gdouble      progress,
				   gint         file_desc,
				   gint         direction )
{
	cairo_t         *cr;
	cairo_surface_t *surface;
	gint             width, height;

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
	
	/*up - left*/
	if (direction == 0) {
		cairo_translate (cr, progress/2*width, progress/2*height);
		cairo_scale  (cr, progress, progress);
		cairo_translate (cr,-progress/2*width, -progress/2*height);
	}
	else
	/*up - right*/
	if (direction == 1) {
		cairo_translate (cr, width-progress/2*width, progress/2*height);
		cairo_scale  (cr, progress, progress);
		cairo_translate (cr, progress/2*width-width, -progress/2*height);
	}
	else
		/*down - left*/
	if (direction == 2) {
		cairo_translate (cr, progress/2*width, height-progress/2*height);
		cairo_scale  (cr, progress, progress);
		cairo_translate (cr, -progress/2*width, progress/2*height-height);
	}
	else
	/*down - right*/
	if (direction == 3) {
		cairo_translate (cr, width-progress/2*width, height-progress/2*height);
		cairo_scale  (cr, progress, progress);
		cairo_translate (cr,progress/2*width-width, progress/2*height-height);
	}
	
	gdk_cairo_set_source_pixbuf( cr, image_to, 0, 0 );
	cairo_paint( cr );
	
	cairo_destroy(cr);

	if(file_desc < 0)
		return;

	img_export_cairo_to_ppm(surface, file_desc);
	cairo_surface_destroy(surface);
}

