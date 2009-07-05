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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <gtk/gtk.h>
#include "audio.h"
#include "main-window.h"
#include "new_slideshow.h"
#include "slideshow_project.h"

void img_set_window_title(img_window_struct *,gchar *);
void img_new_slideshow(GtkMenuItem *,img_window_struct *);
void img_project_properties(GtkMenuItem *, img_window_struct *);
void img_add_slides_thumbnails(GtkMenuItem *,img_window_struct *);
void img_remove_audio_files (GtkWidget *, img_window_struct *);
void img_remove_foreach_func (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, GList **);
void img_delete_selected_slides(GtkMenuItem *,img_window_struct *);
void img_rotate_selected_slide(GtkWidget *, img_window_struct *);
void img_show_about_dialog (GtkMenuItem *,img_window_struct *);
void img_set_total_slideshow_duration(img_window_struct *);
void img_start_stop_preview(GtkWidget *, img_window_struct *);
void img_goto_first_slide(GtkWidget *, img_window_struct *);
void img_goto_prev_slide(GtkWidget *, img_window_struct *);
void img_goto_next_slide(GtkWidget *, img_window_struct *);
void img_goto_last_slide(GtkWidget *, img_window_struct *);
void img_on_drag_data_received (GtkWidget *,GdkDragContext *, int, int, GtkSelectionData *, unsigned int, unsigned int, img_window_struct *);
void img_start_stop_export(GtkWidget *, img_window_struct *);
void img_choose_slideshow_filename(GtkWidget *,img_window_struct *);
void img_close_slideshow(GtkWidget *,img_window_struct *);
void img_select_audio_files_to_add ( GtkMenuItem* , img_window_struct *);
void img_add_audio_files (gchar *, img_window_struct *);
void img_increase_progressbar(img_window_struct *, gint);
GSList *img_import_slides_file_chooser(img_window_struct *);
void img_free_allocated_memory(img_window_struct *);
gint img_ask_user_confirmation(img_window_struct *, gchar *);
gboolean img_quit_application(GtkWidget *, GdkEvent *, img_window_struct *);
GdkPixbuf *img_scale_pixbuf (img_window_struct *, gchar *);
void img_move_audio_up( GtkButton *, img_window_struct * );
void img_move_audio_down( GtkButton *, img_window_struct * );
gboolean img_on_expose_event(GtkWidget *,GdkEventExpose *,img_window_struct *);
#endif
