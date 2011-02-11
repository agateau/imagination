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
#define PREVIEW_FPS_STEP       5
#define PREVIEW_FPS_MIN        5
#define PREVIEW_FPS_NO_PRESETS 6
#define PREVIEW_FPS_DEFAULT    15

/* The transition speed is defined as a duration in seconds. */
#define	FAST	1
#define	NORMAL	4
#define	SLOW	8

#define old_comment_string \
	"Imagination Slideshow Project - http://imagination.sf.net"
#define comment_string \
	"Imagination 2.0 Slideshow Project - http://imagination.sf.net"


/* ****************************************************************************
 * Subtitles related definitions
 * ************************************************************************* */
/* Enum that holds all available positions of text. */
typedef enum
{
	IMG_SUB_POS_TOP_LEFT = 0,
	IMG_SUB_POS_TOP_CENTER,
	IMG_SUB_POS_TOP_RIGHT,
	IMG_SUB_POS_MIDDLE_LEFT,
	IMG_SUB_POS_MIDDLE_CENTER,
	IMG_SUB_POS_MIDDLE_RIGHT,
	IMG_SUB_POS_BOTTOM_LEFT,
	IMG_SUB_POS_BOTTOM_CENTER,
	IMG_SUB_POS_BOTTOM_RIGHT
}
ImgSubPos;

/* Enum with relative placings */
typedef enum
{
	IMG_REL_PLACING_EXPORTED_VIDEO = 0,
	IMG_REL_PLACING_ORIGINAL_IMAGE
}
ImgRelPlacing;

typedef enum
{
	ANGLE_0 = 0,
	ANGLE_90,
	ANGLE_180,
	ANGLE_270
}
ImgAngle;

/*
 * TextAnimationFunc:
 * @cr: cairo context that should be used for drawing
 * @layout: PangoLayout to be rendered
 * @sw: surface width
 * @sh: surface height
 * @lw: layout width
 * @lh: layout height
 * @posx: final position (x coord)
 * @posy: final position (y coord)
 * @progress: progress of animation
 * @font_color: array of RGBA values
 *
 * This is prototype for subtitle animation function. It's task it to render
 * @layout to @cr according to  @progress, @posx and @posy.
 *
 * When @progress > 1, @layout should be drawn at (@posx, @posy) without any
 * scalling.
 */
typedef void (*TextAnimationFunc)( cairo_t     *cr,
								   PangoLayout *layout,
								   gint         sw,
								   gint         sh,
								   gint         lw,
								   gint         lh,
								   gint         posx,
								   gint         posy,
								   gdouble      progress,
								   gdouble     *font_color,
                                   gdouble     *font_bgcolor );


/* ****************************************************************************
 * Transition related definitions
 * ************************************************************************* */
/* Prototype of transition renderer */
typedef void (*ImgRender)( cairo_t *,
						   cairo_surface_t *,
						   cairo_surface_t *,
						   gdouble );

/* ****************************************************************************
 * Ken Burns effect related definitions
 * ************************************************************************* */
typedef struct _ImgStopPoint ImgStopPoint;
struct _ImgStopPoint
{
	gint    time; /* Duration of this stop point */
	gdouble offx; /* X and Y offsets of zoomed image */
	gdouble offy;
	gdouble zoom; /* Zoom level */
};

/* ****************************************************************************
 * define for gtk clipboard
 * ************************************************************************* */
#define IMG_CLIPBOARD (gdk_atom_intern_static_string ("IMAGINATIO_OWN_CLIPBOARD")) 
#define IMG_INFO_LIST (gdk_atom_intern_static_string ("application/imagination-info-list"))

typedef enum
{
	IMG_CLIPBOARD_CUT,
	IMG_CLIPBOARD_COPY
} ImgClipboardMode;

/* ****************************************************************************
 * Common definitions that are used all over the place
 * ************************************************************************* */
typedef struct _slide_struct slide_struct;
struct _slide_struct
{
	/* Common data - always filled */
	gchar *resolution;        /* Image dimensions */
	gchar *type;              /* Image type */

	/* Fields that are filled when we load slide from disk */
	gchar    *o_filename; /* Filename of the image that slide represents */
	gchar    *r_filename; /* o_filename, rotated according to angle */
	ImgAngle  angle;      /* Angle of rotated image */
    gboolean  load_ok;    /* handle loading problems (file not found, format unknown...) */
    gchar    *original_filename; /* Filename as loaded from the project file */


	/* Fields that are filled if we create slide in memory */
	gint    gradient;         /* Gradient type */
	gdouble g_start_color[3]; /* RGB start color */
	gdouble g_stop_color[3];  /* RGB stop color */
	gdouble g_start_point[2]; /* x, y coordinates of start point */
	gdouble g_stop_point[2];  /* x, y coordinates of stop point */

