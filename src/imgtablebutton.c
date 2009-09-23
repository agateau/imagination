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

#include "imgtablebutton.h"
#include <math.h>

/* Convenience macros */
#define IMG_PARAM_DEFAULTS \
	G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB
#define IMG_PARAM_READWRITE \
	G_PARAM_WRITABLE | G_PARAM_READABLE | IMG_PARAM_DEFAULTS
#define IMG_PARAM_READ \
	G_PARAM_READABLE | IMG_PARAM_DEFAULTS
#define IMG_PARAM_WRITE \
	G_PARAM_WRITABLE | IMG_PARAM_DEFAULTS

enum
{
	S_ACTIVE_ITEM_CHANGED,
	S_LAST_SIGNAL
};

enum
{
	P_0,
	P_ROWS,          /* RW */
	P_COLS,          /* RW */
	P_NO_ITEMS,      /* R  */
	P_PIXBUFS,       /* R  */
	P_LABELS,        /* R  */
	P_USE_MNEMONICS, /* RW */
	P_USE_STOCK,     /* RW */
	P_ACTIVE_ITEM    /* RW */
};

static guint table_button_signals[S_LAST_SIGNAL] = { 0 };

#define IMG_TABLE_BUTTON_GET_PRIVATE( obj ) \
	( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), \
								   IMG_TYPE_TABLE_BUTTON, \
								   ImgTableButtonPrivate ) )

typedef struct _ImgTableButtonPrivate ImgTableButtonPrivate;
struct _ImgTableButtonPrivate
{
	gint        rows;          /* Number of rows */
	gint        cols;          /* Number of columns */
	gint        no_items;      /* Number of items in array */
	gchar     **labels;        /* Array of all labels */
	GdkPixbuf **pixbufs;       /* Array of all pixbufs */
	gboolean    use_mnemonics; /* Do we use mnemonics */
	gboolean    use_stock;     /* Do we use stock items? */
	gint        active_item;   /* Currently active item */

	GtkWidget *popup; /* Popup window */
	GtkWidget *table; /* Main container for buttons */
};

/* ****************************************************************************
 * Local declarations
 * ************************************************************************* */
static void
img_table_button_set_property( GObject      *object,
							   guint         prop_id,
							   const GValue *value,
							   GParamSpec   *pspec );

static void
img_table_button_get_property( GObject    *object,
							   guint       prop_id,
							   GValue     *value,
							   GParamSpec *pspec );

static void
img_table_button_destroy( GtkObject *object );

static void
img_table_button_clicked( GtkButton *button );

static void
child_clicked( GObject        *child,
			   ImgTableButton *parent );

static void
img_table_button_calc_popup_coords( GtkWidget             *widget,
									ImgTableButtonPrivate *priv,
									gint                  *x,
									gint                  *y );

static void
img_table_button_clear_items( ImgTableButtonPrivate *priv );

static void
img_table_button_real_set_pixbufs( ImgTableButtonPrivate  *priv,
								   gint                    no_images,
								   GdkPixbuf             **pixbufs );

static void
img_table_button_calculate_layout( ImgTableButtonPrivate *priv );


/* ****************************************************************************
 * Initialization
 * ************************************************************************* */
G_DEFINE_TYPE( ImgTableButton, img_table_button, GTK_TYPE_BUTTON );

