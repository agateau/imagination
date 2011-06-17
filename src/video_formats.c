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

/**** How-To ad a new video format ***
Adding a new video format is quite easy: you have to add an entry to video_format_lists.
This entry includes:
gchar *name;                The name of the video format. As it will be
                            displayed in the UI, wrap it with gettext_noop
gchar *config_name;         The name to use to store it in the project file.
gchar *ffmpeg_option;       The options (NOT Size/Aspect Ratio/Bitrate/FPS)
                            you have to give ffmpeg to produce a video in this format.
struct video_size *sizelist;                A list of Available video sizes for this
                                            format (mandatory).
struct aspect_ratio *aspect_ratio_list;     A list of Available aspect ratios for
                                            this format or NULL if not needed.
struct video_bitrate *bitratelist;          A list of Bitrates available for this
                                            format or NULL if not needed.
struct video_fps *fps_list;                 A list of Frames per second available
                                            for this format (mandatory)
gchar **file_extensions;                    A list of file extensions used by this format,
                                            the first one is the default one.

Lists must be defined separatly and end with the attribute name = NULL.

Have fun !
*/

#include "video_formats.h"
#include "support.h"
/* As you cant use gettext in the video format declaration,
    we use gettext_noop to mark them,
    and explicitly call gettext when using the strings. */
#define gettext_noop(string) string

/* Define video formats */
struct aspect_ratio aspect_ratio_list[] = {
    {"4:3", "4:3"},
    {"16:9", "16:9"},
    {NULL}
};

struct video_size VOB_size_list[] = {
    /* name, x, y, fps, ratio. bitrate, */
    {"720 x 576 PAL",   720,  576,   0, -1, -1},
    {"720 x 480 NTSC",  720,  480,   1, -1, -1},
    {"1280 x 720 HD",  1280,  720,  -1, -1, -1},
    {"1920 x 1080 HD", 1920, 1080,  -1, -1, -1},
    {NULL}
};

struct video_fps VOB_fps_list[] = {
    {"25 (PAL)", "25 -target pal-dvd", 25},
    {"30 (NTSC)", "30000/1001 -target ntsc-dvd", 30000/1001},
    {NULL}
};

gchar *VOB_extensions[] = {
    ".vob",
    ".mpg",
    NULL
};

struct video_size OGV_size_list[] = {
    {"320 x 240 4:3",   320, 240, -1,  0, 1},
    {"400 x 300",       400, 300, -1, -1, 1},
    {"512 x 384",       512, 384, -1, -1, 1},
    {"640 x 480",       640, 480, -1, -1, 1},
    {"800 x 600",       800, 600, -1, -1, 1},
    {"320 x 180 16:9",  320, 180, -1,  1, 1},
    {"400 x 225",       400, 225, -1, -1, 1},
    {"512 x 288",       512, 288, -1, -1, 1},
    {"640 x 360",       640, 360, -1, -1, 1},
    {"854 x 480",       854, 480, -1, -1, 1},
    {NULL}
};

/* These values have been contributed by Jean-Pierre Redonnet. */
struct video_bitrate OGV_bitrate_list[] = {
    {gettext_noop("512 kbps (low)"), 512*1024},
    {gettext_noop("1024 kbps (medium)"), 1024*1024},
    {gettext_noop("2048 kbps (high)"), 2048*1024},
    {NULL}
};

struct video_fps OGV_fps_list[] = {
    {"30", "30", 30},
    {NULL}
};

gchar *OGV_extensions[] = {
    ".ogv",
    NULL
};

struct video_size FLV_size_list[] = {
    {"320 x 240 4:3",   320, 240, -1,  0, 1},
    {"400 x 300",       400, 300, -1, -1, 1},
    {"512 x 384",       512, 384, -1, -1, 1},
    {"640 x 480",       640, 480, -1, -1, 1},
    {"800 x 600",       800, 600, -1, -1, 1},
    {"320 x 180 16:9",  320, 180, -1,  1, 1},
    {"400 x 225",       400, 225, -1, -1, 1},
    {"512 x 288",       512, 288, -1, -1, 1},
    {"640 x 360",       640, 360, -1, -1, 1},
    {"854 x 480",       854, 480, -1, -1, 1},
    {NULL}
};

struct video_bitrate FLV_bitrate_list[] = {
    {gettext_noop("384 kbps (low)"), 384*1024},
    {gettext_noop("768 kbps (medium)"), 768*1024},
    {gettext_noop("1536 kbps (high)"), 1536*1024},
    {NULL}
};

gchar *FLV_extensions[] = {
    ".flv",
    NULL
};

struct video_size x3GP_size_list[] = {
    {"128 x 96",     128,   96, -1, -1, -1},
    {"176 x 144",    176,  144, -1, -1, -1},
    {"352 x 288",    352,  288, -1, -1, -1},
    {"704 x 576",    704,  576, -1, -1, -1},
    {"1408 x 1152", 1408, 1153, -1, -1, -1},
    {NULL}
};

struct video_fps x3GP_fps_list[] = {
    {"25", "25", 25},
    {NULL}
};

gchar *x3GP_extensions[] = {
    ".3gp",
    NULL
};

/* x264 */
struct video_size x264_size_list[] = {
    {"HD 852x480",      852,    480, -1, -1, -1},
    {"HD 1280x720",    1280,    720, -1, -1, -1},
    {"HD 1920x1080",   1920,   1080, -1, -1, -1},
    {NULL}
};

gchar *x264_extensions[] = {
    ".mp4",
    NULL
};

struct video_format video_format_list[] = {
    /* name, config_name, video_format, ffmpeg_option, sizelist,
            aspect_ratio_list, bitratelist, fps_list, file_extensions */
    {gettext_noop("VOB (DVD Video)"), "VOB",
        "-loglevel debug "
        "-bf 2 " /* use 2 B-Frames */
        /* target is set with aspect ratio: pal-dvd or ntsc-dvd */
        , VOB_size_list, aspect_ratio_list, NULL, VOB_fps_list,
        VOB_extensions
    },

    {gettext_noop("OGV (Theora Vorbis)"), "OGV",
        "-f ogg "
        "-vcodec libtheora -acodec libvorbis"
        , OGV_size_list, aspect_ratio_list, OGV_bitrate_list, OGV_fps_list,
        OGV_extensions
    },

    {gettext_noop("FLV (Flash Video)"), "FLV",
        "-f flv -vcodec flv -acodec libmp3lame"
        "-ab 56000"     /* audio bitrate */
        "-ar 22050"     /* audio sampling frequency*/
        "-ac 1 "        /* number of audio channels */
        , FLV_size_list, aspect_ratio_list, FLV_bitrate_list, OGV_fps_list,
        FLV_extensions
    },

    {gettext_noop("3GP (Mobile Phones)"), "3GP",
        "-f 3gp -vcodec h263 -acodec libfaac"
        "-b 192k "                  /* bitrate */
        "-ab 32k -ar 8000 -ac 1"    /* audio bitrate, sampling frequency and number of channels */
        , x3GP_size_list, NULL, NULL, x3GP_fps_list,
        x3GP_extensions
    },
    /* ffmpeg options for x264 thanks to David Gnedt */
    {gettext_noop("H.264/MPEG-4 AVC"), "x264",
        "-vcodec libx264 -crf 15 " /* FIXME -crf should be in a "Quality" option that also includes bitrate */
        "-acodec libmp3lame -ac 2 -ar 44100 -ab 128k",
        x264_size_list,
        aspect_ratio_list,
        NULL,
        VOB_fps_list,
        x264_extensions
    },
    {NULL}
};
