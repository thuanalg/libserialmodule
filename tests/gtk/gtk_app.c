#include <gtk/gtk.h>
#include <serialmodule.h>
#include <stdio.h>
#include <simplelog.h>

int is_master = 0;
int baudrate = 0;
char is_port[32];
char cfgpath[1024];
int myusleep = 0;

#define __ISMASTER__				"--is_master="
#define __ISPORT__					"--is_port="
#define __ISCFG__					"--is_cfg="
#define __ISBAUDRATE__				"--is_baudrate="
#define __IS_USLEEP__				"--is_usleep="
char *test_spsr_list_ports[100];
int TEST_CALLBACK_OBJ = 179;
int spsr_test_callback(void *data) ;
static int spsr_call_back_read(void *data);
static gboolean update_ui(gpointer data);
GtkWidget *entries[7];
void *arr_obj[10];

void on_button_clicked_00(GtkWidget *widget, gpointer data) {
    int i = 0;
    int ret = 0;
    SP_SERIAL_INPUT_ST *obj = 0;
    const char *portname = gtk_entry_get_text(GTK_ENTRY(entries[0])); 
    spllog(0, "Button %s clicked, text: %s!\n", (char *)data, portname);
    spsr_malloc(sizeof(SP_SERIAL_INPUT_ST), obj, SP_SERIAL_INPUT_ST);
    if(!obj) {
        exit(1);
    }
    for(i = 0; i < 10; ++i) {
        if(!arr_obj[i]) {
            arr_obj[i] = (void*) obj;
            break;
        }
    }
	snprintf(obj->port_name, SPSR_PORT_LEN, "%s", portname);
	/*obj.baudrate = 115200;*/
	obj->baudrate = baudrate; 
    //obj->cb_evt_fn = spsr_call_back_read;
    obj->cb_evt_fn = spsr_test_callback;
    obj->cb_obj = &TEST_CALLBACK_OBJ;
    obj->t_delay = 55;
    
    spllog(0, "baudrate:=========================++++++++++++> %d, portname: %s", 
        obj->baudrate, obj->port_name);
    ret = spsr_inst_open(obj);
    if(ret) {
        spllog(0, "spsr_inst_create:=========================> %d", ret);
    }
    spsr_free(obj);   
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
    //ret = spsr_inst_del((char*)portname);
    n = strlen(datawrtite);
    if(myusleep > 0) {
        for(i = 0; i < n; ++i) 
        {
            ret = spsr_inst_write(portname, datawrtite + i, 1);
            usleep(myusleep);
        }
    } 
    else {
        ret = spsr_inst_write(portname, datawrtite, n);
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
        if(ret) {
            exit(1);
        }
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
gboolean update_ui(void* data) {
    //gtk_label_set_text(GTK_LABEL(label), "Updated from worker thread!");
    //Run in main thread
    SP_SERIAL_GENERIC_ST* evt = 0;
    evt = (SP_SERIAL_GENERIC_ST *) data;
    spllog(SPL_LOG_DEBUG, "evt->data: %s", evt->data + evt->pc);
    spl_free(evt);
    return FALSE;  
}

int spsr_call_back_read(void *data) {
    //Access main thread
    int n = 0;
    SP_SERIAL_GENERIC_ST* evt = (SP_SERIAL_GENERIC_ST *)data;
    n = evt->total;
    spllog(SPL_LOG_DEBUG, "evt->total: %d, evt->pc: %d, evt->pl: %d, data: %s", 
        evt->total,  evt->pc, evt->pl, evt->data + evt->pc);
    spsr_malloc(n, evt, SP_SERIAL_GENERIC_ST);
    memcpy(evt, data, n);
    g_idle_add(update_ui, (void*)evt);
    return 0;
}

int spsr_test_callback(void *data) {
    /* Data is borrowed from background thread, you should make a copy to use and delete. */
    /* Here, please note data type. */
    if (!data) {
        spllog(SPL_LOG_DEBUG, "Data is empty");
        return 0;
    }
    void* obj = 0;
    int total = 0;
    char* realdata = 0;
    int datalen = 0;
    /* Data is borrowed from background thread, you should make a copy to use and delete. */
    SP_SERIAL_GENERIC_ST* evt = (SP_SERIAL_GENERIC_ST*)data;
    /*char *realdata: from evt->pc to evt->pl.*/
    /* SPSR_MODULE_EVENT evt->type */
    total = evt->total;
    spsr_malloc(total, evt, SP_SERIAL_GENERIC_ST); /* Clone evt memory. */
    if (!evt) {
        exit(1);
    }
    memcpy(evt, data, total);
    spllog(SPL_LOG_DEBUG, "total: %d, type: %d", evt->total, evt->type);
    if (sizeof(void*) == sizeof(unsigned int)) {
        /* 32 bit */
        unsigned int* temp = (unsigned int*)evt->data;
        obj = (void*)(*temp);
    } 
    else if (sizeof(void*) == sizeof(unsigned long long int)) {
        /* 64 bit */
        unsigned long long int* temp = (unsigned long long int*)evt->data;
        obj = (void*)(*temp);
    }
    spllog(0, "obj: 0x%p, value: %d", obj, *((int*)obj));
    do {
        datalen = evt->pl - evt->pc; /*datalen.*/
        realdata = evt->data + evt->pc; /*char *realdata: from evt->pc to evt->pl.*/
        if (evt->type == SPSR_EVENT_READ_BUF) {
            /* Read data.*/
            spllog(0, "SPSR_EVENT_READ_BUF, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSR_EVENT_WRITE_OK) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_WRITE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSR_EVENT_WRITE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_WRITE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSR_EVENT_OPEN_DEVICE_OK) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_OPEN_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }
        if (evt->type == SPSR_EVENT_OPEN_DEVICE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_OPEN_DEVICE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }         
        if (evt->type == SPSR_EVENT_CLOSE_DEVICE_OK) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_CLOSE_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }      
        if (evt->type == SPSR_EVENT_CLOSE_DEVICE_ERROR) {
            /* Port name .*/
            spllog(0, "SPSR_EVENT_CLOSE_DEVICE_ERROR, realdata: %s, datalen: %d", realdata, datalen);
            break;
        }               
    } while (0);
    spsr_free(evt);
    return 0;
}
