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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main-window.h"
#include "callbacks.h"
#include "export.h"

static const GtkTargetEntry drop_targets[] =
{
  { "text/uri-list",0,0 },
};

static void img_combo_box_transition_type_changed (GtkComboBox *, img_window_struct *);
static void img_random_button_clicked(GtkButton *, img_window_struct *);
static gpointer img_set_random_transition(img_window_struct *, slide_struct *);
static void img_combo_box_speed_changed (GtkComboBox *,  img_window_struct *);
static void img_spinbutton_value_changed (GtkSpinButton *, img_window_struct *);
static void img_clear_audio_files(GtkButton *, img_window_struct *);
static void img_expand_button_clicked(GtkButton *, img_window_struct *);
static void img_on_drag_audio_data_received (GtkWidget *,GdkDragContext *, int, int, GtkSelectionData *, unsigned int, unsigned int, img_window_struct *);
static void img_activate_remove_button_music_liststore(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, img_window_struct *);
static void img_quit_menu(GtkMenuItem *, img_window_struct *);
static void img_select_all_thumbnails(GtkMenuItem *, img_window_struct *);
static void img_unselect_all_thumbnails(GtkMenuItem *, img_window_struct *);
static void img_goto_line_entry_activate(GtkWidget *, img_window_struct *);
static gint img_sort_none_before_other(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);
static void img_check_numeric_entry (GtkEditable *entry, gchar *text, gint lenght, gint *position, gpointer data);
static void img_show_uri(GtkMenuItem *, img_window_struct *);

static void
img_create_export_menu( GtkWidget         *item,
						img_window_struct *img );

static gboolean
img_iconview_selection_button_press( GtkWidget         *widget,
									 GdkEventButton    *button,
									 img_window_struct *img );

static gboolean
img_scroll_thumb( GtkWidget         *widget,
				  GdkEventScroll    *scroll,
				  img_window_struct *img );


