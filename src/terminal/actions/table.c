/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como properties.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <lib3270/actions.h>
 #include <v3270/actions.h>
 #include <v3270/selection.h>
 #include <terminal.h>

// static int fire_kp_add_action(GtkWidget *widget, const struct _v3270_action * action);
// static int fire_kp_sub_action(GtkWidget *widget, const struct _v3270_action * action);

 static int fire_copy_as_html(GtkWidget *widget, const struct _v3270_action * action);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT const V3270_ACTION * v3270_get_actions() {

	 static const V3270_ACTION actions[] = {

	 	/*
		{
			.name = "keypad-add",
			.keys = "<Mod2>KP_Add",
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.activate = fire_kp_add_action
		},
		{
			.name = "keypad-subtract",
			.keys = "<Mod2>KP_Subtract",
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.activate = fire_kp_sub_action
		},
		*/

		// Standard Clipboard actions
		{
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_SMART,
			.name = "copy",
			.keys = "<Primary>c",
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy" ),
			.summary = N_("Copy selection to clipboard"),
			.description = N_("Replace current clipboard contents with the selected area"),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-html",
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Copy as HTML" ),
			.summary = N_("Copy selection in HTML format"),
			.description = N_("Replace current clipboard contents with the selected area in HTML format"),
			.activate = fire_copy_as_html
		},

		{
			.name = "copy-append",
			.keys = "<Alt>c",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_APPEND,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Add to copy" ),
			.summary = N_("Append selection to clipboard"),
			.description = N_("Append selected area to current clipboard contents"),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-text",
			.keys = "<Primary><Shift>c",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_TEXT,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy" ),
			.summary = N_( "Copy as plain text" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "copy-table",
			.keys = "<Shift><Alt>c",
			.flags = (V3270_ACTION_FLAGS) V3270_COPY_TABLE,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-copy",
			.label = N_( "Copy as table" ),
			.summary = N_( "Copy as table" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut",
			.keys = "<Primary>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_SMART,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-append",
			.keys = "<Alt>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_APPEND,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.label = N_( "Cut" ),
			.summary = N_( "Cut and append to copy" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-text",
			.keys = "<Primary><Shift>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TEXT,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut" ),
			.summary = N_( "Cut as plain text" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "cut-table",
			.keys = "<Shift><Alt>x",
			.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TABLE,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.icon = "edit-cut",
			.label = N_( "Cut as table" ),
			.summary = N_( "Cut as table" ),
			.activate = fire_copy_accelerator
		},

		{
			.name = "paste",
			.keys = "<Primary>v",
			.flags = 0,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "edit-paste",
			.label = N_("Paste"),
			.summary = N_("Paste clipboard contents"),
			.description = N_("Input current clipboard contents to screen"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-text",
			.keys = "<Primary><Shift>v",
			.flags = 1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "edit-paste",
			.label = N_("Paste"),
			.summary = N_("Paste as plain text"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-file",
			.keys = "<Alt>v",
			.flags = 2,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Paste file"),
			.summary = N_("Paste from text file"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "paste-screen",
			.keys = "<Primary><Shift><Alt>v",
			.flags = 3,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Paste formatted screen"),
			.summary = N_("Paste similar screen from clipboard"),
			.description = N_("Search clipboard for a similar screen, if found paste unprotected fields and restore cursor position"),
			.activate = fire_paste_accelerator
		},

		{
			.name = "zoom-in",
			.keys = "<Primary>KP_Add",
			.flags = 0,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.icon = "zoom-in",
			.label = N_("Zoom in"),
			.summary = N_("Increase the font size"),
			.activate = fire_zoom_action
		},

		{
			.name = "zoom-out",
			.keys = "<Primary>KP_Subtract",
			.flags = 1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Zoom out"),
			.summary = N_("decrease the font size"),
			.icon = "zoom-out",
			.activate = fire_zoom_action
		},

		{
			.name = "zoom-fit-best",
			.keys = "<Primary>0",
			.flags = 2,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.label = N_("Fit best"),
			.summary = N_("Set the font to the best size for window"),
			.icon = "zoom-fit-best",
			.activate = fire_zoom_action
		},

		//
		// Save actions
		//
		{
			.flags = -1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "save",
			.icon = "document-save-as",
			.label = N_("Save"),
			.summary = N_("Save screen or selection"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_ALL,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "save-all",
			.label = N_("Save all"),
			.icon = "document-save-as",
			.summary = N_("Save screen"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_SELECTED,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.name = "save-selected",
			.label = N_("Save selected"),
			.icon = "document-save-as",
			.summary = N_("Save selected area"),
			.activate = fire_save_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_COPY,
			.group = LIB3270_ACTION_GROUP_COPY,
			.name = "save-copy",
			.label = N_("Save copy"),
			.icon = "document-save-as",
			.summary = N_("Save Copy"),
			.activate = fire_save_action

		},

		//
		// Print actions
		//
		{
			.flags = -1,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "print",
			.icon = "document-print",
			.label = N_("Print"),
			.summary = N_("Print screen or selection"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_ALL,
			.group = LIB3270_ACTION_GROUP_ONLINE,
			.name = "print-all",
			.icon = "document-print",
			.label = N_("Print screen"),
			.summary = N_("Print the entire screen"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_SELECTED,
			.group = LIB3270_ACTION_GROUP_SELECTION,
			.name = "print-selected",
			.icon = "document-print",
			.label = N_("Print selected"),
			.summary = N_("Print selected area"),
			.activate = fire_print_action

		},

		{
			.flags = (V3270_ACTION_FLAGS) LIB3270_CONTENT_COPY,
			.group = LIB3270_ACTION_GROUP_COPY,
			.name = "print-copy",
			.icon = "document-print",
			.label = N_("Print Copy"),
			.activate = fire_print_action

		},

		{
			.name = NULL
		}
	 };

	return actions;

 }

 static int fire_copy_as_html(GtkWidget *widget, const struct _v3270_action G_GNUC_UNUSED(* action)) {
	v3270_copy_as_html(widget);
	return 0;
 }

 /*
 int fire_kp_add_action(GtkWidget *widget, const struct _v3270_action G_GNUC_UNUSED(* action)) {

	if(v3270_get_toggle(widget,LIB3270_TOGGLE_KP_ALTERNATIVE))
		return lib3270_nextfield(GTK_V3270(widget)->host);

	v3270_set_string(widget, "+");

	return 0;

 }

 int fire_kp_sub_action(GtkWidget *widget, const struct _v3270_action G_GNUC_UNUSED(* action)) {

	if(v3270_get_toggle(widget,LIB3270_TOGGLE_KP_ALTERNATIVE))
		return lib3270_previousfield(GTK_V3270(widget)->host);

	v3270_set_string(widget, "-");

	return 0;

 }
*/
