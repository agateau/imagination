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

#include "support.h"

#define PLUGINS_INSTALLED 0

static gboolean img_plugin_is_loaded(img_window_struct *, GModule *);

GtkWidget *img_load_icon(gchar *filename, GtkIconSize size)
{
    GtkWidget *file_image;
	gchar *path;
	GdkPixbuf *file_pixbuf = NULL;

	path = g_strconcat(DATADIR, "/imagination/pixmaps/",filename,NULL);
	file_pixbuf = gdk_pixbuf_new_from_file(path,NULL);
	g_free (path);

	if (file_pixbuf == NULL)
		file_image = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, size);
    else
	{
		file_image = gtk_image_new_from_pixbuf(file_pixbuf);
	    g_object_unref (file_pixbuf);
	}
    return file_image;
}

gchar *img_convert_seconds_to_time(gint total_secs)
{
	gint h, m, s;

	h =  total_secs / 3600;
	m = (total_secs % 3600) / 60;
	s =  total_secs - (h * 3600) - (m * 60);
	return g_strdup_printf("%02d:%02d:%02d", h, m, s);
}

GtkWidget *_gtk_combo_box_new_text(gboolean pointer)
{
	GtkWidget *combo_box;
	GtkCellRenderer *cell;
	GtkListStore *list;
	GtkTreeStore *tree;
	GtkTreeModel *model;

	if (pointer)
	{
		tree = gtk_tree_store_new (4, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
		model = GTK_TREE_MODEL( tree );

		combo_box = gtk_combo_box_new_with_model (model);
		g_object_unref (G_OBJECT( model ));
		cell = gtk_cell_renderer_pixbuf_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, FALSE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell, "pixbuf", 0, NULL);
	}
	else
	{
		list = gtk_list_store_new (1, G_TYPE_STRING);
		model = GTK_TREE_MODEL( list );
		
		combo_box = gtk_combo_box_new_with_model (model);
		g_object_unref (G_OBJECT( model ));
	}

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell, "text", pointer ? 1 : 0, NULL);
	g_object_set(cell, "ypad", (guint)0, NULL);
	return combo_box;
}

void img_set_statusbar_message(img_window_struct *img_struct, gint selected)
{
	gchar *message = NULL;
	gchar *total_slides = NULL;

	if (img_struct->slides_nr == 0)
	{
		message = g_strdup_printf(_("Welcome to Imagination - %d transitions loaded."),img_struct->nr_transitions_loaded);
		gtk_statusbar_push(GTK_STATUSBAR(img_struct->statusbar),img_struct->context_id,message);
		g_free(message);
		gtk_label_set_text(GTK_LABEL(img_struct->total_slide_number_label),NULL);
	}
	else if (selected)
	{
		message = g_strdup_printf(_("%d slides selected"),selected);
		gtk_statusbar_push(GTK_STATUSBAR(img_struct->statusbar),img_struct->context_id,message);
		g_free(message);
	}
	else
	{
		total_slides = g_strdup_printf("%d",img_struct->slides_nr);
		gtk_label_set_text(GTK_LABEL(img_struct->total_slide_number_label),total_slides);
		message = g_strdup_printf(ngettext("%d slide imported %s" ,"%d slides imported %s",img_struct->slides_nr),img_struct->slides_nr,_(" - Use the CTRL key to select/unselect or SHIFT for multiple select"));
		gtk_statusbar_push(GTK_STATUSBAR(img_struct->statusbar),img_struct->context_id,message);
		g_free(total_slides);
		g_free(message);
	}
}

void img_load_available_transitions(img_window_struct *img)
{
	GDir          *dir;
	const gchar   *transition_name;
	gchar         *fname = NULL, *name, *filename;
	gchar        **trans, **bak;
	GModule       *module;
	GdkPixbuf     *pixbuf;
	GtkTreeIter    piter, citer;
	GtkTreeStore  *model;
	gpointer       address;
	gchar         *search_paths[3], **path;
	void (*plugin_set_name)(gchar **, gchar ***);

	model = GTK_TREE_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(img->transition_type)));
	
	/* Fill the combo box with no transition */
	gtk_tree_store_append(model, &piter, NULL);
	gtk_tree_store_set(model, &piter, 0, NULL, 1, _("None"), 2, NULL, 3, -1, -1);
	gtk_combo_box_set_active(GTK_COMBO_BOX(img->transition_type), 0);

	/* Create NULL terminated array of paths that we'll be looking at */
#if PLUGINS_INSTALLED
	search_paths[0] = g_strconcat(PACKAGE_LIB_DIR,"/imagination",NULL);
#else
	search_paths[0] = g_strdup("./transitions");
