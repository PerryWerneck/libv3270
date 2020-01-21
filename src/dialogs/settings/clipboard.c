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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /**
  * @brief Implements the cut & paste settings widget.
  *
  */

 #include <internals.h>
 #include <v3270/settings.h>
 #include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 typedef struct _V3270ClipboardSettings {

 	V3270Settings parent;


 } V3270ClipboardSettings;

 typedef struct _V3270ClipboardSettingsClass {

	V3270SettingsClass parent_class;


 } V3270ClipboardSettingsClass;

 static void load(GtkWidget *w, GtkWidget *terminal);
 static void apply(GtkWidget *w, GtkWidget *terminal);

 G_DEFINE_TYPE(V3270ClipboardSettings, V3270ClipboardSettings, GTK_TYPE_V3270_SETTINGS);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void V3270ClipboardSettings_class_init(V3270ClipboardSettingsClass *klass) {

	V3270SettingsClass * widget = GTK_V3270_SETTINGS_CLASS(klass);

	widget->apply = apply;
	widget->load = load;

}

static void V3270ClipboardSettings_init(V3270ClipboardSettings *widget) {

}

GtkWidget * v3270_clipboard_settings_new() {

 	V3270Settings * settings = GTK_V3270_SETTINGS(g_object_new(V3270ClipboardSettings_get_type(), NULL));

 	settings->title = _("Cut & Paste settings");
 	settings->label = _("Clipboard");

 	return GTK_WIDGET(settings);
}

static void apply(GtkWidget *w, GtkWidget *terminal) {

}

static void load(GtkWidget *w, GtkWidget *terminal) {

}