img_window_struct *img_create_window (void)
{
	img_window_struct *img_struct = NULL;
	GtkWidget *vbox1;
	GtkWidget *menubar;
	GtkWidget *menuitem1;
	GtkWidget *menu1;
	GtkWidget *imagemenuitem1;
	GtkWidget *imagemenuitem5;
	GtkWidget *separatormenuitem1;
	GtkWidget *menuitem2;
	GtkWidget *slide_menu;
	GtkWidget *separator_slide_menu;
	GtkWidget *image_menu;
	GtkWidget *select_all_menu;
	GtkWidget *deselect_all_menu;
	GtkWidget *menuitem3;
	GtkWidget *tmp_image;
	GtkWidget *menu3;
	GtkWidget *about;
	GtkWidget *contents;
	GtkWidget *toolbar;
	GtkWidget *new_button;
	GtkWidget *separatortoolitem;
	GtkWidget *toolbutton_slide_goto;
	GtkWidget *first_slide, *last_slide, *prev_slide, *next_slide, *label_of;
	GtkWidget *hbox;
	GtkWidget *swindow, *scrollable_window;
	GtkWidget *viewport;
	GtkWidget *align;
	GtkWidget *image_area_frame;
	GtkWidget *vbox_frames, *frame1_alignment, *frame2_alignment, *frame3_alignment,*frame4_alignment;
	GtkWidget *frame1, *frame2, *frame3, *frame4, *frame_label;
	GtkWidget *transition_label;
	GtkWidget *vbox_info_slide, *vbox_slide_motion, *vbox_slide_caption;
	GtkWidget *table;
	GtkWidget *duration_label;
	GtkWidget *trans_duration_label;
	GtkWidget *total_time;
	GtkWidget *hbox_stop_points, *stop_points_label, *left_point_button,*right_point_button;
	GtkWidget *hbox_time_offset, *time_offset_label, *add_stop_point_button, *remove_stop_point_button;
	GtkWidget *caption_textview, *hbox_textview, *font_button, *text_animation_hbox;
	GtkWidget *hbox_music_label;
	GtkWidget *music_time;
	GtkWidget *hbox_buttons, *move_up_button;
	GtkWidget *move_down_button, *clear_button, *image_buttons, *vbox2, *scrolledwindow1;
	GtkAccelGroup *accel_group;
	GtkCellRenderer *renderer, *pixbuf_cell;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkIconSize tmp_toolbar_icon_size;
	GtkIconTheme *icon_theme;
	GdkPixbuf *icon;
	gint x;
	GtkWidget *eventbox;

	/* Added after cleaning up the img_window_struct */
	GtkWidget *properties_menu;
	GtkWidget *close_menu;
	GtkWidget *import_menu;
	GtkWidget *import_audio_menu;
	GtkWidget *import_button;
	GtkWidget *import_audio_button;
	GtkWidget *remove_menu;
	GtkWidget *remove_button;
	GtkWidget *zoom_in_button, *zoom_out_button, *zoom_normal;
	GtkWidget *export_menu;
	GtkWidget *thumb_scrolledwindow;
	GdkPixbuf *pixbuf;

	accel_group = gtk_accel_group_new();
	icon_theme = gtk_icon_theme_get_default();
	icon = gtk_icon_theme_load_icon(icon_theme, "imagination", 24, 0, NULL);

	img_struct = g_new0(img_window_struct, 1);
	img_struct->imagination_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon (GTK_WINDOW(img_struct->imagination_window),icon);
	gtk_window_set_position (GTK_WINDOW(img_struct->imagination_window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (img_struct->imagination_window), 1072, 700);
	img_set_window_title(img_struct,NULL);
	g_signal_connect (G_OBJECT (img_struct->imagination_window),"delete-event",G_CALLBACK (img_quit_application),img_struct);
	g_signal_connect (G_OBJECT (img_struct->imagination_window), "destroy", G_CALLBACK (gtk_main_quit), NULL );

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (img_struct->imagination_window), vbox1);

	/* Create the menu items */
	menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), menubar, FALSE, TRUE, 0);

	menuitem1 = gtk_menu_item_new_with_mnemonic (_("_Slideshow"));
	gtk_container_add (GTK_CONTAINER (menubar), menuitem1);

	menu1 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem1), menu1);

	imagemenuitem1 = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), imagemenuitem1);
	g_signal_connect (G_OBJECT (imagemenuitem1),"activate",G_CALLBACK (img_new_slideshow),img_struct);

	img_struct->open_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), img_struct->open_menu);
	g_signal_connect (G_OBJECT (img_struct->open_menu),"activate",G_CALLBACK (img_choose_slideshow_filename),img_struct);

	img_struct->save_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), img_struct->save_menu);
	g_signal_connect (G_OBJECT (img_struct->save_menu),"activate",G_CALLBACK (img_choose_slideshow_filename),img_struct);

	img_struct->save_as_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE_AS, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), img_struct->save_as_menu);
	g_signal_connect (G_OBJECT (img_struct->save_as_menu),"activate",G_CALLBACK (img_choose_slideshow_filename),img_struct);

	close_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), close_menu);
	g_signal_connect (G_OBJECT (close_menu),"activate",G_CALLBACK (img_close_slideshow),img_struct);

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);

	properties_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), properties_menu);
	g_signal_connect (G_OBJECT (properties_menu), "activate", G_CALLBACK (img_project_properties), img_struct);

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);

	img_struct->preview_menu = gtk_image_menu_item_new_with_mnemonic (_("_Preview"));
	gtk_container_add (GTK_CONTAINER (menu1), img_struct->preview_menu);
	gtk_widget_add_accelerator (img_struct->preview_menu, "activate",accel_group,GDK_p,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (img_struct->preview_menu),"activate",G_CALLBACK (img_start_stop_preview),img_struct);

	tmp_image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (img_struct->preview_menu),tmp_image);

	export_menu = gtk_image_menu_item_new_with_mnemonic (_("Export"));
	gtk_container_add (GTK_CONTAINER (menu1), export_menu);

	image_menu = img_load_icon ("imagination-generate.png",GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_menu),image_menu);

	/* Attach subitems to export menu */
	img_create_export_menu( export_menu, img_struct );

	separatormenuitem1 = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu1), separatormenuitem1);

	imagemenuitem5 = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, accel_group);
	gtk_container_add (GTK_CONTAINER (menu1), imagemenuitem5);
	g_signal_connect (G_OBJECT (imagemenuitem5),"activate",G_CALLBACK (img_quit_menu),img_struct);

	menuitem2 = gtk_menu_item_new_with_mnemonic (_("_Slide"));
	gtk_container_add (GTK_CONTAINER (menubar), menuitem2);

	slide_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), slide_menu);

	import_menu = gtk_image_menu_item_new_with_mnemonic (_("Import p_ictures"));
	gtk_container_add (GTK_CONTAINER (slide_menu),import_menu);
	gtk_widget_add_accelerator (import_menu,"activate",accel_group,GDK_i,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (import_menu),"activate",G_CALLBACK (img_add_slides_thumbnails),img_struct);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"image", GTK_ICON_SIZE_MENU, 0, NULL);
	image_menu = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (import_menu),image_menu);
	
	import_audio_menu = gtk_image_menu_item_new_with_mnemonic (_("Import _music"));
	gtk_container_add (GTK_CONTAINER (slide_menu),import_audio_menu);
	gtk_widget_add_accelerator (import_audio_menu,"activate",accel_group,GDK_m,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (import_audio_menu),"activate",G_CALLBACK (img_select_audio_files_to_add),img_struct);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"sound", GTK_ICON_SIZE_MENU, 0, NULL);
	image_menu = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (import_audio_menu),image_menu);

	remove_menu = gtk_image_menu_item_new_with_mnemonic (_("Dele_te"));
	gtk_container_add (GTK_CONTAINER (slide_menu), remove_menu);
	gtk_widget_add_accelerator (remove_menu,"activate",accel_group, GDK_Delete,0,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (remove_menu),"activate",G_CALLBACK (img_delete_selected_slides),img_struct);
	
	tmp_image = gtk_image_new_from_stock (GTK_STOCK_DELETE,GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (remove_menu),tmp_image);

	/* Preview quality menu */
	menuitem1 = gtk_menu_item_new_with_mnemonic( _("Preview quality") );
	gtk_menu_shell_append( GTK_MENU_SHELL( slide_menu ), menuitem1 );

	menu3 = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( menuitem1 ), menu3 );

	menuitem2 = gtk_radio_menu_item_new_with_mnemonic( NULL, _("_Low") );
	//g_signal_connect( G_OBJECT( menuitem2 ), "toggled", G_CALLBACK( img_quality_toggled ), img_struct );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu3 ), menuitem2 );
	menuitem3 = gtk_radio_menu_item_new_with_mnemonic_from_widget(GTK_RADIO_MENU_ITEM( menuitem2 ), _("High") );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu3 ), menuitem3 );

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"object-rotate-left",GTK_ICON_SIZE_MENU,0,NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	/* Rotate menu */
	img_struct->rotate_left_menu = gtk_image_menu_item_new_with_mnemonic (_("Rotate _clockwise"));
	gtk_container_add (GTK_CONTAINER (slide_menu),img_struct->rotate_left_menu);
	gtk_widget_add_accelerator (img_struct->rotate_left_menu,"activate",accel_group, GDK_c,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (img_struct->rotate_left_menu),"activate",G_CALLBACK (img_rotate_selected_slide),img_struct);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (img_struct->rotate_left_menu),tmp_image);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"object-rotate-right",GTK_ICON_SIZE_MENU,0,NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	img_struct->rotate_right_menu = gtk_image_menu_item_new_with_mnemonic (_("Rotate co_unter-clockwise"));
	gtk_container_add (GTK_CONTAINER (slide_menu),img_struct->rotate_right_menu);
	gtk_widget_add_accelerator (img_struct->rotate_right_menu,"activate",accel_group, GDK_u,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (img_struct->rotate_right_menu),"activate",G_CALLBACK (img_rotate_selected_slide),img_struct);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (img_struct->rotate_right_menu),tmp_image);

	/* Zoom controls */
	menuitem1 = gtk_menu_item_new_with_mnemonic( _("_Zoom") );
	gtk_menu_shell_append( GTK_MENU_SHELL( slide_menu ), menuitem1 );

	menu3 = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( menuitem1 ), menu3 );

	menuitem2 = gtk_image_menu_item_new_from_stock( GTK_STOCK_ZOOM_IN,accel_group );
	gtk_widget_add_accelerator( menuitem2, "activate", accel_group, GDK_plus,GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
	//g_signal_connect( G_OBJECT( menuitem2 ), "activate", G_CALLBACK( img_image_area_zoom_in ), img_struct );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu3 ), menuitem2 );

	menuitem2 = gtk_image_menu_item_new_from_stock( GTK_STOCK_ZOOM_OUT, accel_group );
	gtk_widget_add_accelerator( menuitem2, "activate", accel_group, GDK_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
	//g_signal_connect( G_OBJECT( menuitem2 ), "activate", G_CALLBACK( img_image_area_zoom_out ), img_struct );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu3 ), menuitem2 );

	menuitem2 = gtk_image_menu_item_new_from_stock( GTK_STOCK_ZOOM_100,	accel_group );
	gtk_widget_add_accelerator( menuitem2, "activate", accel_group, GDK_0,GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
	//g_signal_connect( G_OBJECT( menuitem2 ), "activate", G_CALLBACK( img_image_area_zoom_reset ), img_struct );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu3 ), menuitem2 );

	separator_slide_menu = gtk_separator_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (slide_menu),separator_slide_menu);

	select_all_menu = gtk_image_menu_item_new_from_stock (GTK_STOCK_SELECT_ALL, accel_group);
	gtk_container_add (GTK_CONTAINER (slide_menu),select_all_menu);
	gtk_widget_add_accelerator (select_all_menu,"activate",accel_group,GDK_a,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (select_all_menu),"activate",G_CALLBACK (img_select_all_thumbnails),img_struct);

	deselect_all_menu = gtk_image_menu_item_new_with_mnemonic (_("Un_select all"));
	gtk_container_add (GTK_CONTAINER (slide_menu),deselect_all_menu);
	gtk_widget_add_accelerator (deselect_all_menu,"activate",accel_group,GDK_e,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (deselect_all_menu),"activate",G_CALLBACK (img_unselect_all_thumbnails),img_struct);

	menuitem3 = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_container_add (GTK_CONTAINER (menubar), menuitem3);
	menu3 = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem3), menu3);

	contents = gtk_image_menu_item_new_with_mnemonic (_("Contents"));
	gtk_container_add (GTK_CONTAINER (menu3),contents);
	gtk_widget_add_accelerator (contents,"activate",accel_group,GDK_F1,GDK_MODE_DISABLED,GTK_ACCEL_VISIBLE);
	g_signal_connect (G_OBJECT (contents),"activate",G_CALLBACK (img_show_uri),img_struct);

	tmp_image = gtk_image_new_from_stock ("gtk-help",GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (contents),tmp_image);

	about = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, accel_group);
	gtk_container_add (GTK_CONTAINER (menu3), about);
	gtk_widget_show_all (menubar);
	g_signal_connect (G_OBJECT (about),"activate",G_CALLBACK (img_show_about_dialog),img_struct);

	/* Create the toolbar */
	toolbar = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), toolbar, FALSE, FALSE, 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
	tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar));

	new_button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_NEW));
	gtk_container_add (GTK_CONTAINER (toolbar),new_button);
	gtk_widget_set_tooltip_text(new_button, _("Create a new slideshow"));
	g_signal_connect (G_OBJECT (new_button),"clicked",G_CALLBACK (img_new_slideshow),img_struct);

	img_struct->open_button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_OPEN));
	gtk_container_add (GTK_CONTAINER (toolbar),img_struct->open_button);
	gtk_widget_set_tooltip_text(img_struct->open_button, _("Open a slideshow"));
	g_signal_connect (G_OBJECT (img_struct->open_button),"clicked",G_CALLBACK (img_choose_slideshow_filename),img_struct);

	img_struct->save_button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_SAVE));
	gtk_container_add (GTK_CONTAINER (toolbar), img_struct->save_button);
	gtk_widget_set_tooltip_text(img_struct->save_button, _("Save the slideshow"));
	g_signal_connect (G_OBJECT (img_struct->save_button),"clicked",G_CALLBACK (img_choose_slideshow_filename),img_struct);

	separatortoolitem = GTK_WIDGET (gtk_separator_tool_item_new());
	gtk_widget_show (separatortoolitem);
	gtk_container_add (GTK_CONTAINER (toolbar),separatortoolitem);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"image", 20, 0, NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	import_button = GTK_WIDGET (gtk_tool_button_new (tmp_image,""));
	gtk_container_add (GTK_CONTAINER (toolbar),import_button);
	gtk_widget_set_tooltip_text(import_button, _("Import pictures"));
	g_signal_connect ((gpointer)import_button, "clicked", G_CALLBACK (img_add_slides_thumbnails),img_struct);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"sound", 20, 0, NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	import_audio_button = GTK_WIDGET (gtk_tool_button_new (tmp_image,""));
	gtk_container_add (GTK_CONTAINER (toolbar),import_audio_button);
	gtk_widget_set_tooltip_text(import_audio_button, _("Import music"));
	g_signal_connect(G_OBJECT(import_audio_button), "clicked", G_CALLBACK(img_select_audio_files_to_add), img_struct);

	remove_button = GTK_WIDGET (gtk_tool_button_new_from_stock ("gtk-delete"));
	gtk_container_add (GTK_CONTAINER (toolbar),remove_button);
	gtk_widget_set_tooltip_text(remove_button, _("Delete the selected slides"));
	g_signal_connect (G_OBJECT (remove_button),"clicked",G_CALLBACK (img_delete_selected_slides),img_struct);

	pixbuf = gtk_icon_theme_load_icon(icon_theme,"object-rotate-left",GTK_ICON_SIZE_LARGE_TOOLBAR,0,NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	img_struct->rotate_left_button = GTK_WIDGET (gtk_tool_button_new(tmp_image,""));
	gtk_container_add (GTK_CONTAINER (toolbar), img_struct->rotate_left_button);
	gtk_widget_set_tooltip_text(img_struct->rotate_left_button, _("Rotate the slide clockwise"));
	g_signal_connect (G_OBJECT (img_struct->rotate_left_button),"clicked",G_CALLBACK (img_rotate_selected_slide),img_struct);
	
	pixbuf = gtk_icon_theme_load_icon(icon_theme,"object-rotate-right",GTK_ICON_SIZE_LARGE_TOOLBAR,0,NULL);
	tmp_image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	img_struct->rotate_right_button = GTK_WIDGET (gtk_tool_button_new(tmp_image,""));
	gtk_container_add (GTK_CONTAINER (toolbar),img_struct->rotate_right_button);
	gtk_widget_set_tooltip_text(img_struct->rotate_right_button, _("Rotate the slide counter-clockwise"));
	g_signal_connect (G_OBJECT (img_struct->rotate_right_button),"clicked",G_CALLBACK (img_rotate_selected_slide),img_struct);

	zoom_in_button = GTK_WIDGET (gtk_tool_button_new_from_stock ("gtk-zoom-in"));
	gtk_container_add (GTK_CONTAINER (toolbar),zoom_in_button);
	gtk_widget_set_tooltip_text(zoom_in_button, _("Zoom In"));
	//g_signal_connect (G_OBJECT (zoom_in_button),"clicked",G_CALLBACK (img_image_area_zoom_in),img_struct);

	zoom_out_button = GTK_WIDGET (gtk_tool_button_new_from_stock ("gtk-zoom-out"));
	gtk_container_add (GTK_CONTAINER (toolbar),zoom_out_button);
	gtk_widget_set_tooltip_text(zoom_out_button, _("Zoom Out"));
	//g_signal_connect (G_OBJECT (zoom_out_button),"clicked",G_CALLBACK (img_image_area_zoom_out),img_struct);

	zoom_normal = GTK_WIDGET (gtk_tool_button_new_from_stock ("gtk-zoom-fit"));
	gtk_container_add (GTK_CONTAINER (toolbar),zoom_normal);
	gtk_widget_set_tooltip_text(zoom_normal, _("Normal Size"));
	//g_signal_connect (G_OBJECT (zoom_normal),"clicked",G_CALLBACK (img_image_area_zoom_reset),img_struct);

	separatortoolitem = GTK_WIDGET (gtk_separator_tool_item_new());
	gtk_container_add (GTK_CONTAINER (toolbar),separatortoolitem);

	img_struct->preview_button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PLAY));
	gtk_container_add (GTK_CONTAINER (toolbar),img_struct->preview_button);
	gtk_widget_set_tooltip_text(img_struct->preview_button, _("Starts the preview"));
	g_signal_connect (G_OBJECT (img_struct->preview_button),"clicked",G_CALLBACK (img_start_stop_preview),img_struct);

	separatortoolitem = GTK_WIDGET (gtk_separator_tool_item_new());
	gtk_container_add (GTK_CONTAINER (toolbar),separatortoolitem);

	first_slide = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_FIRST));
	gtk_container_add (GTK_CONTAINER (toolbar),first_slide);
	gtk_widget_set_tooltip_text(first_slide, _("Goto first slide"));
	g_signal_connect (G_OBJECT (first_slide),"clicked",G_CALLBACK (img_goto_first_slide),img_struct);

	prev_slide = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK));
	gtk_container_add (GTK_CONTAINER (toolbar),prev_slide);
	gtk_widget_set_tooltip_text(prev_slide, _("Goto previous selected slide"));
	g_signal_connect (G_OBJECT (prev_slide),"clicked",G_CALLBACK (img_goto_prev_slide),img_struct);

	toolbutton_slide_goto = GTK_WIDGET(gtk_tool_item_new ());
	gtk_container_add (GTK_CONTAINER (toolbar), toolbutton_slide_goto);
 
	img_struct->slide_number_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY (img_struct->slide_number_entry), 4);
	gtk_entry_set_width_chars(GTK_ENTRY (img_struct->slide_number_entry), 6);
	gtk_container_add(GTK_CONTAINER(toolbutton_slide_goto),img_struct->slide_number_entry);
	g_signal_connect(G_OBJECT (img_struct->slide_number_entry), "activate", G_CALLBACK(img_goto_line_entry_activate), img_struct);
	g_signal_connect(G_OBJECT (img_struct->slide_number_entry), "insert-text", G_CALLBACK (img_check_numeric_entry), NULL );

	toolbutton_slide_goto = GTK_WIDGET(gtk_tool_item_new ());
	gtk_container_add (GTK_CONTAINER (toolbar), toolbutton_slide_goto);

	label_of = gtk_label_new(_(" of "));
	gtk_container_add (GTK_CONTAINER (toolbutton_slide_goto),label_of);

	toolbutton_slide_goto = GTK_WIDGET(gtk_tool_item_new ());
	gtk_container_add (GTK_CONTAINER (toolbar), toolbutton_slide_goto);

	img_struct->total_slide_number_label = gtk_label_new(NULL);
	gtk_container_add (GTK_CONTAINER (toolbutton_slide_goto),img_struct->total_slide_number_label);

	next_slide = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD));
	gtk_container_add (GTK_CONTAINER (toolbar),next_slide);
	gtk_widget_set_tooltip_text(next_slide, _("Goto next selected slide"));
	g_signal_connect (G_OBJECT (next_slide),"clicked",G_CALLBACK (img_goto_next_slide),img_struct);

	last_slide = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_LAST));
	gtk_container_add (GTK_CONTAINER (toolbar),last_slide);
	gtk_widget_set_tooltip_text(last_slide, _("Goto last slide"));
	g_signal_connect (G_OBJECT (last_slide),"clicked",G_CALLBACK (img_goto_last_slide),img_struct);

	gtk_widget_show_all (toolbar);

	/* Create the image area and the other widgets */
	hbox = gtk_hpaned_new();
	gtk_box_pack_start (GTK_BOX (vbox1), hbox, TRUE, TRUE, 0);

	swindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_add1( GTK_PANED( hbox ), swindow );

	align = gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(swindow), align);

	image_area_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(image_area_frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(align), image_area_frame);

	align = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(image_area_frame), align);

	img_struct->image_area = gtk_image_new();
	gtk_widget_set_size_request(img_struct->image_area, 720, 576);
	gtk_container_add(GTK_CONTAINER(align), img_struct->image_area);

	viewport = gtk_bin_get_child(GTK_BIN(swindow));
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);

	vbox_frames = gtk_vbox_new(FALSE, 10);
	scrollable_window = gtk_scrolled_window_new(NULL, NULL);
	g_object_set (G_OBJECT (scrollable_window),"hscrollbar-policy",GTK_POLICY_AUTOMATIC,"vscrollbar-policy",GTK_POLICY_AUTOMATIC,NULL);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollable_window), vbox_frames);
	gtk_paned_add2( GTK_PANED( hbox ), scrollable_window );
	gtk_paned_set_position( GTK_PANED( hbox ), 650 );

	viewport = gtk_bin_get_child(GTK_BIN(scrollable_window));
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_container_set_border_width( GTK_CONTAINER( viewport ), 10 );

	/* Slide frame */
	frame1 = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_frames), frame1, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_OUT);

	frame1_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame1), frame1_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (frame1_alignment), 2, 2, 5, 5);

	frame_label = gtk_label_new (_("<b>Slide Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame1), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (frame_label), 2, 2);

	vbox_info_slide = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (frame1_alignment), vbox_info_slide);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_info_slide), 2);

	/* Transition types label */
	transition_label = gtk_label_new (_("Transition Type:"));
	gtk_box_pack_start (GTK_BOX (vbox_info_slide), transition_label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (transition_label), 0, -1);

	/* Slide selected, slide resolution, slide type and slide total duration */
	table = gtk_table_new (4, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox_info_slide), table, TRUE, TRUE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);

	/* Transition type */
	img_struct->transition_type = _gtk_combo_box_new_text(TRUE);
	gtk_table_attach (GTK_TABLE (table), img_struct->transition_type, 0, 1, 0, 1,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(gtk_combo_box_get_model(GTK_COMBO_BOX(img_struct->transition_type))),
										1, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(gtk_combo_box_get_model(GTK_COMBO_BOX(img_struct->transition_type))),1, img_sort_none_before_other,NULL,NULL);

	gtk_widget_set_sensitive(img_struct->transition_type, FALSE);
	g_signal_connect (G_OBJECT (img_struct->transition_type), "changed",G_CALLBACK (img_combo_box_transition_type_changed),img_struct);

	img_struct->random_button = gtk_button_new_with_mnemonic (_("Random"));
	gtk_widget_set_tooltip_text(img_struct->random_button,_("Imagination randomly decides which transition to apply"));
	gtk_table_attach (GTK_TABLE (table), img_struct->random_button, 1, 2, 0, 1,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_sensitive(img_struct->random_button, FALSE);
	g_signal_connect (G_OBJECT (img_struct->random_button),"clicked",G_CALLBACK (img_random_button_clicked),img_struct);

	/* Transition duration */
	trans_duration_label = gtk_label_new (_("Transition Speed:"));
	gtk_table_attach (GTK_TABLE (table), trans_duration_label, 0, 1, 1, 2,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (trans_duration_label), 0, 0.5);

	img_struct->trans_duration = _gtk_combo_box_new_text(FALSE);
	gtk_table_attach (GTK_TABLE (table), img_struct->trans_duration, 1, 2, 1, 2,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (GTK_FILL), 0, 0);
	{
		GtkTreeIter   iter;
		GtkListStore *store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( img_struct->trans_duration ) ) );

		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, _("Fast"), -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, _("Normal"), -1 );
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, _("Slow"), -1 );
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(img_struct->trans_duration),1);
	gtk_widget_set_sensitive(img_struct->trans_duration, FALSE);
	g_signal_connect (G_OBJECT (img_struct->trans_duration),"changed",G_CALLBACK (img_combo_box_speed_changed),img_struct);

	/* Slide duration */
	duration_label = gtk_label_new (_("Slide Duration in sec:"));
	gtk_table_attach (GTK_TABLE (table), duration_label, 0, 1, 2, 3,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (duration_label), 0, 0.5);

	img_struct->duration = gtk_spin_button_new_with_range (1, 300, 1);
	gtk_table_attach (GTK_TABLE (table), img_struct->duration, 1, 2, 2, 3,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_sensitive(img_struct->duration, FALSE);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON (img_struct->duration),TRUE);
	g_signal_connect (G_OBJECT (img_struct->duration),"value-changed",G_CALLBACK (img_spinbutton_value_changed),img_struct);

	/* Slide Total Duration */
	total_time = gtk_label_new (_("Slideshow Length:"));
	gtk_table_attach (GTK_TABLE (table), total_time, 0, 1, 3, 4,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (total_time), 0, 0.5);

	img_struct->total_time_data = gtk_label_new ("");
	gtk_table_attach (GTK_TABLE (table), img_struct->total_time_data, 1, 2, 3, 4,(GtkAttachOptions) (GTK_FILL),(GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (img_struct->total_time_data), 0, 0.5);

	/* Slide motion frame */
	frame2 = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_frames), frame2, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_OUT);

	frame2_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame2), frame2_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (frame2_alignment), 2, 2, 5, 5);

	frame_label = gtk_label_new (_("<b>Slide Motion</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame2), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (frame_label), 2, 2);

	vbox_slide_motion = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (frame2_alignment), vbox_slide_motion);

	hbox_stop_points = gtk_hbox_new(FALSE,5);
	gtk_box_pack_start (GTK_BOX (vbox_slide_motion), hbox_stop_points, TRUE, FALSE, 0);
	stop_points_label = gtk_label_new(("Stop Points:"));
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), stop_points_label, TRUE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(stop_points_label),0.0, 0.5);
	left_point_button = gtk_button_new();
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), left_point_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
	gtk_button_set_image(GTK_BUTTON(left_point_button), image_buttons);
	img_struct->img_current_stop_point_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY (img_struct->img_current_stop_point_entry), 2);
	gtk_entry_set_width_chars(GTK_ENTRY (img_struct->img_current_stop_point_entry), 4);
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), img_struct->img_current_stop_point_entry, FALSE, TRUE, 0);
	label_of = gtk_label_new(_(" of "));
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), label_of, FALSE, FALSE, 0);
	img_struct->total_stop_points_label = gtk_label_new(NULL);
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), img_struct->total_stop_points_label, FALSE, FALSE, 0);
	right_point_button = gtk_button_new();
	gtk_box_pack_start (GTK_BOX (hbox_stop_points), right_point_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
	gtk_button_set_image(GTK_BUTTON(right_point_button), image_buttons);

	hbox_time_offset = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX (vbox_slide_motion), hbox_time_offset, FALSE, FALSE, 0);
	time_offset_label = gtk_label_new(("Time Offset:"));
	gtk_box_pack_start (GTK_BOX (hbox_time_offset), time_offset_label, TRUE, TRUE, 0);
	gtk_misc_set_alignment(GTK_MISC(time_offset_label),0.0, 0.5);
	img_struct->stop_point_duration = gtk_spin_button_new_with_range (1, 60, 1);
	gtk_box_pack_start (GTK_BOX (hbox_time_offset), img_struct->stop_point_duration, FALSE, FALSE, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON (img_struct->stop_point_duration),TRUE);
	//g_signal_connect (G_OBJECT (img_struct->stop_point_duration),"value-changed",G_CALLBACK (img_time_offset_spin_button_value_changed),img_struct);
	GtkWidget *hbox_zoom = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX (vbox_slide_motion), hbox_zoom, FALSE, FALSE, 0);

	GtkWidget *label = gtk_label_new(_("Zoom: "));
	gtk_misc_set_alignment(GTK_MISC(label),0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox_zoom), label, FALSE, TRUE, 0);

	GtkWidget *zoom_scale = gtk_hscale_new_with_range(1,30,0.10000000000000001);
	gtk_scale_set_value_pos (GTK_SCALE(zoom_scale), GTK_POS_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox_zoom), zoom_scale, TRUE, TRUE, 0);
	
	hbox_buttons = gtk_hbutton_box_new();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox_buttons), GTK_BUTTONBOX_SPREAD);
	gtk_box_pack_start (GTK_BOX (vbox_slide_motion), hbox_buttons, FALSE, FALSE, 0);
	add_stop_point_button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start (GTK_BOX (hbox_buttons), add_stop_point_button, FALSE, FALSE, 0);
	remove_stop_point_button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start (GTK_BOX (hbox_buttons), remove_stop_point_button, FALSE, FALSE, 0);

	/* Slide text frame */
	frame4 = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_frames), frame4, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame4), GTK_SHADOW_OUT);

	frame4_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame4), frame4_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (frame4_alignment), 5, 5, 5, 5);

	frame_label = gtk_label_new (_("<b>Slide Text</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame4), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (frame_label), 2, 2);

	vbox_slide_caption = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (frame4_alignment), vbox_slide_caption);

	hbox_textview = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox_slide_caption), hbox_textview, FALSE, FALSE, 0);
	caption_textview = gtk_text_view_new();
	img_struct->slide_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(caption_textview));
	img_struct->scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(img_struct->scrolled_win, -1, 18);
	g_object_set (G_OBJECT (img_struct->scrolled_win),"hscrollbar-policy",GTK_POLICY_AUTOMATIC,"vscrollbar-policy",GTK_POLICY_AUTOMATIC,NULL);
	gtk_container_add(GTK_CONTAINER (img_struct->scrolled_win), caption_textview);
	gtk_box_pack_start (GTK_BOX (hbox_textview), img_struct->scrolled_win, TRUE, TRUE, 0);
	img_struct->expand_button = gtk_button_new();
	gtk_widget_set_tooltip_text(img_struct->expand_button, _("Click to expand the area"));
	gtk_button_set_relief(GTK_BUTTON(img_struct->expand_button), GTK_RELIEF_NONE);
	gtk_widget_set_size_request(img_struct->expand_button, 22, 20);
	g_signal_connect (G_OBJECT (img_struct->expand_button),"clicked",G_CALLBACK (img_expand_button_clicked),img_struct);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
	gtk_widget_set_size_request(image_buttons, 14, 14);
	gtk_button_set_image(GTK_BUTTON(img_struct->expand_button),image_buttons);
	gtk_box_pack_start (GTK_BOX (hbox_textview), img_struct->expand_button, FALSE, FALSE, 0);
	font_button = gtk_font_button_new();
	gtk_box_pack_start (GTK_BOX (vbox_slide_caption), font_button, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text(font_button, _("Click to choose the font"));
	text_animation_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_slide_caption), text_animation_hbox, FALSE, FALSE, 0);
	GtkWidget *animation_label = gtk_label_new(_("Animation:"));
	gtk_misc_set_alignment(GTK_MISC(animation_label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (text_animation_hbox), animation_label, TRUE, TRUE, 0);
	img_struct->text_animation_combo = _gtk_combo_box_new_text(FALSE);
	gtk_box_pack_start (GTK_BOX (text_animation_hbox), img_struct->text_animation_combo, FALSE, FALSE, 0);
	{
		GtkTreeIter   iter;
		GtkListStore *store = GTK_LIST_STORE( gtk_combo_box_get_model(GTK_COMBO_BOX(img_struct->text_animation_combo)));
		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter, 0, _("None"), -1 );
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(img_struct->text_animation_combo), 0);
	/* Background music frame */
	frame3 = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_frames), frame3, FALSE, FALSE, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_OUT);

	frame3_alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame3), frame3_alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (frame3_alignment), 5, 5, 5, 5);

	frame_label = gtk_label_new (_("<b>Background Music</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (frame3), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (frame_label), 2, 2);

	/* Add the liststore */
	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (frame3_alignment), vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 0);
	
	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow1, FALSE, FALSE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);
	gtk_widget_set_size_request(scrolledwindow1, -1, 73);

	img_struct->music_file_liststore = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
	g_signal_connect (G_OBJECT (img_struct->music_file_liststore), "row-inserted",	G_CALLBACK (img_activate_remove_button_music_liststore) , img_struct);

	img_struct->music_file_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(img_struct->music_file_liststore));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(img_struct->music_file_treeview));

	gtk_drag_dest_set (GTK_WIDGET(img_struct->music_file_treeview),GTK_DEST_DEFAULT_ALL,drop_targets,1,GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK);
	g_signal_connect (G_OBJECT (img_struct->music_file_treeview),"drag-data-received",G_CALLBACK (img_on_drag_audio_data_received), img_struct);

	/* First and last column aren't displayed, so we only need two columns. */
	for (x = 1; x < 3; x++)
	{
		column = gtk_tree_view_column_new();
		renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_add_attribute(column, renderer, "text", x);
		gtk_tree_view_append_column (GTK_TREE_VIEW (img_struct->music_file_treeview), column);
	}

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(img_struct->music_file_treeview), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (img_struct->music_file_treeview), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), img_struct->music_file_treeview);

	/* Add the total music labels and the buttons */
	hbox_music_label = gtk_hbox_new(FALSE, 2);
	gtk_container_add (GTK_CONTAINER ( vbox2), hbox_music_label);

	music_time = gtk_label_new(_("Music Duration:"));
	gtk_box_pack_start(GTK_BOX(hbox_music_label), music_time, TRUE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (music_time), 0, 0.5);

	img_struct->music_time_data = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox_music_label), img_struct->music_time_data, TRUE, TRUE, 0);
	gtk_misc_set_alignment (GTK_MISC (img_struct->music_time_data), 1, 0.5);

	hbox_buttons = gtk_hbox_new(TRUE, 2);
	gtk_container_add (GTK_CONTAINER ( vbox2), hbox_buttons);

	img_struct->play_audio_button = gtk_button_new();
	gtk_widget_set_sensitive(img_struct->play_audio_button, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox_buttons), img_struct->play_audio_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (img_struct->play_audio_button), image_buttons);
	gtk_widget_set_tooltip_text(img_struct->play_audio_button, _("Play the selected file"));
	g_signal_connect ( (gpointer) img_struct->play_audio_button, "clicked", G_CALLBACK (img_play_stop_selected_file), img_struct);

	img_struct->remove_audio_button = gtk_button_new();
	gtk_widget_set_sensitive(img_struct->remove_audio_button, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox_buttons), img_struct->remove_audio_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (img_struct->remove_audio_button), image_buttons);
	gtk_widget_set_tooltip_text(img_struct->remove_audio_button, _("Delete the selected file"));
	g_signal_connect ( (gpointer) img_struct->remove_audio_button, "clicked", G_CALLBACK (img_remove_audio_files), img_struct);

	move_up_button = gtk_button_new();
	gtk_box_pack_start(GTK_BOX(hbox_buttons), move_up_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (move_up_button), image_buttons);
	gtk_widget_set_tooltip_text(move_up_button, _("Move the selected file up"));
	g_signal_connect( G_OBJECT( move_up_button ), "clicked", G_CALLBACK( img_move_audio_up ), img_struct );

	move_down_button = gtk_button_new();
	gtk_box_pack_start(GTK_BOX(hbox_buttons), move_down_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (move_down_button), image_buttons);
	gtk_widget_set_tooltip_text(move_down_button, _("Move the selected file down"));
	g_signal_connect( G_OBJECT( move_down_button ), "clicked", G_CALLBACK( img_move_audio_down ), img_struct );
	
	clear_button = gtk_button_new();
	gtk_box_pack_start(GTK_BOX(hbox_buttons), clear_button, FALSE, TRUE, 0);
	image_buttons = gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
	gtk_container_add (GTK_CONTAINER (clear_button), image_buttons);
	gtk_widget_set_tooltip_text(clear_button, _("Clear all files"));
	g_signal_connect ( (gpointer) clear_button, "clicked", G_CALLBACK (img_clear_audio_files), img_struct);

	/* Create the model */
	img_struct->thumbnail_model = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_POINTER);

	/* Add wrapper for DnD */
	eventbox = gtk_event_box_new();
	gtk_event_box_set_above_child( GTK_EVENT_BOX( eventbox ), FALSE );
	gtk_event_box_set_visible_window( GTK_EVENT_BOX( eventbox ), FALSE );
	gtk_drag_dest_set( GTK_WIDGET( eventbox ), GTK_DEST_DEFAULT_ALL,
					   drop_targets, 1, GDK_ACTION_COPY | GDK_ACTION_MOVE |
					   GDK_ACTION_LINK | GDK_ACTION_ASK);
	g_signal_connect( G_OBJECT( eventbox ), "drag-data-received",
					  G_CALLBACK( img_on_drag_data_received), img_struct );
	gtk_box_pack_start( GTK_BOX( vbox1 ), eventbox, FALSE, TRUE, 0 );

	/* Create the thumbnail viewer */
	thumb_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request( thumb_scrolledwindow, -1, 115 );
	g_signal_connect( G_OBJECT( thumb_scrolledwindow ), "scroll-event",
					  G_CALLBACK( img_scroll_thumb ), img_struct );
	gtk_container_add( GTK_CONTAINER( eventbox ), thumb_scrolledwindow );
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (thumb_scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (thumb_scrolledwindow), GTK_SHADOW_IN);

	img_struct->thumbnail_iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL (img_struct->thumbnail_model));
	gtk_container_add( GTK_CONTAINER( thumb_scrolledwindow ), img_struct->thumbnail_iconview );

	/* Create the cell layout */
	pixbuf_cell = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (img_struct->thumbnail_iconview), pixbuf_cell, FALSE);
	g_object_set (G_OBJECT (pixbuf_cell), "width", 115, "ypad", 2, NULL);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (img_struct->thumbnail_iconview), pixbuf_cell, "pixbuf", 0, NULL);

	/* Set some iconview properties */
	gtk_icon_view_set_text_column( GTK_ICON_VIEW( img_struct->thumbnail_iconview ), -1 );
	gtk_icon_view_set_reorderable(GTK_ICON_VIEW (img_struct->thumbnail_iconview),TRUE);
	gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (img_struct->thumbnail_iconview), GTK_SELECTION_MULTIPLE);
	gtk_icon_view_set_orientation (GTK_ICON_VIEW (img_struct->thumbnail_iconview), GTK_ORIENTATION_HORIZONTAL);
	gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (img_struct->thumbnail_iconview),0);
	gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (img_struct->thumbnail_iconview),0);
	gtk_icon_view_set_columns (GTK_ICON_VIEW (img_struct->thumbnail_iconview), G_MAXINT);
	g_signal_connect (G_OBJECT (img_struct->thumbnail_iconview),"selection-changed",G_CALLBACK (img_iconview_selection_changed),img_struct);
	g_signal_connect (G_OBJECT (img_struct->thumbnail_iconview),"select-all",G_CALLBACK (img_iconview_selection_changed),img_struct);
	g_signal_connect (G_OBJECT (img_struct->thumbnail_iconview),"button-press-event",G_CALLBACK (img_iconview_selection_button_press),img_struct);
	gtk_widget_show_all( eventbox );

	/* Create the status bar */
	img_struct->statusbar = gtk_statusbar_new ();
	gtk_widget_show (img_struct->statusbar);
	gtk_box_pack_start (GTK_BOX (vbox1), img_struct->statusbar, FALSE, TRUE, 0);
	img_struct->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (img_struct->statusbar), "statusbar");

	/* Create the progress bar */
	img_struct->progress_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request (img_struct->progress_bar, -1, 15);
	{
		GtkWidget *vbox;

		vbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (img_struct->statusbar), vbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox), img_struct->progress_bar, TRUE, FALSE, 0);
		gtk_widget_show (vbox);
	}
	gtk_widget_show_all(hbox);
	gtk_window_add_accel_group (GTK_WINDOW (img_struct->imagination_window), accel_group);

	return img_struct;
}

