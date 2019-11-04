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

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void v3270_install_property(GObjectClass *oclass, guint property_id, GParamSpec *pspec)
 {
 	v3270Class * klass = GTK_V3270_CLASS(oclass);

 	const struct
 	{
 		const char	*name;
 		GParamSpec	**prop;
 	} properties[] = {
 		{ "connected",		&klass->properties.online		},
 		{ "luname",			&klass->properties.luname		},
 		{ "url",			&klass->properties.url			},
 		{ "model",			&klass->properties.model		},
 		{ "has-selection",	&klass->properties.selection	},
 	};

 	size_t ix;

// 	debug("Property %s=%u",g_param_spec_get_name(pspec),(unsigned int) property_id);
	g_object_class_install_property(oclass, property_id, pspec);

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++)
	{
		if(!g_ascii_strcasecmp(properties[ix].name,g_param_spec_get_name(pspec)))
		{
//			debug("Property \"%s\" is special",g_param_spec_get_name(pspec));
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
	klass->properties.font_family = g_param_spec_string(
							"font_family",
							"font_family",
							_("Font family for terminal contents"),
							FALSE,
							G_PARAM_READABLE|G_PARAM_WRITABLE
						);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_FONT_FAMILY,
		klass->properties.font_family
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
	klass->properties.auto_disconnect = g_param_spec_string(
							"auto_disconnect",
							"auto_disconnect",
							_("IDLE minutes for automatic disconnection"),
							FALSE,
							G_PARAM_READABLE|G_PARAM_WRITABLE
						);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_AUTO_DISCONNECT,
		klass->properties.auto_disconnect
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
	klass->properties.remap_file = g_param_spec_string(
							"remap",
							"remap",
							_("XML file with remap table"),
							FALSE,
							G_PARAM_READABLE|G_PARAM_WRITABLE
						);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_REMAP_FILE,
		klass->properties.remap_file
	);

	// Dynamic font spacing
	klass->properties.dynamic_spacing = g_param_spec_boolean(
				"dynamic_font_spacing",
				"dynamic_font_spacing",
				_( "State of the dynamic font spacing" ),
				FALSE,
				G_PARAM_READABLE|G_PARAM_WRITABLE);

	g_object_class_install_property(
		gobject_class,
		V3270_PROPERTY_DYNAMIC_SPACING,
		klass->properties.dynamic_spacing
	);


	//
	// Create dynamic properties
	klass->properties.count = V3270_PROPERTY_DYNAMIC;

	//
	// Extract properties from LIB3270 control tables
	//
	// Extract toggle class.
	klass->properties.type.toggle = klass->properties.count;
	for(ix = 0; ix < LIB3270_TOGGLE_COUNT; ix++)
	{
//		debug("Property %u=%s (Toggle)",(unsigned int) klass->properties.type.toggle + ix, lib3270_get_toggle_name(ix));

		klass->properties.toggle[ix] =
				g_param_spec_boolean(
					lib3270_get_toggle_name(ix),
					lib3270_get_toggle_name(ix),
					lib3270_get_toggle_description(ix),
					FALSE,
					G_PARAM_WRITABLE|G_PARAM_READABLE
		);

		v3270_install_property(gobject_class, klass->properties.count++, klass->properties.toggle[ix]);

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
					FALSE,
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

//		debug("Property %u=%s (String)",(unsigned int) klass->properties.type.str + ix, str_props[ix].name);

		spec = g_param_spec_string(
					str_props[ix].name,
					str_props[ix].name,
					str_props[ix].description,
					FALSE,
					(str_props[ix].set == NULL ? G_PARAM_READABLE : (G_PARAM_READABLE|G_PARAM_WRITABLE))
		);

		v3270_install_property(gobject_class, klass->properties.count++, spec);

	}

 }

