/*
** Copyright (c) 2009 Giuseppe Torelli <colossus73@gmail.com>
** Copyright (C) 2009 Tadej Borovšak   <tadeboro@gmail.com>
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

#ifndef __EXPORT_H__
#define __EXPORT_H__

#include <gtk/gtk.h>
#include "imagination.h"

/*Exporter is a structure that holds information about specific exporter. */
typedef struct _Exporter Exporter;
struct _Exporter
{
	gchar     *description;
	GCallback  func;  /* GCallback is defined as void (*func)( void ) */
};


gint
img_get_exporters_list( Exporter **exporters );

void
img_free_exporters_list( gint      no_exporters,
						 Exporter *exporters );

gboolean
img_stop_export( img_window_struct *img );

gboolean
img_prepare_pixbufs( img_window_struct *img,

					 gboolean           preview );
guint
img_calc_next_slide_time_offset( img_window_struct *img,
								 gdouble            rate );

void
img_render_transition_frame( img_window_struct *img );

void
img_render_still_frame( img_window_struct *img,
						gdouble            rate );

#endif
