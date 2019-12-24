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
 		.key = GDK_KP_Add,
 		.mods = GDK_NUMLOCK_MASK,
 		.activate = fire_keypad_action
	},
 	{
 		.name = "keypad-subtract",
 		.key = GDK_KP_Subtract,
 		.mods = GDK_NUMLOCK_MASK,
 		.activate = fire_keypad_action
	},

	// Standard Clipboard actions
	{
		.flags = 0,
		.name = "copy",
		.icon = "edit-copy",
		.summary = N_( "Copy" ),
		.key = 'c',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_COPY_APPEND,
		.name = "copy-append",
		.summary = N_( "Add to copy" ),
		.key = 'c',
		.mods = GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_COPY_TEXT,
		.name = "copy-text",
		.icon = "edit-copy",
		.summary = N_( "Copy as plain text" ),
		.key = 'c',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_DEFAULT,
		.name = "cut",
		.icon = "edit-cut",
		.summary = N_( "Cut" ),
		.key = 'x',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_APPEND,
		.name = "cut-append",
		.summary = N_( "Cut and append to copy" ),
		.key = 'x',
		.mods = GDK_ALT_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = V3270_ACTION_FLAG_CUT|V3270_COPY_TEXT,
		.name = "cut-text",
		.icon = "edit-cut",
		.summary = N_( "Cut as plain text" ),
		.key = 'x',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_copy_accelerator
	},

	{
		.flags = 0,
		.name = "paste",
		.icon = "edit-paste",
		.summary = N_("Paste"),
		.key = 'v',
		.mods = GDK_CONTROL_MASK,
 		.activate = fire_paste_accelerator
	},

	{
		.flags = 1,
		.name = "paste-text",
		.icon = "edit-paste",
		.summary = N_("Paste as plain text"),
		.key = 'v',
		.mods = GDK_SHIFT_MASK|GDK_CONTROL_MASK,
 		.activate = fire_paste_accelerator
	},

	{
		.flags = 2,
		.name = "paste-file",
		.summary = N_("Paste from text file"),
		.key = 'v',
		.mods = GDK_ALT_MASK,
 		.activate = fire_paste_accelerator
	},

 	{
 		.flags = 0,
 		.name = "zoom-in",
 		.icon = "zoom-in",
 		.summary = N_("Zoom in"),
 		.key = GDK_KP_Add,
 		.mods = GDK_CONTROL_MASK,
 		.activate = fire_zoom_action
	},

 	{
 		.flags = 1,
 		.name = "zoom-out",
 		.summary = N_("Zoom out"),
 		.icon = "zoom-out",
 		.key = GDK_KP_Subtract,
 		.mods = GDK_CONTROL_MASK,
 		.activate = fire_zoom_action
	},

 	{
 		.flags = 2,
 		.name = "zoom-fit-best",
 		.summary = N_("Zoom to best size"),
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
 		.name = "save",
 		.icon = "document-save-as",
 		.summary = N_("Save screen or selection"),
 		.activate = fire_save_action

	},

	{
 		.flags = LIB3270_CONTENT_ALL,
 		.name = "save-all",
 		.icon = "document-save-as",
 		.summary = N_("Save screen"),
 		.activate = fire_save_action

	},

	{
 		.flags = LIB3270_CONTENT_SELECTED,
 		.name = "save-selected",
 		.icon = "document-save-as",
 		.summary = N_("Save selected area"),
 		.activate = fire_save_action

	},

	{
 		.flags = LIB3270_CONTENT_COPY,
 		.name = "save-copy",
 		.icon = "document-save-as",
 		.summary = N_("Save Copy"),
 		.activate = fire_save_action

	},

	//
	// Print actions
	//
	{
 		.flags = -1,
 		.name = "print",
 		.icon = "document-print",
 		.summary = N_("Print screen or selection"),
 		.activate = fire_print_action

	},

	{
 		.flags = LIB3270_CONTENT_ALL,
 		.name = "print-all",
 		.icon = "document-print",
 		.summary = N_("Print screen"),
 		.activate = fire_print_action

	},

	{
 		.flags = LIB3270_CONTENT_SELECTED,
 		.name = "print-selected",
 		.icon = "document-print",
 		.summary = N_("Print selected area"),
 		.activate = fire_print_action

	},

	{
 		.flags = LIB3270_CONTENT_COPY,
 		.name = "print-copy",
 		.icon = "document-print",
 		.summary = N_("Print Copy"),
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


