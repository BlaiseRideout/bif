#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

enum {
  COL_NAME = 0,
  COL_R,
  COL_G,
  COL_B,
  COL_A,
  NUM_COLS
};

static GtkWidget    *window;
static GtkWidget    *parent;
static GtkWidget    *toolbar;
static GtkToolItem  *openbutton;
static GtkToolItem  *newbutton;
static GtkToolItem  *savebutton;
static GtkToolItem  *quitbutton;
static GtkWidget    *colorselect;
static GtkWidget    *namebox;
static GtkWidget    *addbutton;
static GtkWidget    *removebutton;
static GtkWidget    *scrollinglist;
static GtkWidget    *colorlistview;
static GtkListStore *colorlist;
static GtkWidget    *textboxscroll;
static GtkWidget    *textbox;

static void     cleanup(void);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean addpress(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean savepress(GtkWidget *widget, GtkWidget *ev, gpointer data);

static void cleanup(void) {
  gtk_main_quit();
}

static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data) {
  cleanup();
  exit(0);
  return FALSE;
}

static gboolean addpress(GtkWidget *widget, GtkWidget *ev, gpointer data) {
  if(gtk_entry_get_text_length(GTK_ENTRY(namebox)) != 0) {
    unsigned int i;
    gchar *name = (char *)gtk_entry_get_text(GTK_ENTRY(namebox));
    for(i = 0; name[i] != ' ' && name[i] != '\0'; ++i);
    name[i] = '\0';
    GtkTreeIter iter;
    GdkColor color;
    gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colorselect), &color);

    gtk_list_store_append(colorlist, &iter);
    gtk_list_store_set(colorlist, &iter,
                       COL_NAME, name,
                       COL_R, color.red,
                       COL_G, color.green,
                       COL_B, color.blue,
                       COL_A, gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colorselect)), -1);

    gtk_entry_set_text(GTK_ENTRY(namebox), "");
  }
  return FALSE;
}

static gboolean removepress(GtkWidget *widget, GtkWidget *ev, gpointer data) {
  GtkTreeIter iter;
  gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(colorlistview)), NULL, &iter);
  gtk_list_store_remove(colorlist, &iter);
  return FALSE;
}

static gboolean newpress(GtkWidget *widget, GtkWidget *ev, gpointer data) {
  gtk_list_store_clear(colorlist);
  return FALSE;
}

static gboolean openpress(GtkWidget *widget, GtkWidget *ev, gpointer data) {

  return FALSE;
}

static gboolean savepress(GtkWidget *widget, GtkWidget *ev, gpointer data) {
  GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(window),
                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    FILE *file;
    file = fopen(filename, "w");
    
    fclose(file);
  }
  gtk_widget_destroy(dialog);
  return FALSE;
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);

  /* Window Initialization */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request(window, 800, 600);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5); 
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);
  gtk_window_set_title(GTK_WINDOW(window), "Image creator");
  g_signal_connect(window, "destroy-event", G_CALLBACK(killevent), NULL);
  g_signal_connect(window, "delete-event", G_CALLBACK(killevent), NULL);
  parent = gtk_vbox_new(FALSE, 5);

  /* Toolbar initialization */
  toolbar = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
  gtk_container_set_border_width(GTK_CONTAINER(toolbar), 5);
  newbutton = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
  openbutton = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
  savebutton = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
  quitbutton = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
  GtkToolItem *sep = gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), newbutton, -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), openbutton, -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), savebutton, -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), quitbutton, -1);
  gtk_box_pack_start(GTK_BOX(parent), toolbar, FALSE, TRUE, 5);
  g_signal_connect(newbutton, "clicked", G_CALLBACK(newpress), NULL);
  g_signal_connect(openbutton, "clicked", G_CALLBACK(openpress), NULL);
  g_signal_connect(savebutton, "clicked", G_CALLBACK(savepress), NULL);
  g_signal_connect(quitbutton, "clicked", G_CALLBACK(killevent), NULL);

  /* Color selector initialization */
  colorselect = gtk_color_selection_new();
  gtk_box_pack_start(GTK_BOX(parent), colorselect, FALSE, TRUE, 5);

  /* Name textbo initialization */
  namebox = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(parent), namebox, FALSE, TRUE, 5);

  /* Add button initialization */
  addbutton = gtk_button_new_with_label("Add");
  gtk_box_pack_start(GTK_BOX(parent), addbutton, FALSE, TRUE, 5);
  g_signal_connect(addbutton, "button-press-event", G_CALLBACK(addpress), NULL);

  /* Remove button initialization */
  removebutton = gtk_button_new_with_label("Remove");
  gtk_box_pack_start(GTK_BOX(parent), removebutton, FALSE, TRUE, 5);
  g_signal_connect(removebutton, "button-press-event", G_CALLBACK(removepress), NULL);

  /* Color list initialization */
  scrollinglist = gtk_scrolled_window_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0)), GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 9000001, 5, 50, 50)));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  colorlist = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_UCHAR, G_TYPE_UCHAR, G_TYPE_UCHAR, G_TYPE_UCHAR);
  colorlistview = gtk_tree_view_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(colorlistview), -1, "Name", renderer, "text", COL_NAME, NULL);
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(colorlistview), -1, "R", renderer, "text", COL_R, NULL);
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(colorlistview), -1, "G", renderer, "text", COL_G, NULL);
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(colorlistview), -1, "B", renderer, "text", COL_B, NULL);
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(colorlistview), -1, "A", renderer, "text", COL_A, NULL);
  gtk_tree_view_set_model(GTK_TREE_VIEW(colorlistview), GTK_TREE_MODEL(colorlist));
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollinglist), colorlistview);
  gtk_box_pack_start(GTK_BOX(parent), scrollinglist, TRUE, TRUE, 5);

  /* Content textbox initialization */
  textboxscroll = gtk_scrolled_window_new(GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0)), GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 9000001, 5, 50, 50)));
  textbox = gtk_text_view_new();
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(textboxscroll), textbox);
  gtk_box_pack_start(GTK_BOX(parent), textboxscroll, TRUE, TRUE, 5);

  /* Adding everything to the window */
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(parent));
  gtk_widget_show(GTK_WIDGET(parent));
  gtk_widget_show_all(window);

  gtk_main();

  cleanup();

  return 0;
}
