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
 #include <internals.h>
 #include <terminal.h>
 #include <lib3270/actions.h>
 #include <gdk/gdkkeysyms-compat.h>
 #include <v3270/actions.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #ifndef GDK_ALT_MASK
	#define GDK_ALT_MASK GDK_MOD1_MASK
 #endif

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const V3270_ACTION actions[] =
 {
 	{
 		.name = "keypad-add",
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.key = GDK_KP_Add,
 		.mods = GDK_NUMLOCK_MASK,
 		.activate = fire_keypad_action
	},
 	{
 		.name = "keypad-subtract",
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.key = GDK_KP_Subtract,
 		.mods = GDK_NUMLOCK_MASK,
 		.activate = fire_keypad_action
	},

	// Standard Clipboard actions
	{
		.flags = 0,
		.name = "copy",
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.icon = "edit-copy",
		.label = N_( "Copy" ),
		.summary = N_("Copy selection to clipboard"),
		.description = N_("Replace current clipboard contents with the selected area"),
		.key = 'c',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_COPY_APPEND,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "copy-append",
		.label = N_( "Add to copy" ),
		.summary = N_("Append selection to clipboard"),
		.description = N_("Append selected area to current clipboard contents"),
		.key = 'c',
		.mods = GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_COPY_TEXT,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "copy-text",
		.icon = "edit-copy",
		.label = N_( "Copy" ),
		.summary = N_( "Copy as plain text" ),
		.key = 'c',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_COPY_TABLE,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "copy-table",
		.icon = "edit-copy",
		.label = N_( "Copy as table" ),
		.summary = N_( "Copy as table" ),
		.key = 'c',
		.mods = GDK_SHIFT_MASK|GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_DEFAULT,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "cut",
		.icon = "edit-cut",
		.label = N_( "Cut" ),
		.key = 'x',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_APPEND,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "cut-append",
		.label = N_( "Cut" ),
		.summary = N_( "Cut and append to copy" ),
		.key = 'x',
		.mods = GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TEXT,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "cut-text",
		.icon = "edit-cut",
		.label = N_( "Cut" ),
		.summary = N_( "Cut as plain text" ),
		.key = 'x',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TABLE,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
		.name = "cut-table",
		.icon = "edit-cut",
		.label = N_( "Cut as table" ),
		.summary = N_( "Cut as table" ),
		.key = 'x',
		.mods = GDK_SHIFT_MASK|GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = 0,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
		.name = "paste",
		.icon = "edit-paste",
		.label = N_("Paste"),
		.summary = N_("Paste clipboard contents"),
		.description = N_("Input current clipboard contents to screen"),
		.key = 'v',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_paste_accelerator
	},

	{
		.flags = 1,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
		.name = "paste-text",
		.icon = "edit-paste",
		.label = N_("Paste"),
		.summary = N_("Paste as plain text"),
		.key = 'v',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_paste_accelerator
	},

	{
		.flags = 2,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
		.name = "paste-file",
		.label = N_("Paste file"),
		.summary = N_("Paste from text file"),
		.key = 'v',
		.mods = GDK_ALT_MASK,
 		.activate = fire_paste_accelerator
	},

 	{
 		.flags = 0,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.name = "zoom-in",
 		.icon = "zoom-in",
 		.label = N_("Zoom in"),
 		.summary = N_("Increase the font size"),
 		.key = GDK_KP_Add,
 		.mods = GDK_CONTROL_MASK,
 		.activate = fire_zoom_action
	},

 	{
 		.flags = 1,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.name = "zoom-out",
 		.label = N_("Zoom out"),
 		.summary = N_("decrease the font size"),
 		.icon = "zoom-out",
 		.key = GDK_KP_Subtract,
 		.mods = GDK_CONTROL_MASK,
 		.activate = fire_zoom_action
	},

 	{
 		.flags = 2,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.name = "zoom-fit-best",
 		.label = N_("Fit best"),
 		.summary = N_("Set the font to the best size for window"),
 		.icon = "zoom-fit-best",
 		.key = '0',
 		.mods = GDK_CONTROL_MASK,
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
 		.flags = LIB3270_CONTENT_ALL,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.name = "save-all",
 		.label = N_("Save all"),
 		.icon = "document-save-as",
 		.summary = N_("Save screen"),
 		.activate = fire_save_action

	},

	{
 		.flags = LIB3270_CONTENT_SELECTED,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
 		.name = "save-selected",
 		.label = N_("Save selected"),
 		.icon = "document-save-as",
 		.summary = N_("Save selected area"),
 		.activate = fire_save_action

	},

	{
 		.flags = LIB3270_CONTENT_COPY,
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
 		.flags = LIB3270_CONTENT_ALL,
 		.group = LIB3270_ACTION_GROUP_ONLINE,
 		.name = "print-all",
 		.icon = "document-print",
 		.label = N_("Print screen"),
 		.summary = N_("Print the entire screen"),
 		.activate = fire_print_action

	},

	{
 		.flags = LIB3270_CONTENT_SELECTED,
 		.group = LIB3270_ACTION_GROUP_SELECTION,
 		.name = "print-selected",
 		.icon = "document-print",
 		.label = N_("Print selected"),
 		.summary = N_("Print selected area"),
 		.activate = fire_print_action

	},

	{
 		.flags = LIB3270_CONTENT_COPY,
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

 LIB3270_EXPORT const V3270_ACTION * v3270_get_actions()
 {
	return actions;
 }

 static void activate_v3270(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal)
 {
 	debug("Activating action \"%s\"",g_action_get_name(action));

 	V3270_ACTION * descriptor = (V3270_ACTION *) ((V3270Action *) action)->info;
	descriptor->activate(terminal,descriptor);

 }

 void g_action_map_add_v3270_actions(GActionMap *action_map)
 {

	const V3270_ACTION * actions = v3270_get_actions();
	size_t ix;

	for(ix = 0; actions[ix].name; ix++) {

		V3270Action * action = V3270_ACTION(g_object_new(V3270_TYPE_ACTION, NULL));

		action->info = (const LIB3270_PROPERTY *) &actions[ix];
		action->activate = activate_v3270;
		action->translation_domain = GETTEXT_PACKAGE;

		if(!g_action_get_name(G_ACTION(action))) {
			g_warning("Action \"%s\" is invalid",actions[ix].name);
		} else {
			g_action_map_add_action(action_map,G_ACTION(action));
		}

	}

 }