static void
img_table_button_class_init( ImgTableButtonClass *klass )
{
	GParamSpec     *spec;
	GObjectClass   *gobject_class = G_OBJECT_CLASS( klass );
	GtkObjectClass *object_class = (GtkObjectClass *)klass;
	GtkButtonClass *button_class = (GtkButtonClass *)klass;

	gobject_class->set_property = img_table_button_set_property;
	gobject_class->get_property = img_table_button_get_property;
	object_class->destroy = img_table_button_destroy;
	button_class->clicked = img_table_button_clicked;
	klass->active_item_changed = NULL;

	/* Number of rows */
	spec = g_param_spec_int( "rows", "Rows",
							 "Number of rows in popup widget.",
							 -1, G_MAXINT, -1, IMG_PARAM_READWRITE );
	g_object_class_install_property( gobject_class, P_ROWS, spec );

	/* Number of columns */
	spec = g_param_spec_int( "columns", "Columns",
							 "Number of columns in popup widget.",
							 -1, G_MAXINT, -1, IMG_PARAM_READWRITE );
	g_object_class_install_property( gobject_class, P_COLS, spec );

	/* Nunber of images/string to be added onto buttons */
	spec = g_param_spec_int( "number-of-items", "Number of items",
							 "Number of items in pixbufs or labels array.",
							 0, G_MAXINT, 0, IMG_PARAM_READ );
	g_object_class_install_property( gobject_class, P_NO_ITEMS, spec );

	/* Images to be added onto buttons */
	spec = g_param_spec_pointer( "pixbufs", "Pixbufs",
								 "Array of pixbufs for buttons.",
								 IMG_PARAM_READ );
	g_object_class_install_property( gobject_class, P_PIXBUFS, spec );

	/* Labels to be added onto buttons */
	spec = g_param_spec_pointer( "labels", "Labels", 
								 "Array of labels for buttons.",
								 IMG_PARAM_READ );
	g_object_class_install_property( gobject_class, P_LABELS, spec );

	/* Do labels use mnemonics? */
	spec = g_param_spec_boolean( "use-mnemonics", "Use mnemonics",
								 "Do labels contain mnemonics?",
								 FALSE, IMG_PARAM_READWRITE );
	g_object_class_install_property( gobject_class, P_USE_MNEMONICS, spec );

	/* Do labels represent stock items? */
	g_object_class_override_property( gobject_class, P_USE_STOCK, "use-stock" );

	/* Currently active item */
	spec = g_param_spec_int( "active-item", "Active item",
							 "Index of currently active item.",
							 -1, G_MAXINT, -1, IMG_PARAM_READWRITE );
	g_object_class_install_property( gobject_class, P_ACTIVE_ITEM, spec );


	/* Selection changed signal */
	table_button_signals[S_ACTIVE_ITEM_CHANGED] =
		g_signal_new( "active-item-changed",
					  G_TYPE_FROM_CLASS( klass ),
					  G_SIGNAL_RUN_FIRST,
					  G_STRUCT_OFFSET( ImgTableButtonClass,
									   active_item_changed ),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__INT,
					  G_TYPE_NONE, 1, G_TYPE_INT );

	g_type_class_add_private( gobject_class, sizeof( ImgTableButtonPrivate ) );
}

static void
img_table_button_init( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	/* Set default values for properties */
	priv->rows = -1;
	priv->cols = -1;
	priv->no_items = 0;
	priv->labels = NULL;
	priv->pixbufs = NULL;
	priv->use_mnemonics = FALSE;
	priv->use_stock = FALSE;
	priv->active_item = -1;

	/* Create additional widgets */
	priv->popup = gtk_window_new( GTK_WINDOW_POPUP );
	g_signal_connect( G_OBJECT( priv->popup ), "leave-notify-event",
					  G_CALLBACK( gtk_widget_hide ), NULL );

	priv->table = NULL;
}


/* ****************************************************************************
 * Local functions
 * ************************************************************************* */
