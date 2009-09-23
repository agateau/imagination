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

#ifndef __IMG_TABLE_BUTTON_H__
#define __IMG_TABLE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IMG_TYPE_TABLE_BUTTON \
	( img_table_button_get_type() )

#define IMG_TABLE_BUTTON( obj ) \
	( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), \
								  IMG_TYPE_TABLE_BUTTON, \
								  ImgTableButton ) )

#define IMG_TABLE_BUTTON_CLASS( klass ) \
	( G_TYPE_CHECK_CLASS_CAST( ( klass ), \
							   IMG_TYPE_TABLE_BUTTON_CLASS, \
							   ImgTableButtonClass ) )

#define IMG_IS_TABLE_BUTTON( obj ) \
	( G_TYPE_CHECK_INSTANCE_TYPE( ( obj ), \
								  IMG_TYPE_TABLE_BUTTON ) )

#define IMG_IS_TABLE_BUTTON_CLASS( klass ) \
	( G_TYPE_CHECK_CLASS_TYPE( ( klass ), \
							   IMG_TYPE_TABLE_BUTTON_CLASS ) )

typedef struct _ImgTableButton ImgTableButton;
typedef struct _ImgTableButtonClass ImgTableButtonClass;

struct _ImgTableButton
{
	GtkButton parent;
};

struct _ImgTableButtonClass
{
	GtkButtonClass parent_class;

	void (* active_item_changed)( ImgTableButton *button,
								  gint            item );
};


GType
img_table_button_get_type( void ) G_GNUC_CONST;

GtkWidget *
img_table_button_new( void );

void
img_table_button_set_pixbufs( ImgTableButton  *button,
							  gint             no_images,
							  GdkPixbuf      **pixbufs );

void
img_table_button_set_rows( ImgTableButton *button,
						   gint            rows );

void
img_table_button_set_columns( ImgTableButton *button,
							  gint            columns );

void
img_table_button_set_use_mnemonics( ImgTableButton *button,
									gboolean        use_mnemonics );

void
img_table_button_set_use_stock( ImgTableButton *button,
								gboolean        use_stock );

void
img_table_button_set_active_item( ImgTableButton *button,
								  gint            active );

gint
img_table_button_get_rows( ImgTableButton *button );

gint
img_table_button_get_columns( ImgTableButton *button );

gint
img_table_button_get_no_items( ImgTableButton *button );

GdkPixbuf **
img_table_button_get_pixbufs( ImgTableButton *button );

gchar **
img_table_button_get_labels( ImgTableButton *button );

gboolean
img_table_button_get_use_mnemonics( ImgTableButton *button );

gboolean
img_table_button_get_use_stock( ImgTableButton *button );

gint
img_table_button_get_active_item( ImgTableButton *button );

void
img_table_button_update_layout( ImgTableButton *button );

G_END_DECLS

#endif /* __IMG_TABLE_BUTTON_H__ */

