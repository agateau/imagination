/*
 *  Copyright (c) 2011 Robert Chéramy   <robert@cheramy.net>
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

#ifndef __VIDEO_FORMAT_H__
#define __VIDEO_FORMAT_H__

#include <gtk/gtk.h>

struct video_size {
    gchar  *name;
    gint    x;
    gint    y;
    gint    default_fps_idx, default_aspect_ratio_idx, default_bitrate_idx;
};

struct video_bitrate {
    gchar  *name;
    gint    value;
};

struct aspect_ratio {
    gchar   *name;
    gchar   *ffmpeg_option;
};

struct video_fps {
    gchar   *name;
    gchar   *ffmpeg_option;
    gdouble value;
};

struct video_format {
    gchar *name;
    gchar *config_name;
    gchar *ffmpeg_option;
    struct video_size *sizelist;
    struct aspect_ratio *aspect_ratio_list;
    struct video_bitrate *bitratelist;
    struct video_fps *fps_list;
    gchar **file_extensions;
};

extern struct video_format video_format_list[];

#endif
