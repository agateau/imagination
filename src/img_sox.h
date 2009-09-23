/*
** Copyright (C) 2009 Tadej Borovšak <tadeboro@gmail.com>
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

#ifndef __IMG_SOX_H__
#define __IMG_SOX_H__

#include <sox.h>
#include <glib.h>

/* Structure that holds all thread related data */
typedef struct _ImgThreadData ImgThreadData;
struct _ImgThreadData
{
	/* Next field is accesses simultaneously, so it should only be
	 * modified using atomic operations!!! */
	gint *sox_flags; /* 0 - thread operates normally
						1 - force thread to finish */

	/* Next four fields are set in preparation stage, user as read-only
	 * fileds inside sox thread and freed when export has finished/been
	 * terminated */
	gchar **files;     /* Audio files that should be exported */
	gint    no_files;  /* Number of files in files array */
	gchar  *fifo;      /* Write output here */
	gint    length;    /* Slideshow length */

	/* All of the fields from here on are only accessed from thread */
	sox_format_t *input;
	sox_format_t *output;
	gint          current_input;
};

/* Public API */
gpointer
img_produce_audio_data( ImgThreadData *data );

#endif /* __IMG_SOX_H__ */
