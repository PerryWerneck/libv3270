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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#include <internals.h>
	#include <lib3270/log.h>
	#include <lib3270/toggle.h>
	#include <v3270/settings.h>

	G_GNUC_INTERNAL void gtk_combo_box_select_column_uint(GtkComboBox * combo_box, gint column, guint value);

	/// @brief Toggle button factory.
	struct ToggleButtonDefinition {
		gint left;
		gint top;
		gint width;
		unsigned short grid;
		LIB3270_TOGGLE_ID	id;
	};

	G_GNUC_INTERNAL void v3270_settings_create_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget **grids, GtkToggleButton **toggles);
	G_GNUC_INTERNAL void v3270_settings_load_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget *terminal, GtkToggleButton **toggles);
	G_GNUC_INTERNAL void v3270_settings_apply_toggle_buttons(const struct ToggleButtonDefinition * definitions, size_t length, GtkWidget *terminal, GtkToggleButton **toggles);

	/// @brief Entry field factory.
	struct EntryFieldDefinition {

		ENTRY_FIELD_HEAD

		unsigned short grid;
		gint max_length;
		gint width_chars;

	};

	G_GNUC_INTERNAL void v3270_settings_create_entry_fields(const struct EntryFieldDefinition * definitions, size_t length, GtkWidget **grids, GtkEntry **entries);

	/// @brief Combo Box factory
	struct ComboBoxDefinition {

		ENTRY_FIELD_HEAD

		unsigned short grid;

		gint n_columns;
		const GType *types;

	};

	G_GNUC_INTERNAL void v3270_settings_create_combos(const struct ComboBoxDefinition * definitions, size_t length, GtkWidget **grids, GtkComboBox **combos);

#endif // PRIVATE_H_INCLUDED
