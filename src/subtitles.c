/*
** Copyright (C) 2009 Tadej Borovšak <tadeboro@gmail.com>
** Copyright (C) 2010 Robert Chéramy <robert@cheramy.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "subtitles.h"
#include "support.h"

/* Border width around image (no text is placed there) */
#define BORDER 20

/* Wrap width for subtitles (fraction of image size) */
#define WRAP_WIDTH 0.75

/* ****************************************************************************
 * Local declarations
 * ************************************************************************* */
static void
img_calc_text_pos( gint      surface_w,
				   gint      surface_h,
				   gint      layout_w,
				   gint      layout_h,
				   ImgSubPos position,
				   gint     *posx,
				   gint     *posy );

static void
img_text_ani_fade( cairo_t     *cr,
				   PangoLayout *layout,
				   gint         sw,
				   gint         sh,
				   gint         lw,
				   gint         lh,
				   gint         posx,
				   gint         posy,
				   gdouble      progress,
				   gdouble     *font_color,
				   gdouble     *font_bgcolor);

static void
img_text_draw_layout( cairo_t     *cr,
                      PangoLayout *layout,
                      gint posx,
                      gint posy,
                      gdouble     *font_color,
                      gdouble     *font_bgcolor);

static void
img_text_from_left( cairo_t     *cr,
					PangoLayout *layout,
 					gint         sw,
 					gint         sh,
 					gint         lw,
 					gint         lh,
 					gint         posx,
 					gint         posy,
 					gdouble      progress,
                    gdouble     *font_color,
                    gdouble     *font_bgcolor);

static void
img_text_from_right( cairo_t     *cr,
					 PangoLayout *layout,
 					 gint         sw,
 					 gint         sh,
 					 gint         lw,
 					 gint         lh,
 					 gint         posx,
 					 gint         posy,
 					 gdouble      progress,
                     gdouble     *font_color,
                     gdouble     *font_bgcolor);

static void
img_text_from_top( cairo_t     *cr,
				   PangoLayout *layout,
				   gint         sw,
				   gint         sh,
				   gint         lw,
				   gint         lh,
				   gint         posx,
				   gint         posy,
				   gdouble      progress,
                   gdouble     *font_color,
                   gdouble     *font_bgcolor);

static void
img_text_from_bottom( cairo_t     *cr,
					  PangoLayout *layout,
					  gint         sw,
					  gint         sh,
  					  gint         lw,
  					  gint         lh,
  					  gint         posx,
  					  gint         posy,
  					  gdouble      progress,
                      gdouble     *font_color,
                      gdouble     *font_bgcolor);

static void
img_text_grow( cairo_t     *cr,
			   PangoLayout *layout,
			   gint         sw,
			   gint         sh,
			   gint         lw,
			   gint         lh,
			   gint         posx,
			   gint         posy,
			   gdouble      progress,
               gdouble     *font_color,
               gdouble     *font_bgcolor);

               
/* ****************************************************************************
 * Function definitions
 * ************************************************************************* */

/*
 * img_get_text_animation_list:
 * @animations: location to put list of available text animations
 *
 * This function is here to simplify accessing all available animations.
 *
 * Any newly added exporters should be listed in array returned by this function
 * or Imagination WILL NOT create combo box entries for them.
 *
 * List that is placed in exporters parameter should be considered read-only and
 * freed after usage with img_free_text_animation_list. If @animations is NULL,
 * only number of available animations is returned.
 *
 * Return value: Size of list in animations.
 */
