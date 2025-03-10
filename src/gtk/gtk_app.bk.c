
#include <gtk/gtk.h>

// Hàm xử lý sự kiện khi nhấn nút
static void on_button_clicked(GtkWidget *widget, gpointer entry) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry)); // Lấy nội dung textbox
    g_print("Bạn đã nhập: %s\n", text); // In ra terminal
    gtk_entry_set_text(GTK_ENTRY(entry), ""); // Xóa nội dung textbox sau khi nhấn nút
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Tạo cửa sổ chính
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Ứng dụng GTK");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Tạo layout chính (hộp dọc)
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Tạo textbox (entry)
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 5);

    // Tạo nút bấm
    GtkWidget *button = gtk_button_new_with_label("Nhấn Tôi!");
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5);

    // Gắn sự kiện click vào button
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), entry);

    // Hiển thị toàn bộ giao diện
    gtk_widget_show_all(window);

    gtk_main(); // Chạy vòng lặp GTK
    return 0;
}

