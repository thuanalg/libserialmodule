#include <gtk/gtk.h>
#include <serialmodule.h>
#include <stdio.h>
#include <simplelog.h>

int is_master = 0;
int baudrate = 0;
char is_port[32];
char cfgpath[1024];
int myusleep = 1000;

#define __ISMASTER__				"--is_master="
#define __ISPORT__					"--is_port="
#define __ISCFG__					"--is_cfg="
#define __ISBAUDRATE__				"--is_baudrate="
#define __IS_USLEEP__				"--is_usleep="

GtkWidget *entries[7];
void *arr_obj[10];

void on_button_clicked_00(GtkWidget *widget, gpointer data) {
    int i = 0;
    int ret = 0;
    SP_SERIAL_INPUT_ST *obj = 0;
    SP_SERIAL_INFO_ST *obj1 = 0;
    const char *portname = gtk_entry_get_text(GTK_ENTRY(entries[0])); 
    spllog(0, "Button %s clicked, text: %s!\n", (char *)data, portname);
    spserial_malloc(sizeof(SP_SERIAL_INPUT_ST), obj, SP_SERIAL_INPUT_ST);
    if(!obj) {
        exit(1);
    }
    for(i = 0; i < 10; ++i) {
        if(!arr_obj[i]) {
            arr_obj[i] = (void*) obj;
            break;
        }
    }
	snprintf(obj->port_name, SPSERIAL_PORT_LEN, "%s", portname);
	/*obj.baudrate = 115200;*/
	obj->baudrate = baudrate; 
    spllog(0, "baudrate:=========================++++++++++++> %d, portname: %s", 
        obj->baudrate, obj->port_name);
    ret = spsr_inst_open(obj);
    if(ret) {
        spllog(0, "spserial_inst_create:=========================> %d", ret);
    }
    spserial_free(obj);   
}
void on_button_clicked_01(GtkWidget *widget, gpointer data) {
    int ret = 0;
    const char *portname = gtk_entry_get_text(GTK_ENTRY(entries[1])); 
    spllog(0, "Button %s clicked!\n", (char *)data);
    ret = spsr_inst_close((char*)portname);
}
void on_button_clicked_02(GtkWidget *widget, gpointer data) {
    int ret = 0;
    int i , n;
    char *datawrtite = (char*)gtk_entry_get_text(GTK_ENTRY(entries[3])); 
    char *portname = (char*)gtk_entry_get_text(GTK_ENTRY(entries[2])); 
    spllog(0, "Button %s clicked, portname: %s, data: %s!\n", (char *)data, portname, datawrtite);
    //ret = spserial_inst_del((char*)portname);
    n = strlen(datawrtite);
    for(i = 0; i < n; ++i) {
        ret = spsr_inst_write(portname, datawrtite + i, 1);
        usleep(myusleep);
    }
    spllog(0, "ret %d!\n", ret);
}
int main(int argc, char *argv[]) {
    int ret = 0;
    int i = 0;
    int k = 0;
#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "C:/z/serialmodule/win32/Debug/simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "/home/thuannt/x/serialmodule/src/linux/simplelog.cfg");
#endif
	snprintf(is_port, 32,"%s", "COM2");
	baudrate = 9600;
	for (i = 0; i < argc; ++i) {
		if (strstr(argv[i], __ISMASTER__)) {
			 k = sscanf(argv[i], __ISMASTER__"%d", &is_master);
			 spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISPORT__)) {
			k = sscanf(argv[i], __ISPORT__"%s", is_port);
			spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISCFG__)) {
			k = sscanf(argv[i], __ISCFG__"%s", cfgpath);
			spl_console_log("k = %d.", k);
			continue;
		}
		if (strstr(argv[i], __ISBAUDRATE__)) {
			k = sscanf(argv[i], __ISBAUDRATE__"%d", &baudrate);
			spl_console_log("k = %d, baudrate: %d.", k, baudrate);
			continue;
		}
		if (strstr(argv[i], __IS_USLEEP__)) {
			k = sscanf(argv[i], __IS_USLEEP__"%d", &myusleep);
			spl_console_log("k = %d, myusleep: %d.", k, myusleep);
			continue;
		}        
	}    
    ret = ret = spl_init_log(cfgpath);
    if(ret) {
        exit(1);
    }
    do {
        ret = spsr_module_init();
        gtk_init(&argc, &argv);
        
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "GTK 7 Buttons & 7 Textboxes");
        gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
        GtkWidget *grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(window), grid);
        
        GtkWidget *buttons[7];
        
        char labels[7][100] = {"ADD COM port", "REM COM port", "WRITE COM port", "write data", "5", "6", "7"};
        
        for (i = 0; i < 7; i++) {
            buttons[i] = gtk_button_new_with_label(labels[i]);
            if(i == 0) {
                g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_00), labels[i]);
            }
            else if(i == 1) {
                g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_01), labels[i]);
            }
            else if(i == 2) {
                g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_02), labels[i]);
            }
            else if(i == 3) {
                //g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_01), labels[i]);
            }
            else if(i == 4) {
                //g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_01), labels[i]);
            }
            else if(i == 5) {
                //g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_01), labels[i]);
            }
            else if(i == 6) {
                //g_signal_connect(buttons[i], "clicked", G_CALLBACK(on_button_clicked_01), labels[i]);
            }       

            entries[i] = gtk_entry_new();

            gtk_grid_attach(GTK_GRID(grid), buttons[i], 0, i, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
            gtk_entry_set_text(GTK_ENTRY(entries[i]), "/dev/ttyUSB0");
        }

        gtk_widget_show_all(window);
        gtk_main();

        
        spsr_module_finish();
    } while(0);
    spl_finish_log();
    return 0;
}

