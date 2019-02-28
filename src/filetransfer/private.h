/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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

	#define PRIVATE_H_INCLUDED 1

	#include <config.h>

	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include <libintl.h>
	#include <glib/gi18n.h>

	#include <v3270/filetransfer.h>
	#include <v3270/ftprogress.h>
	#include <lib3270.h>
	#include <lib3270/log.h>
	#include <lib3270/filetransfer.h>

	#define NUM_OPTIONS_WIDGETS 12
	#define NUM_TYPES			4
	#define ERROR_DOMAIN 		g_quark_from_static_string("v3270ft")

	#if GTK_CHECK_VERSION(3,14,0)
		#define HAVE_GTK_HEADER_BAR 1
	#endif // GTK 3.10

	typedef enum ft_button {

		FT_BUTTON_GO_FIRST,
		FT_BUTTON_GO_PREVIOUS,
		FT_BUTTON_GO_NEXT,
		FT_BUTTON_GO_LAST,
		FT_BUTTON_START_TRANSFER,
		FT_BUTTON_INSERT_FILE,
		FT_BUTTON_REMOVE_FILE,
		FT_BUTTON_SAVE_LIST,
		FT_BUTTON_LOAD_LIST,

		FT_BUTTON_COUNT
	} FT_BUTTON;

	struct v3270ft_option {

		LIB3270_FT_OPTION	  opt;
		const char			* name;
		const char			* label;
		const char			* tooltip;

	};

	struct v3270ft_type {
		LIB3270_FT_OPTION	  opt;
		const gchar			* name;
		const gchar			* type;
		const gchar			* label;
	};

	#define LIB3270_FT_TYPE_OPTIONS (LIB3270_FT_OPTION_SEND|LIB3270_FT_OPTION_RECEIVE|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP)

	struct v3270ft_value {

		const gchar * name;
		guint		  minval;
		guint		  maxval;
		const gchar * label;
		const gchar * tooltip;

	};

	struct v3270_activity_list_option
	{
		LIB3270_FT_OPTION	  option;
		const gchar			* name;
		const gchar			* value;
	};

	struct v3270_ft_worker_field
	{
		const gchar *label;
		const gchar *tooltip;
	};

	typedef enum progress_field {

		PROGRESS_FIELD_LOCAL,
		PROGRESS_FIELD_REMOTE,
		PROGRESS_FIELD_TOTAL,
		PROGRESS_FIELD_CURRENT,
		PROGRESS_FIELD_SPEED,
		PROGRESS_FIELD_ETA,

		PROGRESS_FIELD_COUNT

	} PROGRESS_FIELD;

	G_GNUC_INTERNAL extern const struct v3270ft_option				ft_option[];
	G_GNUC_INTERNAL extern const struct v3270ft_type				ft_type[];
	G_GNUC_INTERNAL extern const struct v3270ft_value				ft_value[];
	G_GNUC_INTERNAL extern const struct v3270_activity_list_option	v3270_activity_list_options[];
	G_GNUC_INTERNAL extern const struct v3270_ft_worker_field		v3270_ft_worker_fields[PROGRESS_FIELD_COUNT];
	G_GNUC_INTERNAL extern const gchar *							v3270_text_file_extensions[];

	#define ENTRY_FILENAME_LENGTH FILENAME_MAX
	struct v3270ft_entry {
		gint				  type;								///< Transfer type.
		gboolean			  valid;							///< Is dialog content valid?
		gchar				  local[ENTRY_FILENAME_LENGTH+1];	///< Local file name.
		gchar				  remote[ENTRY_FILENAME_LENGTH+1];	///< Remote file name.
		LIB3270_FT_OPTION	  options;							///< File Transfer options.
		guint				  value[LIB3270_FT_VALUE_COUNT];	///< File transfer values.
	};

	struct _v3270ft {
		GtkDialog parent;

		GtkComboBox 		* type;
		GtkEntry			* local;							///< Local file name
		GtkEntry			* remote;							///< Remote filename

		GtkWidget 			* opt[NUM_OPTIONS_WIDGETS];
		GtkWidget			* button[FT_BUTTON_COUNT];
		GtkWidget			* radio[2];
		GtkWidget			* value[LIB3270_FT_VALUE_COUNT];

		GList				* files;							/// List of files.
		GList				* active;							/// Active element.

	};

	enum V3270FTPROGRESS_SIGNAL {

		V3270FTPROGRESS_SIGNAL_SUCCESS,
		V3270FTPROGRESS_SIGNAL_FAILED,

		V3270FTPROGRESS_SIGNAL_COUNT,
	};

	struct _v3270ftprogress {

		GtkDialog				  parent;

		struct v3270ft_entry	* active;						///< Transferência em andamento.
		H3270					* session;						///< lib3270 session handle.
		unsigned long			  current;						///< Quantidade de bytes recebidos/enviados.
		time_t					  timeout;						///< When the transfer will fail with timeout.

		GtkEntry				* info[PROGRESS_FIELD_COUNT];	///< Widgets com informações da transferência atual.
		GtkProgressBar			* progress;
		GSource					* pulse;
		GSource					* timer;

	};

	struct _v3270ftClass {

		GtkDialogClass parent_class;
		int dummy;

	};

	struct _v3270ftprogressClass {

		GtkDialogClass parent_class;

		/* Signals */
		void 		(*complete)(GtkWidget *widget);
		void 		(*failed)(GtkWidget *widget);

	};

	G_GNUC_INTERNAL guint	  v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_COUNT];


	G_GNUC_INTERNAL void	  v3270ft_update_actions(v3270ft *dialog);
	G_GNUC_INTERNAL void	  v3270ft_update_state(struct v3270ft_entry *entry);
	G_GNUC_INTERNAL void 	  v3270ft_set_active(v3270ft *dialog, GList * active);
	G_GNUC_INTERNAL GtkGrid	* v3270ft_new_grid(void);
	G_GNUC_INTERNAL	void	  v3270ft_clear(v3270ft *dialog);

	G_GNUC_INTERNAL void	  v3270ftprogress_set_session(GtkWidget *widget, H3270 *session);
	G_GNUC_INTERNAL void	  v3270ftprogress_update(GtkWidget *widget, unsigned long current, unsigned long total, double kbytes_sec);

	G_GNUC_INTERNAL void	  v3270ftprogress_set_transfer(GtkWidget *widget, struct v3270ft_entry *entry);
	G_GNUC_INTERNAL void	  v3270ftprogress_start_transfer(GtkWidget *widget);

	G_GNUC_INTERNAL struct v3270ft_entry * v3270ft_get_selected(v3270ft *dialog);
	G_GNUC_INTERNAL struct v3270ft_entry * v3270ft_create_entry(void);



#endif // PRIVATE_H_INCLUDED