static void img_clear_audio_files(GtkButton *button, img_window_struct *img)
{
	gtk_list_store_clear(GTK_LIST_STORE(img->music_file_liststore));
	gtk_widget_set_sensitive(img->play_audio_button, FALSE);
	gtk_widget_set_sensitive(img->remove_audio_button, FALSE);
	gtk_label_set_text(GTK_LABEL(img->music_time_data), "");
}

static void img_expand_button_clicked(GtkButton *button, img_window_struct *img)
{
	gint width, height;

	gtk_widget_get_size_request(img->scrolled_win, &width, &height);
	if (height == 18)
	{
		gtk_widget_set_size_request(img->scrolled_win, -1, 90);
		gtk_widget_set_tooltip_text(img->expand_button, _("Click to shrink the area"));
	}
	else
	{
		gtk_widget_set_size_request(img->scrolled_win, -1, 18);
		gtk_widget_set_tooltip_text(img->expand_button, _("Click to expand the area"));
	}
}

static void img_on_drag_audio_data_received (GtkWidget *widget,GdkDragContext *context,int x,int y,GtkSelectionData *data,unsigned int info,unsigned int time, img_window_struct *img)
{
	gchar **audio = NULL;
	gchar *filename,*ttime;
	GtkWidget *dialog;
	gint len = 0;

	audio = gtk_selection_data_get_uris(data);
	if (audio == NULL)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(img->imagination_window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("Sorry,I could not perform the operation!"));
		gtk_window_set_title(GTK_WINDOW(dialog),"Imagination");
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (GTK_WIDGET (dialog));
		gtk_drag_finish(context,FALSE,FALSE,time);
		return;
	}
	gtk_drag_finish (context,TRUE,FALSE,time);
	while(audio[len])
	{
		filename = g_filename_from_uri (audio[len],NULL,NULL);
		img_add_audio_files(filename,img);
		g_free(filename);
		len++;
	}
	ttime = img_convert_seconds_to_time(img->total_music_secs);
	gtk_label_set_text(GTK_LABEL(img->music_time_data), ttime);
	g_free(ttime);
	g_strfreev (audio);
}

