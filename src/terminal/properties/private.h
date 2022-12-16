/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #pragma once

 #ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
 #endif // WIN32

 #include <gtk/gtk.h>
 #include <limits.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/properties.h>
 #include <lib3270/toggle.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <v3270.h>
 #include <terminal.h>

 enum _v3270_internal_property
 {
	V3270_PROPERTY_FONT_FAMILY			= 2,	///< @brief Name of the font-family used by widget.
	V3270_PROPERTY_CLIPBOARD			= 3,	///< @brief Name of the selected clipboard.
	V3270_PROPERTY_SESSION_NAME			= 4,	///< @brief Widget's session name.
	V3270_PROPERTY_AUTO_DISCONNECT		= 5,	///< @brief Auto disconnect.
	V3270_PROPERTY_REMAP_FILE			= 6,	///< @brief Path of the remap file.
	V3270_PROPERTY_DYNAMIC_SPACING		= 7,	///< @brief Toggle dynamic font spacing.
	V3270_PROPERTY_LU_NAMES				= 8,	///< @brief The LU names list.
	V3270_PROPERTY_TRACE				= 9,	///< @brief Is the trace widget active?
	V3270_PROPERTY_TERMINAL_COLORS		= 10,	///< @brief Terminal colors.
	V3270_PROPERTY_SELECTION_OPTIONS	= 11,
	V3270_PROPERTY_HAS_COPY				= 12,	///< @brief Terminal has copy.
	V3270_PROPERTY_HAS_TIMER			= 13,	///< @brief Timer indicator state.
	V3270_PROPERTY_OPEN_URL				= 14,	///< @brief Open URL when 'word' selected.

	V3270_PROPERTY_DYNAMIC				= 15	///< @brief Id of the first LIB3270 internal property.
 };

 G_GNUC_INTERNAL void v3270_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 G_GNUC_INTERNAL void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);


