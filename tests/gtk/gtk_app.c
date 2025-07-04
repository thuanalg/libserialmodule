#include <gtk/gtk.h>
#include <serialmodule.h>
#include <stdio.h>
#include <simplelog.h>
#include <regex.h>

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

GtkWidget *gbtextview = 0;
char *test_spsr_list_ports[100];
int TEST_CALLBACK_OBJ = 179;
int spsr_test_callback(void *data) ;
static int spsr_call_back_read(void *data);
static gboolean update_ui(gpointer data);
GtkWidget *entries[7];
void *arr_obj[10];
char* remove_ansi_escape(const char *input);

void on_button_clicked_00(GtkWidget *widget, gpointer data) {
    int i = 0;
    int ret = 0;
    SPSR_INPUT_ST *obj = 0;
    const char *portname = gtk_entry_get_text(GTK_ENTRY(entries[0])); 
    spllog(0, "Button %s clicked, text: %s!\n", (char *)data, portname);
    spsr_malloc(sizeof(SPSR_INPUT_ST), obj, SPSR_INPUT_ST);
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
    obj->cb_obj = gbtextview;
    obj->t_delay = 100;
    obj->offDSR = 1;
    
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
    char commandd[1024] = {0};
    char *datawrtite = (char*)gtk_entry_get_text(GTK_ENTRY(entries[3])); 
    char *portname = (char*)gtk_entry_get_text(GTK_ENTRY(entries[2])); 
    spllog(0, "Button %s clicked, portname: %s, data: %s!\n", (char *)data, portname, datawrtite);
    //ret = spsr_inst_del((char*)portname);
    snprintf(commandd, 1024, "%s\n", datawrtite);
    n = strlen(commandd);
    spllog(2, "commd: %s", commandd);
    if(myusleep > 0) {
        for(i = 0; i < n; ++i) 
        {
            //snprintf(commandd, 1024, "%s\n", datawrtite);
            ret = spsr_inst_write(portname, commandd + i, 1);
            usleep(myusleep);
        }
    } 
    else {
        //snprintf(commandd, 1024, "%s\n", datawrtite);
        ret = spsr_inst_write(portname, commandd, n);
    }

    spllog(0, "ret %d!\n", ret);
}
int main(int argc, char *argv[]) {
    int ret = 0;
    int i = 0;
    int k = 0;
#ifndef UNIX_LINUX
	snprintf(cfgpath, 1024, "simplelog.cfg");
#else
	snprintf(cfgpath, 1024, "simplelog.cfg");
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
        //gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
        gtk_window_maximize(GTK_WINDOW(window)); // Title bar and buttons stay visible
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
        GtkWidget *grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(window), grid);
        
        GtkWidget *buttons[7];
        
        char labels[7][100] = {"ADD COM port", "REM COM port", "WRITE COM port", "written data", "read data", "6", "7"};
        
        for (i = 0; i < 5; i++) {
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
            if (i < 4) {
                entries[i] = gtk_entry_new();

                gtk_grid_attach(GTK_GRID(grid), buttons[i], 0, i, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
                if(i < 4) {
                    gtk_entry_set_text(GTK_ENTRY(entries[i]), "/dev/ttyUSB0");
                } else {
                    gtk_widget_set_size_request(entries[i], 1600, 300); // Width: 300px, Height: 30px
                }
            } 
            else if ( i == 4) {
#if 1                
                GtkWidget *scrolled_window;
                GtkWidget *textview = 0;
                textview = gtk_text_view_new();
                gbtextview = textview;
                
                scrolled_window = gtk_scrolled_window_new(NULL, NULL);
                entries[i] = scrolled_window;
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                               GTK_POLICY_AUTOMATIC,  // horizontal
                               GTK_POLICY_AUTOMATIC); // vertical
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(gbtextview), GTK_WRAP_NONE);
                gtk_container_add(GTK_CONTAINER(scrolled_window), gbtextview);

                gtk_grid_attach(GTK_GRID(grid), buttons[i], 0, i, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);    
                gtk_widget_set_size_request(entries[i], 1600, 300); // Width: 300px, Height: 30px            
#else
                entries[i] = gtk_entry_new();
                gtk_grid_attach(GTK_GRID(grid), buttons[i], 0, i, 1, 1);
                gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
                if(i < 4) {
                    gtk_entry_set_text(GTK_ENTRY(entries[i]), "/dev/ttyUSB0");
                } else {
                    gtk_widget_set_size_request(entries[i], 1600, 300); // Width: 300px, Height: 30px
                }  
#endif              
            }
        }

        gtk_widget_show_all(window);
        gtk_main();

        
        spsr_module_finish();
    } while(0);
    spl_finish_log();
    return 0;
}
#define GTK_TEST_BUF        1024
gboolean update_ui(void* data) {
    //gtk_label_set_text(GTK_LABEL(label), "Updated from worker thread!");
    //Run in main thread
    SPSR_GENERIC_ST* evt = 0;
    void *obj = 0;
    evt = (SPSR_GENERIC_ST *) data;
    char *realdata = 0;
    int datalen = 0;
    char text[GTK_TEST_BUF];
    char text_total[GTK_TEST_BUF * 100];
    const char *cur_text = 0;
    do {
        spllog(SPL_LOG_INFO, "evt->data: %s", evt->data + evt->pc);
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
        if(!obj) {
            break;
        } 
        datalen = evt->pl - evt->pc; /*datalen.*/
        realdata = evt->data + evt->pc; /*char *realdata: from evt->pc to evt->pl.*/   
        text[0] = 0;
        do {
            if (evt->type == SPSR_EVENT_READ_BUF) {
                /* Read data.*/
            #if 1
                snprintf(text, GTK_TEST_BUF, "\t\t%s", realdata);            
            #else
                snprintf(text, GTK_TEST_BUF,
                    "SPSR_EVENT_READ_BUF, datalen: %d, realdata: \n\n------>>>\n\n%s\n\n<<<<------------\n\n", 
                    datalen, realdata);
            #endif
                break;
            }
            
            if (evt->type == SPSR_EVENT_WRITE_OK) {
                /* Port name .*/
            #if 0
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_WRITE_OK, datalen: %d", datalen);
            #else
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_WRITE_OK, realdata: %s, datalen: %d", realdata, datalen);
            #endif
                break;
            }
            if (evt->type == SPSR_EVENT_WRITE_ERROR) {
                /* Port name .*/
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_WRITE_ERROR|%s, realdata: %s, datalen: %d", 
                    spsr_err_txt(evt->err_code),
                    realdata, datalen);
                break;
            }

            if (evt->type == SPSR_EVENT_OPEN_DEVICE_OK) {
                /* Port name .*/
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_OPEN_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
                //g_idle_add(update_ui, (GtkWidget *)obj);
                break;
            }
            if (evt->type == SPSR_EVENT_OPEN_DEVICE_ERROR) {
                /* Port name .*/
                snprintf(text, GTK_TEST_BUF, 
                    "SPSR_EVENT_OPEN_DEVICE_ERROR|%s, realdata: %s, datalen: %d", 
                    spsr_err_txt(evt->err_code),
                    realdata, datalen);
                break;
            }         
            if (evt->type == SPSR_EVENT_CLOSE_DEVICE_OK) {
                /* Port name .*/
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_CLOSE_DEVICE_OK, realdata: %s, datalen: %d", realdata, datalen);
                break;
            }      
            if (evt->type == SPSR_EVENT_CLOSE_DEVICE_ERROR) {
                /* Port name .*/
                snprintf(text, GTK_TEST_BUF, "SPSR_EVENT_CLOSE_DEVICE_ERROR|%s, realdata: %s, datalen: %d", 
                    spsr_err_txt(evt->err_code),
                    realdata, datalen);
                break;
            }                   
        }   while(0);  
#if 1 
        GtkTextIter start, end;
        gchar *gggtext;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(obj));
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        // Extract the text (returns a newly allocated string)
        gggtext = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        char *rpltext = remove_ansi_escape(text);
        snprintf(text_total, GTK_TEST_BUF * 100, "%s\n", rpltext);
        //gtk_text_buffer_set_text(buffer, text_total, -1);   
        gtk_text_buffer_insert(buffer, &end, text_total, -1); // Append the text at the end

        // Free the memory when done
        g_free(gggtext);
        free(rpltext);