static void
img_table_button_set_property( GObject      *object,
							   guint         prop_id,
							   const GValue *value,
							   GParamSpec   *pspec )
{
	ImgTableButton *button = IMG_TABLE_BUTTON( object );

	switch( prop_id )
	{
		case P_ROWS:
			img_table_button_set_rows( button, g_value_get_int( value ) );
			break;

		case P_COLS:
			img_table_button_set_columns( button, g_value_get_int( value ) );
			break;

		case P_USE_MNEMONICS:
			img_table_button_set_use_mnemonics( button,
												g_value_get_boolean( value ) );
			break;

		case P_USE_STOCK:
			img_table_button_set_use_stock( button,
											g_value_get_boolean( value ) );
			break;

		case P_ACTIVE_ITEM:
			img_table_button_set_active_item( button,
											  g_value_get_int( value ) );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
img_table_button_get_property( GObject    *object,
							   guint       prop_id,
							   GValue     *value,
							   GParamSpec *pspec )
{
	ImgTableButton *button = IMG_TABLE_BUTTON( object );

	switch( prop_id )
	{
		case P_ROWS:
			g_value_set_int( value, img_table_button_get_rows( button ) );
			break;

		case P_COLS:
			g_value_set_int( value, img_table_button_get_columns( button ) );
			break;

		case P_NO_ITEMS:
			g_value_set_int( value, img_table_button_get_no_items( button ) );
			break;

		case P_PIXBUFS:
			{
				GdkPixbuf **pixs;

				pixs = img_table_button_get_pixbufs( button );
				g_value_set_pointer( value, pixs );
			}
			break;

		case P_LABELS:
			{
				gchar **labs;

				labs = img_table_button_get_labels( button );
				g_value_set_pointer( value, labs );
			}
			break;

		case P_USE_MNEMONICS:
			g_value_set_boolean( value,
								 img_table_button_get_use_mnemonics( button ) );
			break;

		case P_USE_STOCK:
			g_value_set_boolean( value,
								 img_table_button_get_use_stock( button ) );
			break;

		case P_ACTIVE_ITEM:
			g_value_set_int( value,
							 img_table_button_get_active_item( button ) );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
img_table_button_destroy( GtkObject *object )
{
	/* Free structures here */
	ImgTableButtonPrivate *priv = IMG_TABLE_BUTTON_GET_PRIVATE( object );
	if( priv->popup )
	{
		img_table_button_clear_items( priv );

		gtk_widget_destroy( priv->popup );
		priv->popup = NULL;
	}

	GTK_OBJECT_CLASS( img_table_button_parent_class )->destroy( object );
}

static void
img_table_button_clicked( GtkButton *button )
{
	ImgTableButtonPrivate *priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );
	gint                   x,
						   y;

	/* Calculate proper position */
	img_table_button_calc_popup_coords( GTK_WIDGET( button ), priv, &x, &y );
	gtk_window_move( GTK_WINDOW( priv->popup ), x, y ); 
	gtk_widget_show( priv->popup );
}

static void
child_clicked( GObject        *child,
			   ImgTableButton *parent )
{
	gint count;

	count = GPOINTER_TO_INT( g_object_get_data( child, "count" ) );
	img_table_button_set_active_item( parent, count );
}

static void
img_table_button_calc_popup_coords( GtkWidget             *widget,
									ImgTableButtonPrivate *priv,
									gint                  *x,
									gint                  *y )
{
	gint            a, b,
				    monitor_n;
	GdkScreen      *screen;
	GdkRectangle    monitor;
	GtkRequisition  req;

	/* Calculate proper position */
	gdk_window_get_origin( widget->window, &a, &b );
	gtk_widget_size_request( priv->popup, &req );
	a += widget->allocation.x - req.width;
	b += widget->allocation.y - req.height;

	screen = gtk_widget_get_screen( widget );
	monitor_n = gdk_screen_get_monitor_at_window( screen, widget->window );
	gdk_screen_get_monitor_geometry( screen, monitor_n, &monitor);
	
	if( a < monitor.x )
		a = monitor.x;

	if( b < monitor.y )
		b = monitor.y;

	*x = a;
	*y = b;
}

static void
img_table_button_clear_items( ImgTableButtonPrivate *priv )
{
	if( priv->pixbufs )
	{
		gint i;

		for( i = 0; i < priv->no_items; i++ )
			g_object_unref( G_OBJECT( priv->pixbufs[i] ) );
		g_slice_free1( sizeof( GdkPixbuf * ) * priv->no_items, priv->pixbufs );
		priv->pixbufs = NULL;
	}
	else if( priv->labels )
	{
		gint i;

		for( i = 0; i < priv->no_items; i++ )
			g_free( priv->labels[i] );
		g_slice_free1( sizeof( gchar * ) * priv->no_items, priv->labels );
		priv->labels = NULL;
	}

	priv->no_items = 0;
}

static void
img_table_button_real_set_pixbufs( ImgTableButtonPrivate  *priv,
								   gint                    no_images,
								   GdkPixbuf             **pixbufs )
{
	gint i;

	priv->no_items = no_images;
	priv->pixbufs = g_slice_alloc( sizeof( GdkPixbuf * ) * no_images );
	for( i = 0; i < no_images; i++ )
	{
		priv->pixbufs[i] = pixbufs[i];
		g_object_ref( G_OBJECT( pixbufs[i] ) );
	}
}

static void
img_table_button_calculate_layout( ImgTableButtonPrivate *priv )
{
	gint root;

	if( priv->rows < 0 && priv->cols > 0 )
	{
		/* Calculate number of rows from cols */
		priv->rows = priv->no_items / priv->cols;
		priv->rows += ( priv->no_items % priv->cols ? 1 : 0 );

		return;
	}

	if( priv->rows > 0 && priv->cols < 0 )
	{
		/* Calculate numer fo cols from rows */
		priv->cols = priv->no_items / priv->rows;
		priv->cols += ( priv->no_items % priv->rows ? 1 : 0 );

		return;
	}

	/* Calculate both */
	root = (gint)sqrtf( priv->no_items );
	if( ( root * root ) >= priv->no_items )
	{
		priv->rows = root;
		priv->cols = root;
	}
	else if( ( root * ( root + 1 ) ) >= priv->no_items )
	{
		priv->rows = root + 1;
		priv->cols = root;
	}
	else
	{
		priv->rows = root + 1;
		priv->cols = root + 1;
	}
}

/* ****************************************************************************
 * PUBLIC API
 * ************************************************************************* */

GtkWidget *
img_table_button_new( void )
{
	return( g_object_new( IMG_TYPE_TABLE_BUTTON, NULL ) );
}

void
img_table_button_set_pixbufs( ImgTableButton  *button,
							  gint             no_images,
							  GdkPixbuf      **pixbufs )
{
	ImgTableButtonPrivate *priv;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	img_table_button_clear_items( priv );
	img_table_button_real_set_pixbufs( priv, no_images, pixbufs );
	if( priv->rows < 0 || priv->cols < 0 )
		img_table_button_calculate_layout( priv );
	img_table_button_update_layout( button );
}

void
img_table_button_set_rows( ImgTableButton *button,
						   gint            rows )
{
	ImgTableButtonPrivate *priv;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );
	g_return_if_fail( rows > -2 && rows < G_MAXINT );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );
	if( priv->rows == rows )
		return;

	priv->rows = rows;
	if( priv->rows < 0 || priv->cols < 0 )
		img_table_button_calculate_layout( priv );
	img_table_button_update_layout( button );
}

void
img_table_button_set_columns( ImgTableButton *button,
							  gint            columns )
{
	ImgTableButtonPrivate *priv;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );
	g_return_if_fail( columns > -2 && columns < G_MAXINT );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );
	if( priv->cols == columns )
		return;

	priv->cols = columns;
	if( priv->rows < 0 || priv->cols < 0 )
		img_table_button_calculate_layout( priv );
	img_table_button_update_layout( button );
}