static void img_activate_remove_button_music_liststore(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, img_window_struct *img)
{
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(img->music_file_liststore), iter) == TRUE)
	{
		gtk_widget_set_sensitive ( img->remove_audio_button, TRUE);
		gtk_widget_set_sensitive ( img->play_audio_button, TRUE);
	}
}

static void img_quit_menu(GtkMenuItem *menuitem, img_window_struct *img)
{
	gboolean value;

	g_signal_emit_by_name(img->imagination_window,"delete-event", img, &value);

	if(value)
		return;

	/* Unloads the plugins */
	g_slist_foreach(img->plugin_list,(GFunc)g_module_close,NULL);
	g_slist_free(img->plugin_list);

	gtk_main_quit();
}

void img_iconview_selection_changed(GtkIconView *iconview, img_window_struct *img)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint dummy, nr_selected = 0;
	GList *selected = NULL;
	gchar *slide_info_msg = NULL;
	slide_struct *info_slide;

	if (img->preview_is_running || img->export_is_running)
		return;

	model = gtk_icon_view_get_model(iconview);
	gtk_icon_view_get_cursor(iconview,&path,NULL);

	selected = gtk_icon_view_get_selected_items(iconview);
	nr_selected = g_list_length(selected);
	img_set_total_slideshow_duration(img);

	if (selected == NULL)
	{
		img_set_statusbar_message(img,nr_selected);
		gtk_image_set_from_pixbuf(GTK_IMAGE(img->image_area),NULL);
		gtk_widget_set_sensitive(img->trans_duration,	FALSE);
		gtk_widget_set_sensitive(img->duration,			FALSE);
		gtk_widget_set_sensitive(img->transition_type,	FALSE);
		gtk_widget_set_sensitive(img->random_button,	FALSE);
		if (img->slides_nr == 0)
			gtk_label_set_text(GTK_LABEL (img->total_time_data),"");
		return;
	}
	if (nr_selected > 1)
		img_set_statusbar_message(img,nr_selected);
	else
		gtk_statusbar_pop(GTK_STATUSBAR(img->statusbar), img->context_id);

	gtk_widget_set_sensitive(img->trans_duration,	TRUE);
	gtk_widget_set_sensitive(img->duration,			TRUE);
	gtk_widget_set_sensitive(img->transition_type,	TRUE);
	gtk_widget_set_sensitive(img->random_button,	TRUE);

	dummy = gtk_tree_path_get_indices(selected->data)[0]+1;
	gtk_tree_model_get_iter(model,&iter,selected->data);
	g_list_foreach (selected, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (selected);
	gtk_tree_model_get(model,&iter,1,&info_slide,-1);

	/* Set the transition type */
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(img->transition_type));

	/* Block "changed" signal from model to avoid rewriting the same value back into current slide. */
	g_signal_handlers_block_by_func((gpointer)img->transition_type, (gpointer)img_combo_box_transition_type_changed, img);
	{
		GtkTreeIter   iter;
		GtkTreeModel *model;

		model = gtk_combo_box_get_model( GTK_COMBO_BOX( img->transition_type ) );
		gtk_tree_model_get_iter_from_string( model, &iter, info_slide->path );
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(img->transition_type), &iter );
	}
	g_signal_handlers_unblock_by_func((gpointer)img->transition_type, (gpointer)img_combo_box_transition_type_changed, img);

	/* Moved this piece of code below the setting the transition, since we
	 * get false negatives in certain situations (eg.: if the previously
	 * selected transition doesn't have transition renderer set): */
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(img->transition_type)) == 0)
		gtk_widget_set_sensitive(img->trans_duration,FALSE);
	else
		gtk_widget_set_sensitive(img->trans_duration,TRUE);

	/* Set the transition speed */
	if (info_slide->speed == FAST)
		dummy = 0;
	else if (info_slide->speed == NORMAL)
		dummy = 1;
	else
		dummy = 2;
	g_signal_handlers_block_by_func((gpointer)img->duration, (gpointer)img_combo_box_speed_changed, img);
	gtk_combo_box_set_active(GTK_COMBO_BOX(img->trans_duration),dummy);
	g_signal_handlers_block_by_func((gpointer)img->duration, (gpointer)img_combo_box_speed_changed, img);

	/* Set the transition duration */
	g_signal_handlers_block_by_func((gpointer)img->duration, (gpointer)img_spinbutton_value_changed, img);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(img->duration), info_slide->duration);
	g_signal_handlers_unblock_by_func((gpointer)img->duration, (gpointer)img_spinbutton_value_changed, img);

	/* Added missing total label "setter". Current method is not the most
	 * efficient one, since we're recalculating the whole duration when we
	 * only need to display it. BTW, is total duration label hidding needed?
	 * Even when there is no slide selected, duration stays the same. */
	img->project_is_modified = TRUE;

	if (nr_selected > 1)
		img_set_statusbar_message(img,nr_selected);
	else
	{
		slide_info_msg = g_strdup_printf("%s    %s: %s    %s: %s",info_slide->filename, _("Resolution"), info_slide->resolution, _("Type"), info_slide->type);
		gtk_statusbar_push(GTK_STATUSBAR (img->statusbar), img->context_id, slide_info_msg);
		g_free(slide_info_msg);
	}

	img->slide_pixbuf = img_scale_pixbuf(img,info_slide->filename);
	gtk_image_set_from_pixbuf(GTK_IMAGE (img->image_area),img->slide_pixbuf);
	g_object_unref(img->slide_pixbuf);
}

