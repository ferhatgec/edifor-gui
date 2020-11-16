/* MIT License
# Forked from https://github.com/ferhatgec/gemini
# Copyright (c) 2020 Ferhat Geçdoğan All Rights Reserved.
# Distributed under the terms of the MIT License.
#
# */

#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <vte/vte.h> /* LibVTE */
#include <gtk/gtk.h> /* GTK */
#include <locale.h> /* For Setlocale */

#include <EdiforGUI.h>

#ifdef GDK_WINDOWING_X11
	#include <gdk/gdkx.h>
	#include <X11/Xlib.h>
#endif

/* Color functions */
#define CLR_R(x)   (((x) & 0xff0000) >> 16)
#define CLR_G(x)   (((x) & 0x00ff00) >>  8)
#define CLR_B(x)   (((x) & 0x0000ff) >>  0)
#define CLR_16(x)  ((double)(x) / 0xff)
#define CLR_GDK(x, a) (const GdkRGBA) { .red = CLR_16(CLR_R(x)), \
                                    .green = CLR_16(CLR_G(x)), \
                                    .blue = CLR_16(CLR_B(x)), \
                                    .alpha = a }
/*
	TODO: Add transparent option, customization with given value.
*/

GtkWidget *window, 
	  *terminal, 
	  *header, 
	  *button, 
	  *image,
	  *entry,
	  *new_window,
	  *new_window_label; /* Window, Headerbar && Terminal widget */

GdkPixbuf *icon; /* Icon */

static gchar *input;
static gchar **command, **envp;

static PangoFontDescription *fontDesc; /* Description for the terminal font */
static int currentFontSize;


void changefile(GtkWidget *widget);
void new_window_with_file(GtkWidget *widget);

void edifor_open_dialog(GtkWidget *widget);

bool is_exist(char *filename) {
	struct stat   buffer;   
  	return (stat (filename, &buffer) == 0);
}

GdkPixbuf *create_pixbuf(const gchar * filename) {
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   
   if (!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void edifor_gui_Callback(VteTerminal *term, GPid pid,
	GError *error, gpointer user_data) {
	if(error == NULL) {
		/* Logging */
		g_print("EdiforGUI started. PID: %d", pid); 
	} else {
		g_print(error->message);
		g_clear_error(&error);
	} 
	
	printf("\n"); /* Newline */
}

/* EdiforGUI set font. */
void edifor_gui_set_Term_Font(int fontSize) {
    gchar *fontStr = g_strconcat(EDIFOR_GUI_FONT, " ", g_strdup_printf("%d", fontSize), NULL);
    
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL) {
    	vte_terminal_set_font(VTE_TERMINAL(terminal), fontDesc);
	currentFontSize = fontSize;
	pango_font_description_free(fontDesc);
	g_free(fontStr);	  
    }
}

gboolean edifor_gui_on_title_Changed(GtkWidget *terminal, gpointer user_data) {
    GtkWindow *window = user_data;

    gtk_window_set_title(window,
	vte_terminal_get_window_title(VTE_TERMINAL(terminal))?:"Edifor"); 

    return TRUE;
}

/*
	TODO: Add .config/edifor/configuration and read here.
*/
void edifor_gui_configuration() {
    /* Set numeric locale to en_US.UTF-8 */
    setlocale(LC_NUMERIC, "en_US.UTF-8");

    /* Hide the mouse cursor when typing */
    vte_terminal_set_mouse_autohide(VTE_TERMINAL(terminal), FALSE);

    /* Scroll issues */
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), -1);
    vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), FALSE);
    vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), TRUE);
    	
    /* Disable audible bell */
    vte_terminal_set_audible_bell(VTE_TERMINAL(terminal), FALSE);

    /* Allow hyperlinks */
    vte_terminal_set_allow_hyperlink(VTE_TERMINAL(terminal), TRUE);

    /* Disable Blick mode */
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal), 
        VTE_CURSOR_BLINK_OFF);
	
    /* Set the terminal colors and font */
    vte_terminal_set_colors(VTE_TERMINAL(terminal),
        &CLR_GDK(0xc0d6e4, 1),          /* Foreground */
        &CLR_GDK(0x171421, 1), /* Background (RGBA) */
        (const GdkRGBA[]){           /* Palette */
            CLR_GDK(0x3f3b33, 0),
            CLR_GDK(0xC01C28, 0),
            CLR_GDK(0x26A269, 0),
            CLR_GDK(0xA2734C, 0),
            CLR_GDK(0x12488B, 0),
            CLR_GDK(0xA347BA, 0),
            CLR_GDK(0x2AA1B3, 0),
            CLR_GDK(0xD0CFCC, 0),
            CLR_GDK(0x5E5C64, 0),
            CLR_GDK(0xF66151, 0),
            CLR_GDK(0x33D17A, 0),
            CLR_GDK(0xE9AD0C, 0),
            CLR_GDK(0x2A7BDE, 0),
            CLR_GDK(0xC061CB, 0),
            CLR_GDK(0x33C7DE, 0),
            CLR_GDK(0xFFFFFF, 0)
        }, 16);
	
	
    edifor_gui_set_Term_Font(EDIFOR_GUI_FONT_SIZE);  
    
    gtk_widget_set_visual(window, 
        gdk_screen_get_rgba_visual(gtk_widget_get_screen(window)));
    
}