void
img_table_button_set_use_mnemonics( ImgTableButton *button,
									gboolean        use_mnemonics )
{
	ImgTableButtonPrivate *priv;
	gboolean               cmp;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );
	cmp = ( use_mnemonics ? TRUE : FALSE );

	if( priv->use_mnemonics == cmp )
		return;

	priv->use_mnemonics = cmp;
	img_table_button_update_layout( button );
}

void
img_table_button_set_use_stock( ImgTableButton *button,
								gboolean        use_stock )
{
	ImgTableButtonPrivate *priv;
	gboolean               cmp;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );
	cmp = ( use_stock ? TRUE : FALSE );

	if( priv->use_stock == cmp )
		return;

	priv->use_stock = cmp;
	img_table_button_update_layout( button );
}

void
img_table_button_set_active_item( ImgTableButton *button,
								  gint            active )
{
	ImgTableButtonPrivate *priv;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );
	g_return_if_fail( active > -2 && active < G_MAXINT );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	if( priv->active_item == active )
	{
		gtk_widget_hide( priv->popup );
		return;
	}

	priv->active_item = MIN( active, priv->no_items - 1 );

	/* If no items are present or nothing should be selected, destroy child
	 * widget and display empty button */
	if( ! priv->no_items || priv->active_item < 0 )
	{
		GtkWidget *child = gtk_bin_get_child( GTK_BIN( button ) );

		if( child )
			gtk_widget_destroy( child );

		g_signal_emit( button, table_button_signals[S_ACTIVE_ITEM_CHANGED],
					   0, priv->active_item );

		return;
	}

	gtk_widget_hide( priv->popup );
	if( priv->labels )
		gtk_button_set_label( GTK_BUTTON( button ),
							  priv->labels[priv->active_item] );
	else
	{
		GtkWidget *image;

		image = gtk_bin_get_child( GTK_BIN( button ) );
		if( ! image )
		{
			image = gtk_image_new();
			gtk_container_add( GTK_CONTAINER( button ), image );
			gtk_widget_show( image );
		}

		gtk_image_set_from_pixbuf( GTK_IMAGE( image ),
								   priv->pixbufs[priv->active_item] );
	}

	g_signal_emit( button, table_button_signals[S_ACTIVE_ITEM_CHANGED],
				   0, priv->active_item );
}