static void img_combo_box_transition_type_changed (GtkComboBox *combo, img_window_struct *img)
{
	GList *selected, *bak;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gpointer address;
	slide_struct *info_slide;
	gint transition_id;
	GtkTreePath *p;
	gchar       *path;

	/* Get the address of the transition function stored in the model of the combo box*/
	model = gtk_combo_box_get_model(combo);
	gtk_combo_box_get_active_iter(combo, &iter);
	gtk_tree_model_get(model, &iter, 2, &address, 3, &transition_id, -1);

	/* Get index of currently selected item */
	if (transition_id == -1)
		gtk_widget_set_sensitive(img->trans_duration,FALSE);
	else
		gtk_widget_set_sensitive(img->trans_duration,TRUE);

	/* If user selected group name, automatically select first transition
	 * from this group. */
	if( transition_id == 0 )
	{
		GtkTreeIter parent = iter;
		gtk_tree_model_iter_nth_child( model, &iter, &parent, 0 );
		gtk_tree_model_get(model, &iter, 2, &address, 3, &transition_id, -1);
		g_signal_handlers_block_by_func(img->transition_type,
										img_combo_box_transition_type_changed,
										img);
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(img->transition_type),
									  &iter );
		g_signal_handlers_unblock_by_func(img->transition_type,
										  img_combo_box_transition_type_changed,
										  img);
	}
	p = gtk_tree_model_get_path( model, &iter );
	path = gtk_tree_path_to_string( p );
	gtk_tree_path_free( p );

	model = gtk_icon_view_get_model(GTK_ICON_VIEW (img->thumbnail_iconview));
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW (img->thumbnail_iconview));
	if (selected == NULL)
	{
		g_free( path );
		return;
	}

	/* Avoiding GList memory leak. */
	bak = selected;
	while (selected)
	{
		gtk_tree_model_get_iter(model, &iter,selected->data);
		gtk_tree_model_get(model, &iter,1,&info_slide,-1);
		info_slide->render = address;
		info_slide->transition_id = transition_id;
		g_free( info_slide->path );
		info_slide->path = g_strdup( path );

		/* If this is first slide, we need to copy transition
		 * to the last pseudo-slide too. */
		if( gtk_tree_path_get_indices( selected->data )[0] == 0 )
			img->final_transition.render = address;

		selected = selected->next;
	}
	g_free( path );
	img->project_is_modified = TRUE;
	img_set_total_slideshow_duration(img);
	g_list_foreach (bak, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(bak);
}

