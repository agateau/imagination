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
 
#ifndef __IMAGINATION_H__
#define __IMAGINATION_H__

#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

/* Transition preview frame rate. I decided to use 25 fps, which
 * should be handled on time by most machines. */
#define PREVIEW_FPS 25

/* The transition speed is defined as a duration in seconds. */
#define	FAST	1
#define	NORMAL	4
#define	SLOW	8

#define comment_string	"Imagination Slideshow Project - http://imagination.sf.net"

typedef struct _plugin plugin;

struct _plugin
{
	const gchar	*name;		/* The name of the transition */
	gpointer	address;	/* The mem address of the routine */
};

typedef struct _slide_struct slide_struct;

struct _slide_struct
{
	gchar	*filename;
	gchar	*slide_original_filename;
	guint	duration;
	guint  	speed;
	gchar	*resolution;
	gchar	*type;
	gchar   *path;	/* transition model string path representation */
	gint     transition_id;
	void	(*render) (GdkDrawable*, GdkPixbuf*, GdkPixbuf*, gdouble, gint);
};

typedef struct _img_window_struct img_window_struct;

struct _img_window_struct
{
	/* Main GUI related variables */
	GtkWidget	*imagination_window;	// Main window
	GtkWidget	*open_menu;
	GtkWidget	*save_menu;
	GtkWidget	*save_as_menu;
//	GtkWidget	*properties_menu;
//	GtkWidget	*close_menu;
	GtkWidget	*open_button;
	GtkWidget	*save_button;
//	GtkWidget	*import_menu;
//	GtkWidget	*import_audio_menu;
//	GtkWidget	*remove_menu;
	GtkWidget   *rotate_left_menu;
	GtkWidget   *rotate_right_menu;
	GtkWidget   *rotate_left_button;
	GtkWidget   *rotate_right_button;
//	GtkWidget	*import_button;
//	GtkWidget	*import_audio_button;
//	GtkWidget	*remove_button;
	GtkWidget	*preview_menu;
	GtkWidget 	*preview_button;
//	GtkWidget   *export_menu;
//	GtkWidget   *export_button;
	GtkWidget	*transition_type;
	GtkWidget	*random_button;
	GtkWidget	*duration;				// Duration spin button
	GtkWidget	*stop_point_duration;
	GtkWidget	*trans_duration;
	//GtkWidget	*slide_selected_data;
	GtkWidget	*total_time_data;
	GtkWidget	*filename_data;
//	GtkWidget	*thumb_scrolledwindow;
	GtkTextBuffer *slide_text_buffer;
	GtkWidget	*text_animation_combo;
	GtkWidget	*scrolled_win;
	GtkWidget	*expand_button;
  	GtkWidget	*thumbnail_iconview;
  	GtkWidget	*statusbar;
  	GtkWidget	*progress_bar;
//	GtkWidget	*viewport;
  	GtkWidget	*image_area;
  	guint		context_id;
  	GtkListStore *thumbnail_model;
  	gchar		*current_dir;
	GSList		*rotated_files;

	/* Import slides dialog variables */
	GtkWidget	*dim_label;
	GtkWidget	*size_label;
  	GtkWidget	*preview_image;

	/* Renderers and module stuff */
  	gint		nr_transitions_loaded;
  	GSList		*plugin_list;

	/* Project related variables */
	gchar       *project_filename;		// project name for saving
	gboolean	distort_images;
	gboolean	project_is_modified;
	guint32     background_color;
  	gint		total_secs;
	gint		total_music_secs;
  	gint		slides_nr;
	slide_struct final_transition;  /* Only speed, render and duration fields
									   of this structure are used (and duration is
									   always 0). */

	/* Variables common to export and preview functions */
  	slide_struct *current_slide;
  	GdkPixbuf	*slide_pixbuf;
  	GdkPixbuf	*pixbuf1;
  	GdkPixbuf	*pixbuf2;
  	GtkTreeIter *cur_ss_iter;
  	guint		source_id;
	gdouble     progress;

	/* Preview related variables */
  	gboolean	preview_is_running;
  	GtkWidget	*import_slide_chooser;
	GtkWidget	*img_current_stop_point_entry;
	GtkWidget	*total_stop_points_label;
  	GtkWidget	*total_slide_number_label;
	GtkWidget	*slide_number_entry;

	/* Export dialog related stuff */
	gint        export_is_running;  /* 0 - export is not running
									     . no cleaning needed
									   1 - geting info from user
									     . no cleaning needed
									   2 - preparing audio
									     . terminate sox thread
										 . delete any created files
										 . free cmd_line
										 . free audio_file
									   3 - exporting video (pre-spawn)
									     . free cmd_line
										 . free audio_file
									   4 - exporting video (post-spawn)
									     . kill ffmpeg
										 . free cmd_line
										 . free audio_file
										 */
	gint        file_desc;
	guchar      *pixbuf_data;
	GtkWidget   *export_pbar1;
	GtkWidget   *export_pbar2;
	GtkWidget   *export_label;
	GtkWidget   *export_dialog;
	gdouble      export_fps;        /* Frame rate for exported video */
	gchar       *export_cmd_line;   /* ffmpeg spawn cmd line */
	gchar       *export_audio_file; /* Full path to audio */
	guint        export_frame_nr;	/* Total number of frames */
	guint        export_frame_cur;	/* Current frame */
	guint        export_slide_nr;	/* Number of frames fo current slide */
	guint        export_slide_cur;	/* Current slide frame */
	guint        export_slide;		/* Number of slide being exported */
	GSourceFunc  export_idle_func;	/* If TRUE, connect transition export, else
									   still export function. */
	GPid         ffmpeg_export;     /* ffmpeg's process id */
	GPid         sox_export;        /* sox's process id - FIXME: This will only
									   be needed if we decide to mangle audio
									   files using external sox binary. */
	
	/* Audio related stuff */
	GtkWidget	*music_file_treeview;
	GtkWidget	*play_audio_button;
	GtkWidget	*remove_audio_button;
	GtkListStore *music_file_liststore;
	GtkWidget	*music_time_data;
	GPid		play_child_pid;
};

#endif