	/* Still part of the slide params */
	guint duration; /* Duration of still part */ /* NOTE: sub1 */

	/* Transition params */
	gchar     *path;          /* Transition model path to transition */
	gint       transition_id; /* Transition id */
	ImgRender  render;        /* Transition render function */
	guint      speed;         /* Transition speed */ /* NOTE: sub1 */

	/* Ken Burns effect variables */
	GList *points;    /* List with stop points */
	gint   no_points; /* Number of stop points in list */
	gint   cur_point; /* Currently active stop point */

	/* Subtitle variables */
	gchar                *subtitle;        /* Subtitle text */
	TextAnimationFunc     anim;            /* Animation functions */
	gint                  anim_id;         /* Animation id */
	gint                  anim_duration;   /* Duration of animation */
	ImgSubPos             position;        /* Final position of subtitle */
	ImgRelPlacing         placing;         /* Relative placing */
	PangoFontDescription *font_desc;       /* Font description */
	gdouble               font_color[4];   /* Font color (RGBA format) */
    gdouble               font_bgcolor[4]; /* Font background color (RGBA format) */
};

typedef struct _img_window_struct img_window_struct;
struct _img_window_struct
{
	/* Main GUI related variables */
	GtkWidget	*imagination_window;	// Main window
	GtkAccelGroup *accel_group;
	GtkWidget	*open_menu;
	GtkWidget	*save_menu;
	GtkWidget	*save_as_menu;
	GtkWidget	*open_button;
	GtkWidget	*save_button;
	GtkWidget	*cut;
	GtkWidget	*copy;
	GtkWidget	*paste;
	GtkWidget	*remove_menu;
	GtkWidget	*report_menu;
	GtkWidget	*preview_menu;
	GtkWidget 	*preview_button;
	GtkWidget	*menu_preview_mode;
	GtkWidget	*menu_overview_mode;
	GtkWidget	*transition_type;
	GtkWidget	*random_button;
	GtkWidget	*duration;				// Duration spin button
	GtkWidget	*trans_duration;
	GtkWidget	*total_time_data;
	GtkWidget	*filename_data;
	GtkTextBuffer *slide_text_buffer;
	GtkWidget	*scrolled_win;
	GtkWidget	*expand_button;
	GtkWidget   *text_pos_button;
  	GtkWidget	*thumbnail_iconview;
  	GtkWidget	*statusbar;
  	GtkWidget	*progress_bar;
  	GtkWidget	*image_area;
  	GtkWidget	*video_format_combo;
  	GtkWidget	*video_size_combo;
  	guint		context_id;
  	GtkListStore *thumbnail_model;
  	gchar		*current_dir;

	GtkWidget   *paned; /* Main paned (used for saving/restoring geometry) */

	/* Mode switching */
	gint       mode;        /* In what mode are we in: 0 - preview
													   1 - overview */
	gboolean   auto_switch; /* Has been last mode switch made automatically? */
	GtkWidget *prev_root;   /* Preview mode root widget */
	GtkWidget *over_root;   /* Overview mode root widget */
	GtkWidget *thum_root;   /* Thumbnail root widget */
	GtkWidget *over_icon;   /* Overview iconview */
	GtkWidget *active_icon; /* Currently active icon view */
	GObject   *over_cell;   /* Overview cell renderer */

	/* Ken Burns related controls */
	GtkWidget *ken_left;     /* Go to left stop point button */
	GtkWidget *ken_entry;    /* Jump to stop point entry */
	GtkWidget *ken_right;    /* Go to right stop point button */
	GtkWidget *ken_duration; /* Stop point duration spin button */
	GtkWidget *ken_zoom;     /* Zoom slider */
	GtkWidget *ken_add;      /* Add stop point button */
	GtkWidget *ken_update;   /* Update stop point button */
	GtkWidget *ken_remove;   /* Remove stop point button */

	/* Subtitle related controls */
	GtkWidget *sub_textview;      /* Text view */
	GtkWidget *sub_font;          /* Font button */
	GtkWidget *sub_color;         /* Color selector button */
    GtkWidget *sub_bgcolor;       /* Background Color selector button */
	GtkWidget *sub_anim;          /* Animation combo box */
	GtkWidget *sub_anim_duration; /* Animation duration spin button */
	GtkWidget *sub_placing;       /* Placing combo box */
	GtkWidget *sub_pos;           /* Position selector button */

	/* Import slides dialog variables */
	GtkWidget	*dim_label;
	GtkWidget	*size_label;
  	GtkWidget	*preview_image;