gint
img_get_text_animation_list( TextAnimation **animations )
{
	TextAnimation *list;              /* List of all animations */
	gint           no_animations = 7; /* Number of animations */
	gint           i = 0;

	if( animations )
	{
		/* Populate list */
		/* DO NOT SHUFFLE THIS LIST! ONLY ADD NEW ANIMATIONS AT THE END OF THE
		 * LIST OR LOADING OF OLD PROJECTS WON'T WORK PROPERLY!!!! */
		list = g_slice_alloc( sizeof( TextAnimation ) * no_animations );

		/* No animation function (id = 0) */
		list[i].name   = g_strdup( _("None") );
		list[i].id     = i;
		list[i++].func = NULL;

		list[i].name   = g_strdup( _("Fade") );
		list[i].id     = i;
		list[i++].func = img_text_ani_fade;

		list[i].name   = g_strdup( _("Slide from left") );
		list[i].id     = i;
		list[i++].func = img_text_from_left;

		list[i].name   = g_strdup( _("Slide from right") );
		list[i].id     = i;
		list[i++].func = img_text_from_right;

		list[i].name   = g_strdup( _("Slide from top") );
		list[i].id     = i;
		list[i++].func = img_text_from_top;

		list[i].name   = g_strdup( _("Slide from bottom") );
		list[i].id     = i;
		list[i++].func = img_text_from_bottom;

		list[i].name   = g_strdup( _("Grow") );
		list[i].id     = i;
		list[i++].func = img_text_grow;

		/* FIXME: Add more animations here.
		 *
		 * DO NOT FORGET TO UPDATE no_animations VARIABLE AT THE TOP OF THIS
		 * FUNCTION WHEN ADDING NEW ANIMATIONS!! */
		
		*animations = list;
	}

	return( no_animations );
}

/*
 * img_free_text_animation_list:
 * @no_animations: number of animations in @animations
 * @animations: array of TextAnimation structs
 *
 * This function takes care of freeing any memory allocated by
 * img_get_text_animation_list function.
 */
void
img_free_text_animation_list( gint           no_animations,
							  TextAnimation *animations )
{
	register gint i;

	for( i = 0; i < no_animations; i++ )
		g_free( animations[i].name );

	g_slice_free1( sizeof( TextAnimation ) * no_animations, animations );
}

void
img_render_subtitle( cairo_t              *cr,
					 gint                  width,
					 gint                  height,
					 gdouble               zoom,
					 ImgSubPos             position,
					 ImgRelPlacing         placing,
					 gdouble               factor,
					 gdouble               offx,
					 gdouble               offy,
					 gchar                *subtitle,
					 PangoFontDescription *font_desc,
					 gdouble              *font_color,
                     gdouble              *font_bgcolor,
                     TextAnimationFunc     func,
					 gdouble               progress )
{
	gint		 lw,     /* Layout width */
				 lh,     /* Layout height */
				 posx,   /* Final subtitle position */
				 posy;
	PangoLayout *layout;

	/* Save cairo state */
	cairo_save( cr );

	/* Transform cairo context to get proper text measurements */
	if( placing == IMG_REL_PLACING_ORIGINAL_IMAGE )
	{
		cairo_scale( cr, factor * zoom, factor * zoom );
		cairo_translate( cr, offx / factor, offy / factor );
	}
	else
		cairo_scale( cr, zoom, zoom );

	/* Create pango layout and measure it */
	layout = pango_cairo_create_layout( cr );
	pango_layout_set_font_description( layout, font_desc );
	/* Disable wrapping
	pango_layout_set_wrap( layout, PANGO_WRAP_WORD );
	*/
	switch( position )
	{
		case IMG_SUB_POS_TOP_LEFT:
		case IMG_SUB_POS_MIDDLE_LEFT:
		case IMG_SUB_POS_BOTTOM_LEFT:
			pango_layout_set_alignment( layout, PANGO_ALIGN_LEFT );
			break;
			
		case IMG_SUB_POS_TOP_CENTER:
		case IMG_SUB_POS_MIDDLE_CENTER:
		case IMG_SUB_POS_BOTTOM_CENTER:
			pango_layout_set_alignment( layout, PANGO_ALIGN_CENTER );
			break;
			
		case IMG_SUB_POS_TOP_RIGHT:
		case IMG_SUB_POS_MIDDLE_RIGHT:
		case IMG_SUB_POS_BOTTOM_RIGHT:
			pango_layout_set_alignment( layout, PANGO_ALIGN_RIGHT );
			break;
	}
	pango_layout_set_text( layout, subtitle, -1 );
	pango_layout_get_size( layout, &lw, &lh );
	lw /= PANGO_SCALE;

	/* Disable wrapping
	if( lw > ( width * WRAP_WIDTH ) )
	{
		pango_layout_set_width( layout, width * WRAP_WIDTH * PANGO_SCALE );
		pango_layout_get_size( layout, &lw, &lh );
		lw /= PANGO_SCALE;
	}
	*/
	lh /= PANGO_SCALE;

	/* Calculate relative dimensions and final position of this subtitle */
	img_calc_text_pos( width, height, lw, lh, position, &posx, &posy );

	/* Do animation */
	if( func )
		(*func)( cr, layout, width, height, lw, lh, posx, posy, progress, font_color, font_bgcolor );
	else
	{
		/* No animation renderer */
        img_text_draw_layout(cr, layout, posx, posy, font_color, font_bgcolor);
	}

	/* Destroy layout */
	g_object_unref( G_OBJECT( layout ) );

	/* Restore cairo */
	cairo_restore( cr );
}