static void img_random_button_clicked(GtkButton *button, img_window_struct *img)
{
	GList *selected, *bak;
	GtkTreeIter iter;
	GtkTreeModel *model;
	slide_struct *info_slide;

	model = gtk_icon_view_get_model(GTK_ICON_VIEW (img->thumbnail_iconview));
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW (img->thumbnail_iconview));
	if (selected == NULL)
		return;

	/* Avoiding GList memory leak. */
	bak = selected;
	while (selected)
	{
		gtk_tree_model_get_iter(model, &iter,selected->data);
		gtk_tree_model_get(model, &iter,1,&info_slide,-1);
		info_slide->render = img_set_random_transition(img, info_slide);

		/* If this is first slide, copy transition to last
		 * pseudo-slide */
		if( gtk_tree_path_get_indices( selected->data )[0] == 0 )
			img->final_transition.render = info_slide->render;

		selected = selected->next;
	}
	img->project_is_modified = TRUE;
	g_list_foreach (bak, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(bak);
}

static gpointer img_set_random_transition(img_window_struct *img, slide_struct *info_slide)
{
	gint          nr;
	gint          r1, r2;
	gpointer      address;
	gint          transition_id;
	GtkTreeModel *model;
	GtkTreeIter   iter;
	gchar         path[10];

	/* Get tree store that holds transitions */
	model = gtk_combo_box_get_model( GTK_COMBO_BOX( img->transition_type ) );

	/* Get number of top-levels (categories) and select one */
	nr = gtk_tree_model_iter_n_children( model, NULL );

	/* Fix crash if no modules are loaded */
	if( nr < 2 )
		return( NULL );

	r1 = g_random_int_range( 1, nr );
	g_snprintf( path, sizeof( path ), "%d", r1 );
	gtk_tree_model_get_iter_from_string( model, &iter, path );

	/* Get number of transitions in selected category and select one */
	nr = gtk_tree_model_iter_n_children( model, &iter );
	r2 = g_random_int_range( 0, nr );
	g_snprintf( path, sizeof( path ), "%d:%d", r1, r2 );
	gtk_tree_model_get_iter_from_string( model, &iter, path );

	gtk_tree_model_get( model, &iter, 2, &address, 3, &transition_id, -1 );
	info_slide->transition_id = transition_id;

	/* Prevent leak here */
	if( info_slide->path )
		g_free( info_slide->path );
	info_slide->path = g_strdup( path );

	/* Select proper iter in transition model */
	g_signal_handlers_block_by_func((gpointer)img->transition_type, (gpointer)img_combo_box_transition_type_changed, img);	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(img->transition_type), &iter);
	g_signal_handlers_unblock_by_func((gpointer)img->transition_type, (gpointer)img_combo_box_transition_type_changed, img);	

	return address;
}