	/* Current image position parameters */
	gdouble       x;             /* Last button press coordinates */
	gdouble       y;
	gdouble       bak_offx;      /* Stored offset at button press */
	gdouble       bak_offy;
	gdouble       maxoffx;       /* Maximal offsets for current zoom */
	gdouble       maxoffy;
	ImgStopPoint  current_point; /* Data for rendering current image */
  	slide_struct *current_slide;
	
	/* Update ids */
	gint subtitle_update_id; /* Update subtitle display */
	gint total_dur_id;       /* Update total duration */

	/* Renderers and module stuff */
  	gint		nr_transitions_loaded;
  	GSList		*plugin_list;

	/* Project related variables */
	gchar       *project_filename;		// project name for saving
	gboolean	distort_images;
	gboolean	project_is_modified;
	gchar		video_format;
	gint        video_size[2];
	gdouble     video_ratio;
	gdouble     background_color[3];
  	gint		total_secs;
	gint		total_music_secs;
  	gint		slides_nr;
	slide_struct final_transition;  /* Only speed, render and duration fields
									   of this structure are used (and duration
									   is always 0). */

	/* Variables common to export and preview functions */
	slide_struct    *work_slide;
	cairo_surface_t *current_image;  /* Image in preview area */
	cairo_surface_t *exported_image; /* Image being exported */
	cairo_surface_t *image1;         /* Original images */
	cairo_surface_t *image2;
	cairo_surface_t *image_from;     /* Images used in transition rendering */
	cairo_surface_t *image_to;
	ImgStopPoint    *point1;        /* Last stop point of image1 */
	ImgStopPoint    *point2;        /* First stop point of image2 */
  	GtkTreeIter      cur_ss_iter;
  	guint		     source_id;

	/* Counters that control animation flow */
	guint  total_nr_frames;    /* Total number of frames */
	guint  displayed_frame;    /* Current frame */
	guint  slide_nr_frames;    /* Number of frames fo current slide */
	guint  slide_cur_frame;    /* Current slide frame */
	guint  slide_trans_frames; /* Number of frames in transition */
	guint  slide_still_frames; /* Number of frames in still part */
	guint  next_slide_off;     /* Time offset of next slide */

	gint   still_counter; /* Currently displayed still frame */
	gint   still_max;     /* Number of frames per stop point */
	gint   still_offset;  /* Offset in seconds for next stop point */
	guint  still_cmlt;    /* Cumulative number of still frames */
	GList *cur_point;     /* Current stop point */

	gint   cur_text_frame; /* Current text frame being displayed */
	gint   no_text_frames; /* All text frames */

	/* Preview related variables */
  	gboolean	preview_is_running;
  	GtkWidget	*import_slide_chooser;
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
	GtkWidget   *export_pbar1;
	GtkWidget   *export_pbar2;
	GtkWidget   *export_label;
	GtkWidget   *export_dialog;
	gdouble      export_fps;        /* Frame rate for exported video */
	gchar       *export_cmd_line;   /* ffmpeg spawn cmd line */
	guint        export_slide;		/* Number of slide being exported */
	GSourceFunc  export_idle_func;	/* Stored procedure for pause */
	GPid         ffmpeg_export;     /* ffmpeg's process id */

	/* Sox thread related variables */
	gint      sox_flags;         /* Thread controls:
									 0 - normal operation
									 1 - thread must be killed
									 2 - thread finished normally */
	GThread  *sox;               /* sox thread handler */
	gchar   **exported_audio;    /* Audio files to be exported */
	gint      exported_audio_no; /* Number of audio files inside array */
	gchar    *fifo;              /* Named pipe path */

	/* Audio related stuff */
	GtkWidget	*music_file_treeview;
	GtkWidget	*play_audio_button;
	GtkWidget	*remove_audio_button;
	GtkListStore *music_file_liststore;
	GtkWidget	*music_time_data;
	GPid		play_child_pid;
    
    /* Messages related stuf */
    GtkTextBuffer  *message_buffer;
    GtkWidget      *message_label;
    gint            message_page;

	/* Application related stuff */
	gdouble  image_area_zoom; /* Zoom to be applied to image area */
	gdouble  overview_zoom;   /* Zoom to be applied in overview mode */
	gint     preview_fps;     /* Preview frame rate */
	gboolean low_quality;     /* Preview quality:
								  TRUE  - preview in low-res
								  FALSE - preview in hi-res */

	/* Clipboard related stuff */
	GList				*selected_paths;
	ImgClipboardMode	clipboard_mode;
	
	/* Report dialog related widgets */
	GtkWidget	*report_dialog;
	GtkWidget	*vbox_slide_report_rows;
	GSList		*report_dialog_row_slist;
};

#endif