#else       
        cur_text = gtk_entry_get_text(GTK_ENTRY(obj));
        snprintf(text_total, GTK_TEST_BUF * 4, "%s\n%s", text, cur_text);
        gtk_entry_set_text( GTK_ENTRY(obj), text_total);
#endif
    } while(0);

    spsr_free(evt);
    return FALSE;  
}

int spsr_call_back_read(void *data) {
    //Access main thread
    int n = 0;
    SPSR_GENERIC_ST* evt = (SPSR_GENERIC_ST *)data;
    n = evt->total;
    spllog(SPL_LOG_DEBUG, "evt->total: %d, evt->pc: %d, evt->pl: %d, data: %s", 
        evt->total,  evt->pc, evt->pl, evt->data + evt->pc);
    spsr_malloc(n, evt, SPSR_GENERIC_ST);
    memcpy(evt, data, n);
    g_idle_add(update_ui, (void*)evt);
    //spsr_free(evt);
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
    SPSR_GENERIC_ST* evt = (SPSR_GENERIC_ST*)data;
    /*char *realdata: from evt->pc to evt->pl.*/
    /* SPSR_MODULE_EVENT evt->type */
    total = evt->total;
    spsr_malloc(total, evt, SPSR_GENERIC_ST); /* Clone evt memory. */
    if (!evt) {
        exit(1);
    }
    memcpy(evt, data, total);
    spllog(SPL_LOG_DEBUG, "total: %d, type: %d", evt->total, evt->type);
    g_idle_add(update_ui, evt);


    return 0;
}

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* remove_ansi_escape(const char *input) {
    regex_t regex;
    regmatch_t match;
    const char *pattern = "\x1B\\[[0-9;]*[A-Za-z]";

    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return NULL;
    }

    char *output = malloc(strlen(input) + 1);
    if (!output) return NULL;
    output[0] = '\0';

    const char *cursor = input;
    while (*cursor) {
        if (regexec(&regex, cursor, 1, &match, 0) == 0) {
            strncat(output, cursor, match.rm_so);
            cursor += match.rm_eo;
        } else {
            strcat(output, cursor);
            break;
        }
    }

    regfree(&regex);
    return output;
}