void edifor_gui_connect_signals() {
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
 
    /* For Shortcuts */
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(edifor_gui_on_keypress), 
    	GTK_WINDOW(window));
    
    /* Terminal window title */
    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(edifor_gui_on_title_Changed), 
                        GTK_WINDOW(window));
}

void edifor_gui_new_buffer_start(char* arg) {
    //terminal = vte_terminal_new();
    
    if(is_exist(arg) == true) {
    	command = (gchar *[]){"/bin/edifor", arg, NULL}; /* Get SHELL environment. */	
    	
    	/* Spawn asynchronous terminal */
		vte_terminal_spawn_async(VTE_TERMINAL(terminal), 
        		VTE_PTY_DEFAULT, /* VTE_PTY flag */
        		NULL,		 /* Working Dir */
        		command, 	 /* Argv */
        		NULL, 		 /* Environment value */
        		G_SPAWN_DEFAULT, /* Spawn flag */
        		NULL,		 /* Child setup function */
        		NULL,		 /* Child setup data */
        		NULL,		 /* Child setup data destroy */
        		-1,		 /* Timeout */
        		NULL,		 /* Cancellable */
        		edifor_gui_Callback, /* Async Callback */
        		NULL);		 /* Callback data */
    
    	

    	/* Connect signals */
    	edifor_gui_connect_signals();
    
    	/* Edifor configuration */
    	edifor_gui_configuration();
    }
}

