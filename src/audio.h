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

#ifndef __IMAGINATION_AUDIO_H
#define __IMAGINATION_AUDIO_H

#include <gtk/gtk.h>
#include <signal.h>
#include <errno.h>
#include <sox.h>
#include "support.h"

gchar *img_get_audio_length(img_window_struct *, gchar *, gint *);
void img_play_stop_selected_file(GtkButton *, img_window_struct *);
void output_message(unsigned , const char *, const char *, va_list ap);

void
img_analyze_input_files( gchar   **inputs,
						 gint      no_inputs,
						 gdouble  *rate,
						 gint     *channels );

gboolean
img_eliminate_bad_files( gchar             **inputs,
						 gint                no_inputs,
						 gdouble             rate,
						 gint                channels,
						 img_window_struct  *img );

void
img_update_inc_audio_display( img_window_struct *img );
#endif
