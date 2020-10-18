/* MIT License
# Forked from https://github.com/ferhatgec/gemini
# Copyright (c) 2020 Ferhat Geçdoğan All Rights Reserved.
# Distributed under the terms of the MIT License.
#
# */

#ifndef EDIFOR_GUI_H
#define EDIFOR_GUI_H

#include <vte/vte.h>

#define EDIFOR_GUI_VERSION "0.1-beta-2"

/* Setting values */
#define EDIFOR_GUI_FONT "Monospace Regular"
#define EDIFOR_GUI_FONT_SIZE 12

/* Prototype for Handle terminal keypress events. */
gboolean edifor_gui_on_keypress(GtkWidget *, GdkEventKey *,
	gpointer);

/* Prototype for detect Terminal window title */
gboolean edifor_gui_on_title_Changed(GtkWidget *, gpointer);

void edifor_gui_set_Term_Font(int);

#endif /* EDIFOR_GUI_H */