static void img_combo_box_speed_changed (GtkComboBox *combo, img_window_struct *img)
{
	gint speed;
	gdouble duration;
	GList *selected, *bak;
	GtkTreeIter iter;
	GtkTreeModel *model;
	slide_struct *info_slide;

	model = gtk_icon_view_get_model(GTK_ICON_VIEW (img->thumbnail_iconview));
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW (img->thumbnail_iconview));
	if (selected == NULL)
		return;

	speed = gtk_combo_box_get_active(combo);

	if (speed == 0)
		duration = FAST;
	else if (speed == 1)
		duration = NORMAL;
	else 
		duration = SLOW;

	/* Avoid memory leak */
	bak = selected;
	while (selected)
	{
		gtk_tree_model_get_iter(model, &iter,selected->data);
		gtk_tree_model_get(model, &iter,1,&info_slide,-1);
		info_slide->speed = duration;

		/* If we're modifying fisr slide, we need to modify
		 * last pseudo-slide too. */
		if( gtk_tree_path_get_indices( selected->data )[0] == 0 )
			img->final_transition.speed = duration;

		selected = selected->next;
	}
	img_set_total_slideshow_duration(img);

	g_list_foreach (bak, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(bak);
}

static void img_spinbutton_value_changed (GtkSpinButton *spinbutton, img_window_struct *img)
{
	gint duration = 0;
	GList *selected, *bak;
	GtkTreeIter iter;
	GtkTreeModel *model;
	slide_struct *info_slide;

	model = gtk_icon_view_get_model(GTK_ICON_VIEW(img->thumbnail_iconview));
	selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(img->thumbnail_iconview));
	if (selected == NULL)
		return;

	duration = gtk_spin_button_get_value_as_int(spinbutton);
	
	bak = selected;
	while (selected)
	{
		gtk_tree_model_get_iter(model, &iter,selected->data);
		gtk_tree_model_get(model, &iter,1,&info_slide,-1);
		info_slide->duration = duration;
		selected = selected->next;
	}
	img_set_total_slideshow_duration(img);

	g_list_foreach (bak, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(bak);
}

