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

#ifndef __IMAGINATION_SUPPORT_H
#define __IMAGINATION_SUPPORT_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include "imagination.h"
#include "sexy-icon-entry.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  define Q_(String) g_strip_context ((String), gettext (String))
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define Q_(String) g_strip_context ((String), (String))
#  define N_(String) (String)
#endif

GtkWidget *img_load_icon(gchar *, GtkIconSize );
gchar *img_convert_seconds_to_time(gint );
GtkWidget *_gtk_combo_box_new_text(gboolean);
void img_set_statusbar_message(img_window_struct *, gint);
void img_load_available_transitions(img_window_struct *);
void img_show_file_chooser(SexyIconEntry *, SexyIconEntryPosition, int, img_window_struct *);
GdkPixbuf *img_load_pixbuf_from_file(gchar *);
slide_struct *img_set_slide_info(gint , guint , void (*), gint, gchar *, gchar *);

GtkWidget *img_transition_combo_new( GtkTreeModel * );
GtkWidget *img_duration_combo_new( GtkTreeModel * );
#endif
