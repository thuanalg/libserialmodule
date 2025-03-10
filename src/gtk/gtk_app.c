#include <gtk/gtk.h>

void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Button %s clicked!\n", (char *)data);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK 7 Buttons & 7 Textboxes");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    
    GtkWidget *buttons[7];
    GtkWidget *entries[7];
    char labels[7][10] = {"1", "2", "3", "4", "5", "6", "7"};
    
    for (int i = 0; i < 7; i++) {
        buttons[i] = gtk_button_new_with_label(labels[i]);
        g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked), labels[i]);
        
        entries[i] = gtk_entry_new();
        
        gtk_grid_attach(GTK_GRID(grid), buttons[i], 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
    }
    
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}