static void img_select_all_thumbnails(GtkMenuItem *item, img_window_struct *img)
{
	gtk_icon_view_select_all(GTK_ICON_VIEW (img->thumbnail_iconview));
}

static void img_unselect_all_thumbnails(GtkMenuItem *item, img_window_struct *img)
{
	gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
}

static void img_goto_line_entry_activate(GtkWidget *entry, img_window_struct *img)
{
	gint slide;
	GtkTreePath *path;

	slide = strtol(gtk_entry_get_text(GTK_ENTRY(img->slide_number_entry)), NULL, 10);
	if (slide > 0 && slide <= img->slides_nr)
	{
		gtk_icon_view_unselect_all(GTK_ICON_VIEW (img->thumbnail_iconview));
		path = gtk_tree_path_new_from_indices(slide-1,-1);
		gtk_icon_view_set_cursor (GTK_ICON_VIEW (img->thumbnail_iconview), path, NULL, FALSE);
		gtk_icon_view_select_path (GTK_ICON_VIEW (img->thumbnail_iconview), path);
		gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (img->thumbnail_iconview), path, FALSE, 0, 0);
		gtk_tree_path_free (path);
	}
}

static gint img_sort_none_before_other(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer data)
{
	gchar *name1, *name2;
	gint i;

	gtk_tree_model_get(model, a, 1, &name1, -1);
	gtk_tree_model_get(model, b, 1, &name2, -1);

	if (strcmp(name1,_("None")) == 0)
		i = -1;
	else if (strcmp(name2,_("None")) == 0)
		i = 1;
	else
		i = (g_strcmp0 (name1,name2));

	g_free(name1);
	g_free(name2);
	return i;	
}

static void img_check_numeric_entry (GtkEditable *entry, gchar *text, gint lenght, gint *position, gpointer data)
{
	if(*text < '0' || *text > '9')
		g_signal_stop_emission_by_name( (gpointer)entry, "insert-text" );
}

/*
 * img_create_export_menu:
 * @item: menu item to attach export menu to.
 *
 * This fuction queries all available exporters and adds them to the menu.
 */
static void
img_create_export_menu( GtkWidget         *item,
						img_window_struct *img )
{
	Exporter  *exporters;
	gint       number, i;
	GtkWidget *menu;

	number = img_get_exporters_list( &exporters );

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu( GTK_MENU_ITEM( item ), menu );

	for( i = 0; i < number; i++ )
	{
		GtkWidget *ex;

		ex = gtk_menu_item_new_with_label( exporters[i].description );
		g_signal_connect_swapped( G_OBJECT( ex ), "activate",
								  exporters[i].func, img );
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ), ex );
	}

	img_free_exporters_list( number, exporters );
}

/*
 * img_iconview_selection_button_press:
 *
 * This is a temporary hack that should do the job of unselecting slides if
 * single slide should be selected after select all.
 */
static gboolean
img_iconview_selection_button_press( GtkWidget         *widget,
									 GdkEventButton    *button,
									 img_window_struct *img )
{
	if( ( button->button == 1 ) &&
		! ( button->state & ( GDK_SHIFT_MASK | GDK_CONTROL_MASK ) ) )
		gtk_icon_view_unselect_all( GTK_ICON_VIEW( img->thumbnail_iconview ) );

	return( FALSE );
}

static gboolean
img_scroll_thumb( GtkWidget         *widget,
				  GdkEventScroll    *scroll,
				  img_window_struct *img )
{
	GtkAdjustment *adj;
	gdouble        page, step, upper, value;
	gint           dir = 1;

	if( scroll->direction == GDK_SCROLL_UP ||
		scroll->direction == GDK_SCROLL_LEFT )
	{
		dir = - 1;
	}

	adj = gtk_scrolled_window_get_hadjustment( GTK_SCROLLED_WINDOW( widget ) );

	page  = gtk_adjustment_get_page_size( adj );
	step  = gtk_adjustment_get_step_increment( adj );
	upper = gtk_adjustment_get_upper( adj );
	value = gtk_adjustment_get_value( adj );

	g_print( "%f, %f, %f, %f\n", page, step, upper, value );

	gtk_adjustment_set_value( adj, CLAMP( value + step * dir, 0, upper - page ) );

	return( TRUE );
}

static void img_show_uri(GtkMenuItem *menuitem, img_window_struct *img)
{
	gchar *file = NULL;
	gchar *lang = NULL;
	
	lang = g_strndup(g_getenv("LANG"),2);
	file = g_strconcat("file://",DATADIR,"/doc/",PACKAGE,"/html/",lang,"/index.html",NULL);
	g_free(lang);
	g_print ("%s\n",file);

	if ( !gtk_show_uri(NULL,file, GDK_CURRENT_TIME, NULL))
		g_print ("Error!\n");
	g_free(file);
}