#endif
	search_paths[1] = g_strconcat( g_get_home_dir(), "/.imagination/plugins", NULL );
	search_paths[2] = NULL;

	/* Search all paths listed in array */
	for( path = search_paths; *path; path++ )
	{
		/* I removed error from here since it hasn't been used and leaks away */
		dir = g_dir_open(*path, 0, NULL);
		if( dir == NULL )
		{
			g_free( *path );
			continue;
		}
		
		while( TRUE )
		{
			transition_name = g_dir_read_name( dir );
			if (transition_name == NULL)
				break;
			
			fname = g_build_filename(*path, transition_name, NULL);
			module = g_module_open(fname, G_MODULE_BIND_LOCAL);
			if (module && img_plugin_is_loaded(img, module) == FALSE)
			{
				/* Obtain the name from the plugin function */
				g_module_symbol( module, "img_get_plugin_info",
								 (void *)&plugin_set_name);
				plugin_set_name(&name, &trans);
				
				/* Add group name to the store */
				gtk_tree_store_append( model, &piter, NULL );
				gtk_tree_store_set( model, &piter, 0, NULL, 1, name, 3, 0, -1 );
				img->plugin_list = g_slist_append(img->plugin_list, module);
				
				/* Add transitions */
				for( bak = trans; *trans; trans += 3 )
				{
#if PLUGINS_INSTALLED
					filename =
						g_strdup_printf( "%s/imagination/pixmaps/imagination-%d.png",
										 DATADIR, GPOINTER_TO_INT( trans[2] ) );
#else /* PLUGINS_INSTALLED */
					filename =
						g_strdup_printf( "./pixmaps/imagination-%d.png",
										 GPOINTER_TO_INT( trans[2] ) );
#endif /* ! PLUGINS_INSTALLED */

					pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

					/* Local plugins will fail to load images from system
					 * folder, so we'll try to load the from home folder. */
					if( ! pixbuf )
					{
						g_free( filename );
						filename =
							g_strdup_printf( "%s/.imagination/pixmaps/imagination-%d.png",
											 g_get_home_dir(), GPOINTER_TO_INT( trans[2] ) );
						pixbuf = gdk_pixbuf_new_from_file( filename, NULL );
					}
					g_free(filename);
					g_module_symbol( module, trans[1], &address );
					gtk_tree_store_append( model, &citer, &piter );
					gtk_tree_store_set( model, &citer, 0, pixbuf, 1, trans[0], 2, address, 3, GPOINTER_TO_INT( trans[2] ), -1 );
				img->nr_transitions_loaded++;
				}
				g_free( bak );
			}
			g_free(fname);
		}
		g_free(*path);
		g_dir_close(dir);
	}
}

static gboolean img_plugin_is_loaded(img_window_struct *img, GModule *module)
{
	return (g_slist_find(img->plugin_list,module) != NULL);
}

void img_show_file_chooser(SexyIconEntry *entry, SexyIconEntryPosition icon_pos,int button,img_window_struct *img)
{
	GtkWidget *file_selector;
	gchar *dest_dir;
	gint response;

	file_selector = gtk_file_chooser_dialog_new (_("Please choose the slideshow project filename"),
							GTK_WINDOW (img->imagination_window),
							GTK_FILE_CHOOSER_ACTION_SAVE,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_CANCEL,
							GTK_STOCK_SAVE,
							GTK_RESPONSE_ACCEPT,
							NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (file_selector),TRUE);
	response = gtk_dialog_run (GTK_DIALOG(file_selector));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		dest_dir = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (file_selector));
		gtk_entry_set_text(GTK_ENTRY(entry),dest_dir);
		g_free(dest_dir);
	}
	gtk_widget_destroy(file_selector);
}

GdkPixbuf *img_load_pixbuf_from_file(gchar *filename)
{
	GdkPixbuf *thumb = NULL;

	thumb = gdk_pixbuf_new_from_file_at_scale(filename, 93, 70, TRUE, NULL);
	return thumb;
}

slide_struct *img_set_slide_info(gint duration, guint speed, void (*render), gint transition_id, gchar *path, gchar *filename)
{
	slide_struct *slide_info = NULL;
	GdkPixbufFormat *pixbuf_format;
	gint width,height;

	slide_info = g_new0(slide_struct,1);
	if (slide_info)
	{
		slide_info->duration = duration;
		slide_info->speed = speed;
		slide_info->render = render;
		slide_info->transition_id = transition_id;
		slide_info->path = g_strdup( path );
		slide_info->filename = g_strdup(filename);
		pixbuf_format = gdk_pixbuf_get_file_info(filename,&width,&height);
		slide_info->resolution = g_strdup_printf("%d x %d",width,height);
		slide_info->type = gdk_pixbuf_format_get_name(pixbuf_format);
	}
	return slide_info;
}