void edifor_gui_start(char* arg) {
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    header = gtk_header_bar_new();
	entry = gtk_entry_new();
	
    gtk_window_set_title(GTK_WINDOW(window), "Fegeya Edifor");
    gtk_window_set_default_size(GTK_WINDOW(window), 805, 460);
    gtk_window_set_resizable (GTK_WINDOW(window), TRUE);
  	
  	
  	gtk_entry_set_width_chars(GTK_ENTRY(entry), 50);
  	
    /* TODO:
    	Add Edifor's logo.
    */  
    icon = create_pixbuf("/usr/share/pixmaps/edifor/edifor_32.png"); /* Edifor icon. */
    image = gtk_image_new_from_file("/usr/share/pixmaps/edifor/edifor_32.png");
    button = gtk_tool_button_new(image, NULL);
    new_window_label = gtk_label_new("➕");
    
    new_window = gtk_tool_button_new(new_window_label, NULL);
    
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);    
    gtk_window_set_icon(GTK_WINDOW(window), icon);	

	g_signal_connect(GTK_TOOL_BUTTON(button), "clicked", G_CALLBACK(edifor_open_dialog), "button");
	g_signal_connect(GTK_ENTRY(entry), "activate", G_CALLBACK(changefile), "button");
	g_signal_connect(GTK_TOOL_BUTTON(new_window), "clicked", G_CALLBACK(new_window_with_file), "button");
	
    //gtk_button_set_image(GTK_BUTTON (button), image);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), entry);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), new_window);
    
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    
    /* Start a new shell */
    envp = g_get_environ();
    
   	if(arg == NULL) {
		command = (gchar *[]){"/bin/edifor", NULL }; /* Get SHELL environment. */	
    	
    	/* Spawn asynchronous terminal */
		vte_terminal_spawn_async(VTE_TERMINAL(terminal), 
        		VTE_PTY_DEFAULT, /* VTE_PTY flag */
        		NULL,		 /* Working Dir */
        		command, 	 /* Argv */
        		NULL, 		 /* Environment value */
        		G_SPAWN_DEFAULT, /* Spawn flag */
        		NULL,		 /* Child setup function */
        		NULL,		 /* Child setup data */
        		NULL,		 /* Child setup data destroy */
        		-1,		 /* Timeout */
        		NULL,		 /* Cancellable */
        		edifor_gui_Callback, /* Async Callback */
        		NULL);		 /* Callback data */
    } else {
    	command = (gchar *[]){"/bin/edifor", arg, NULL}; /* Get SHELL environment. */	
    	
    	/* Spawn asynchronous terminal */
		vte_terminal_spawn_async(VTE_TERMINAL(terminal), 
        		VTE_PTY_DEFAULT, /* VTE_PTY flag */
        		NULL,		 /* Working Dir */
        		command, 	 /* Argv */
        		NULL, 		 /* Environment value */
        		G_SPAWN_DEFAULT, /* Spawn flag */
        		NULL,		 /* Child setup function */
        		NULL,		 /* Child setup data */
        		NULL,		 /* Child setup data destroy */
        		-1,		 /* Timeout */
        		NULL,		 /* Cancellable */
        		edifor_gui_Callback, /* Async Callback */
        		NULL);		 /* Callback data */
    }
    
    g_strfreev(envp);

    /* Connect signals */
    edifor_gui_connect_signals();
    
    /* Edifor configuration */
    edifor_gui_configuration();

    /* Put widgets together and run the main loop */
    gtk_container_add(GTK_CONTAINER(window), terminal);
    gtk_widget_show_all(window);
    g_object_unref(icon);
    gtk_main();
}

void changefile(GtkWidget *widget) {
	gchar* _text = gtk_entry_get_text(entry);
	
	edifor_gui_new_buffer_start(_text);
}

void new_window_with_file(GtkWidget *widget) {
	gchar* _text = gtk_entry_get_text(entry);
	
	
	printf("%s\n", _text);
	
	edifor_gui_start(_text);
}


void edifor_open_dialog(GtkWidget *widget) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new (("Open"),
	                                                 GTK_WINDOW(window),
	                                                 GTK_FILE_CHOOSER_ACTION_OPEN,
	                                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	                                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                                 NULL);
									 
	switch(gtk_dialog_run(GTK_DIALOG(dialog))) {
		case GTK_RESPONSE_ACCEPT: {
			gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			
			edifor_gui_new_buffer_start(filename);
			
			break;
		}
		
		default:
			break;
	}
	
	gtk_widget_destroy(dialog);
}

/* Prototype for Handle terminal keypress events. */
gboolean edifor_gui_on_keypress(GtkWidget *terminal, GdkEventKey *event, 
	gpointer user_data) {
    /* Check for CTRL, ALT and SHIFT keys */
    switch (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) {
        /* CTRL + ALT */
        case GDK_MOD1_MASK | GDK_CONTROL_MASK:
            switch (event->keyval) {
            /* Paste */
            case GDK_KEY_v:
                vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                return TRUE;

            /* Copy */
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                	VTE_FORMAT_TEXT);
                return TRUE;  

	     /* 
		Change font size 
			CTRL + ALT + 1
			CTRL + ALT + 2
			CTRL + ALT + = 
	      */
             case GDK_KEY_plus:
             case GDK_KEY_1:
                edifor_gui_set_Term_Font(currentFontSize + 1);
                return TRUE;
             
	     case GDK_KEY_minus:
             case GDK_KEY_2:
                edifor_gui_set_Term_Font(currentFontSize - 1);
                return TRUE;

             case GDK_KEY_equal:
	        edifor_gui_set_Term_Font(EDIFOR_GUI_FONT_SIZE);
	        return TRUE;
	    }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    /* Initialize GTK, the window and the terminal */  
    gtk_init(&argc, &argv);
    
    if(argc > 1)
    	input = argv[1];
	else input = NULL;
    
    printf("arg: %s\n", argv[1]);
    
    edifor_gui_start(argv[1]);
}