static void
img_calc_text_pos( gint      surface_w,
				   gint      surface_h,
				   gint      layout_w,
				   gint      layout_h,
				   ImgSubPos position,
				   gint     *posx,
				   gint     *posy )
{
	switch( position )
	{
		case IMG_SUB_POS_TOP_LEFT:
			*posx = BORDER;
			*posy = BORDER;
			break;

		case IMG_SUB_POS_TOP_CENTER:
			*posx = ( surface_w - layout_w ) / 2;
			*posy = BORDER;
			break;

		case IMG_SUB_POS_TOP_RIGHT:
			*posx = surface_w - BORDER - layout_w;
			*posy = BORDER;
			break;

		case IMG_SUB_POS_MIDDLE_LEFT:
			*posx = BORDER;
			*posy = ( surface_h - layout_h ) / 2;
			break;

		case IMG_SUB_POS_MIDDLE_CENTER:
			*posx = ( surface_w - layout_w ) / 2;
			*posy = ( surface_h - layout_h ) / 2;
			break;

		case IMG_SUB_POS_MIDDLE_RIGHT:
			*posx = surface_w - BORDER - layout_w;
			*posy = ( surface_h - layout_h ) / 2;
			break;

		case IMG_SUB_POS_BOTTOM_LEFT:
			*posx = BORDER;
			*posy = surface_h - BORDER - layout_h;
			break;

		case IMG_SUB_POS_BOTTOM_CENTER:
			*posx = ( surface_w - layout_w ) / 2;
			*posy = surface_h - BORDER - layout_h;
			break;

		case IMG_SUB_POS_BOTTOM_RIGHT:
			*posx = surface_w - BORDER - layout_w;
			*posy = surface_h - BORDER - layout_h;
			break;
	}
}


/* ****************************************************************************
 * Text animation renderers
 * ************************************************************************* */
static void
img_text_ani_fade( cairo_t     *cr,
				   PangoLayout *layout,
				   gint         sw,
				   gint         sh,
				   gint         lw,
				   gint         lh,
				   gint         posx,
				   gint         posy,
				   gdouble      progress,
                   gdouble     *font_color,
                   gdouble     *font_bgcolor)
{
    gdouble  progress_font_color[4], progress_font_bgcolor[4];

	/* Calculate colors */
    progress_font_color[0] = font_color[0];
    progress_font_color[1] = font_color[1];
    progress_font_color[2] = font_color[2];
    progress_font_color[3] = font_color[3] * progress;

    progress_font_bgcolor[0] = font_bgcolor[0];
    progress_font_bgcolor[1] = font_bgcolor[1];
    progress_font_bgcolor[2] = font_bgcolor[2];
    progress_font_bgcolor[3] = font_bgcolor[3] * pow(progress, 6);

    /* Paint text */
    img_text_draw_layout(cr, layout, posx, posy, progress_font_color, progress_font_bgcolor);
}