gint
img_table_button_get_rows( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), -1 );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->rows );
}

gint
img_table_button_get_columns( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), -1 );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->cols );
}

gint
img_table_button_get_no_items( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), -1 );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->no_items );
}

GdkPixbuf **
img_table_button_get_pixbufs( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), NULL );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->pixbufs );
}

gchar **
img_table_button_get_labels( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), NULL );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->labels );
}

gboolean
img_table_button_get_use_mnemonics( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), FALSE );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->use_mnemonics );
}

gboolean
img_table_button_get_use_stock( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;


	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), FALSE );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->use_stock );
}

gint
img_table_button_get_active_item( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;

	g_return_val_if_fail( IMG_IS_TABLE_BUTTON( button ), -1 );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	return( priv->active_item );
}

void
img_table_button_update_layout( ImgTableButton *button )
{
	ImgTableButtonPrivate *priv;
	gint                   x,
						   y,
						   count = 0;
	GtkWidget             *child;

	g_return_if_fail( IMG_IS_TABLE_BUTTON( button ) );

	priv = IMG_TABLE_BUTTON_GET_PRIVATE( button );

	if( priv->table )
	{
		gtk_widget_destroy( priv->table );
		priv->table = NULL;
	}

	if( ! priv->no_items )
		return;

	priv->table = gtk_table_new( priv->rows, priv->cols, TRUE );
	gtk_container_add( GTK_CONTAINER( priv->popup ), priv->table );

	/* This outer if is for diferent packing directions. Currently only
	 * horizontal is implemented. */
	if( TRUE )
	{
		for( x = 0; x < priv->rows; x++ )
		{
			for( y = 0;
				 y < priv->cols && count < priv->no_items;
				 y++, count++ )
			{
				if( priv->labels )
				{
					gchar *label = priv->labels[count];

					child = gtk_button_new_with_label( label );
					gtk_button_set_use_stock( GTK_BUTTON( child ),
											  priv->use_stock );
					gtk_button_set_use_underline( GTK_BUTTON( child ),
												  priv->use_mnemonics );
				}
				else
				{
					GdkPixbuf *pix = priv->pixbufs[count];
					GtkWidget *image;

					child = gtk_button_new();

					image = gtk_image_new_from_pixbuf( pix );
					gtk_container_add( GTK_CONTAINER( child ), image );
				}

				g_object_set_data( G_OBJECT( child ), "count",
								   GINT_TO_POINTER( count ) );
				g_signal_connect( G_OBJECT( child ), "clicked",
								  G_CALLBACK( child_clicked ), button );
				gtk_table_attach( GTK_TABLE( priv->table ), child, y, y + 1,
								  x, x + 1, GTK_EXPAND | GTK_FILL,
								  GTK_EXPAND | GTK_FILL, 0, 0 );
			}
		}
	}

	/* Prepare for showing */
	gtk_widget_show_all( priv->table );
}

