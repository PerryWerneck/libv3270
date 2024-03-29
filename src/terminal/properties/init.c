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

 #include "private.h"
 #include <lib3270/toggle.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void v3270_install_property(GObjectClass *oclass, guint property_id, GParamSpec *pspec)
 {
 	v3270Class * klass = GTK_V3270_CLASS(oclass);

 	const struct
 	{
 		const char	*name;
 		GParamSpec	**prop;
 	} properties[] = {
 		{ "connected",				&klass->properties.online									},
 		{ "associated-lu",			&klass->properties.associated_lu							},
 		{ "has-selection",			&klass->properties.selection								},
 	};

 	size_t ix;

	g_object_class_install_property(oclass, property_id, pspec);

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++)
	{
		if(!g_ascii_strcasecmp(properties[ix].name,g_param_spec_get_name(pspec)))
		{
			*properties[ix].prop = pspec;
			break;
		}
	}

 }

 void v3270_init_properties(GObjectClass * gobject_class)
 {
 	size_t		  ix;
 	GParamSpec	* spec;
 	v3270Class	* klass = GTK_V3270_CLASS(gobject_class);

 	debug("%s",__FUNCTION__);

	gobject_class->set_property = v3270_set_property;
	gobject_class->get_property = v3270_get_property;

	// Setup internal properties.

	// Font family
	spec =
		g_param_spec_string(
			"font_family",
			"font_family",
			_("Font family for terminal contents"),
			v3270_get_default_font_name(),
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_FONT_FAMILY,
		spec
	);

	// Session name.
	klass->properties.session_name = g_param_spec_string(
							"session_name",
							"session_name",
							_("TN3270 Session name"),
							FALSE,
							G_PARAM_READABLE|G_PARAM_WRITABLE
						);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_SESSION_NAME,
		klass->properties.session_name
	);

	// Auto disconnect
	spec =
		g_param_spec_uint(
			"auto_disconnect",
			"auto_disconnect",
			_("IDLE minutes for automatic disconnection"),
			0,
			G_MAXUINT,
			0,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_AUTO_DISCONNECT,
		spec
	);

	// Clipboard
	spec = g_param_spec_string(
							"clipboard",
							"clipboard",
							_("Clipboard name"),
							FALSE,
							G_PARAM_READABLE|G_PARAM_WRITABLE
						);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_CLIPBOARD,
		spec
	);

	// Remap file
	spec =
		g_param_spec_string(
			"remap_file",
			"remap_file",
			_("XML file with remap table"),
			FALSE,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_REMAP_FILE,
		spec
	);

	// Dynamic font spacing
	spec =
		g_param_spec_boolean(
			"dynamic_font_spacing",
			"dynamic_font_spacing",
			_( "State of the dynamic font spacing" ),
			FALSE,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_DYNAMIC_SPACING,
		spec
	);

	// Open URL
	spec =
		g_param_spec_boolean(
			"open-url",
			"open-url",
			_( "Emit signal 'open-url' when an http:// or https:// string is selected by action 'word-select'" ),
			FALSE,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_OPEN_URL,
		spec
	);

	// Lu names
	spec =
		g_param_spec_string(
			"lu_names",
			"lu_names",
			_("Comma separated list of LU names"),
			NULL,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_LU_NAMES,
		spec
	);

	// Trace
	klass->properties.trace = g_param_spec_boolean(
				"trace",
				"trace",
				_( "Trace window" ),
				FALSE,
				G_PARAM_READABLE|G_PARAM_WRITABLE);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_TRACE,
		klass->properties.trace
	);

	// Colors
	spec =
		g_param_spec_string(
			"colors",
			"colors",
			_("The terminal colors"),
			v3270_get_default_colors(),
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_TERMINAL_COLORS,
		spec
	);

	// Clipboard options
	spec =
		g_param_spec_uint(
			"selection_flags",
			"selection_flags",
			_("Flags to cut&paste"),
			0,
			G_MAXUINT,
			V3270_SELECTION_DEFAULT,
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_SELECTION_OPTIONS,
		spec
	);

	klass->properties.has_copy =
		g_param_spec_boolean(
			"has_copy",
			"has_copy",
			_( "True if the terminal has copy blocks" ),
			FALSE,
			G_PARAM_READABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_HAS_COPY,
		klass->properties.has_copy
	);

	klass->properties.has_timer =
		g_param_spec_boolean(
			"has_timer",
			"has_timer",
			_( "True if the timer indicator is active" ),
			FALSE,
			G_PARAM_READABLE
		);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_HAS_TIMER,
		klass->properties.has_timer
	);

	//
	// Create dynamic properties
	//
	klass->properties.count = V3270_PROPERTY_DYNAMIC;

	//
	// Extract properties from LIB3270 control tables
	//
	// Extract toggle class.
	klass->properties.type.toggle = klass->properties.count;

	{
		const LIB3270_TOGGLE * toggles = lib3270_get_toggles();

		for(ix = 0; ix < LIB3270_TOGGLE_COUNT; ix++)
		{
			if(!toggles[ix].name)
			{
				g_warning("Unexpected toggle id: %u", (unsigned int) ix);
				break;
			}

	//		debug("Property %u=%s (Toggle)",(unsigned int) klass->properties.type.toggle + ix, lib3270_get_toggle_name(ix));

			klass->properties.toggle[ix] =
					g_param_spec_boolean(
						toggles[ix].name,
						toggles[ix].name,
						toggles[ix].description,
						(toggles[ix].def == 0 ? FALSE : TRUE),
						G_PARAM_WRITABLE|G_PARAM_READABLE
			);

			v3270_install_property(gobject_class, klass->properties.count++, klass->properties.toggle[ix]);

		}

	}

	// Creating boolean properties.
	klass->properties.type.boolean = klass->properties.count;
	const LIB3270_INT_PROPERTY * bool_props = lib3270_get_boolean_properties_list();

	for(ix = 0; bool_props[ix].name; ix++)
	{
//		debug("Property %u=%s (Boolean)",(unsigned int) klass->properties.type.boolean + ix, bool_props[ix].name);
		spec = g_param_spec_boolean(
					bool_props[ix].name,
					bool_props[ix].name,
					bool_props[ix].description,
					bool_props[ix].default_value != 0,
					(bool_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, klass->properties.count++, spec);

	}

	// Creating signed integer properties.
	const LIB3270_INT_PROPERTY * int_props = lib3270_get_int_properties_list();
	klass->properties.type.integer = klass->properties.count;

	for(ix = 0; int_props[ix].name; ix++)
	{
//		debug("Property %u=%s (Integer)",(unsigned int) klass->properties.type.integer + ix, int_props[ix].name);

		spec = g_param_spec_int(
			int_props[ix].name,
			int_props[ix].name,
			int_props[ix].description,
			0,			// Minimo
			INT_MAX,	// Máximo
			0,			// Default
			(int_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, klass->properties.count++, spec);

	}

	// Creating unsigned integer properties.
	const LIB3270_UINT_PROPERTY * uint_props = lib3270_get_unsigned_properties_list();
	klass->properties.type.uint = klass->properties.count;

	for(ix = 0; uint_props[ix].name; ix++)
	{
//		debug("Property %u=%s (unsigned)",(unsigned int) klass->properties.type.integer + ix, uint_props[ix].name);

		spec = g_param_spec_uint(
			uint_props[ix].name,
			uint_props[ix].name,
			uint_props[ix].description,
			uint_props[ix].min,																	// Minimo
			(uint_props[ix].max ? uint_props[ix].max : UINT_MAX),								// Máximo
			(uint_props[ix].default_value ? uint_props[ix].default_value : uint_props[ix].min),	// Default
			(uint_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, klass->properties.count++, spec);

	}

	// Creating string properties.
	const LIB3270_STRING_PROPERTY * str_props = lib3270_get_string_properties_list();
	klass->properties.type.str = klass->properties.count;

	for(ix = 0; str_props[ix].name; ix++)
	{

		// debug("Property %u=%s (String)",(unsigned int) klass->properties.type.str + ix, str_props[ix].name);

		spec = g_param_spec_string(
					str_props[ix].name,
					str_props[ix].name,
					str_props[ix].description,
					str_props[ix].default_value,
					(str_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, klass->properties.count++, spec);

	}

	//
	// Create action properties.
	//
	/*
	klass->properties.type.responses = klass->properties.count;

	static const struct
	{
		const gchar *name;			///< @brief canonical name of the property specified.
		const gchar *nick;			///< @brief nick name for the property specified.
		const gchar *blurb;			///< @brief description of the property specified.
	} actions[G_N_ELEMENTS(klass->responses)] =
	{
		{
			.name = "paste_fails",
			.nick = "paste_fails",
			.blurb = "The action when formatted paste fails"
		}

	};

	for(ix = 0; ix < G_N_ELEMENTS(klass->responses); ix++)
	{
		if(actions[ix].name)
		{
			// Should be int to make it easier to GKeyFile methods.
			klass->responses[ix] =
				g_param_spec_int(
					actions[ix].name,
					actions[ix].nick,
					actions[ix].blurb,
					INT_MIN,
					INT_MAX,
					(int) GTK_RESPONSE_NONE,
					(G_PARAM_READABLE|G_PARAM_WRITABLE)
				);

			v3270_install_property(gobject_class, klass->properties.count++, klass->responses[ix]);
		}

	}
	*/

 }