static void
img_text_draw_layout( cairo_t     *cr,
                      PangoLayout *layout,
                      gint         posx,
                      gint         posy,
                      gdouble     *font_color,
                      gdouble     *font_bgcolor)
{
    gint x,y;

    /* Draw the background border */
    cairo_set_source_rgba(cr, font_bgcolor[0],
                              font_bgcolor[1],
                              font_bgcolor[2],
                              font_bgcolor[3] );
    for (x=-1; x <=1; x++)
    {
        for (y=-1; y<=1; y++)
        {
            cairo_move_to( cr, posx + x, posy + y );
            pango_cairo_show_layout( cr, layout );
        }
    }

    /* Draw the subtitle */
    /* Set source color */
    cairo_set_source_rgba( cr, font_color[0],
                               font_color[1],
                               font_color[2],
                               font_color[3] );

    /* Move to proper place and paint text */
    cairo_move_to( cr, posx, posy );
    pango_cairo_show_layout( cr, layout );
}

static void
img_text_from_left( cairo_t     *cr,
					PangoLayout *layout,
 					gint         sw,
 					gint         sh,
 					gint         lw,
 					gint         lh,
 					gint         posx,
 					gint         posy,
 					gdouble      progress,
                    gdouble     *font_color,
                    gdouble     *font_bgcolor)
{
    img_text_draw_layout(cr, layout,
                         posx * progress - lw * ( 1 - progress ),
                         posy,
                         font_color, font_bgcolor);
}

static void
img_text_from_right( cairo_t     *cr,
					 PangoLayout *layout,
 					 gint         sw,
 					 gint         sh,
 					 gint         lw,
 					 gint         lh,
 					 gint         posx,
 					 gint         posy,
 					 gdouble      progress,
                     gdouble     *font_color,
                     gdouble     *font_bgcolor)
{
    img_text_draw_layout(cr, layout,
                         posx * progress + sw * ( 1 - progress ),
                         posy,
                         font_color, font_bgcolor);
}

static void
img_text_from_top( cairo_t     *cr,
				   PangoLayout *layout,
				   gint         sw,
				   gint         sh,
				   gint         lw,
				   gint         lh,
				   gint         posx,
				   gint         posy,
				   gdouble      progress,
                   gdouble     *font_color,
                   gdouble     *font_bgcolor)
{
    img_text_draw_layout(cr, layout,
                         posx,
                         posy * progress - lh * ( 1 - progress ),
                         font_color, font_bgcolor);
}

static void
img_text_from_bottom( cairo_t     *cr,
					  PangoLayout *layout,
					  gint         sw,
					  gint         sh,
  					  gint         lw,
  					  gint         lh,
  					  gint         posx,
  					  gint         posy,
  					  gdouble      progress,
                      gdouble     *font_color,
                      gdouble     *font_bgcolor)
{
    img_text_draw_layout(cr, layout,
                         posx,
                         posy * progress + sh * ( 1 - progress ),
                         font_color, font_bgcolor);
}

static void
img_text_grow( cairo_t     *cr,
			   PangoLayout *layout,
			   gint         sw,
			   gint         sh,
			   gint         lw,
			   gint         lh,
			   gint         posx,
			   gint         posy,
			   gdouble      progress,
               gdouble     *font_color,
               gdouble     *font_bgcolor)
{
	cairo_translate( cr, posx + lw * 0.5, posy + lh * 0.5 );
	cairo_scale( cr, progress, progress );

    img_text_draw_layout(cr, layout,
                         - lw * 0.5,
                         - lh * 0.5,
                         font_color, font_bgcolor);
}

