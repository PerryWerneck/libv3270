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
 * Este programa está nomeado como charset.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>

 #ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
	#include <lmcons.h>
 #endif // _WIN32

 #include <terminal.h>
 #include <v3270.h>
 #include <lib3270/charset.h>
 #include <lib3270/log.h>
 #include <v3270/dialogs.h>

 #define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Implement ]------------------------------------------------------------------------------------*/

 struct parse
 {
	char			* host;
	char	 		* display;
	unsigned long	  cgcsgid;
	size_t			  len;

	struct {
		unsigned short		ebc;
		unsigned short		iso;
		lib3270_remap_scope	scope;
		unsigned char		oneway;
	} map[256];

 };

 static unsigned short getChar(const gchar *id, GError **error) {

 	if(*error) {
		return 0;
	}

	unsigned short chr = lib3270_translate_char(id);

	if(!chr) {
		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't parse character value" ));
	}

	return chr;
 }

 static lib3270_remap_scope getRemapScope(const gchar *str, GError **error) {

 	static const char *text[] = { "CS_ONLY","FT_ONLY", "BOTH" };
 	int i;

 	if(!error)
	{
		for(i=0;i < (int) G_N_ELEMENTS(text);i++)
		{
			if(!g_ascii_strcasecmp(str,text[i]))
				return (lib3270_remap_scope) i;
		}

		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Invalid remap scope" ));
	}

	return BOTH;
 }

 static void element_start(G_GNUC_UNUSED GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct parse *info, GError **error)
 {
 	if(!g_ascii_strcasecmp(element_name,"pw3270-remap"))
	{
		const gchar *host		= NULL;
		const gchar *cgcsgid	= NULL;
		const gchar *display	= NULL;

		g_markup_collect_attributes(element_name,names,values,error,
								G_MARKUP_COLLECT_STRING, "host", &host,
								G_MARKUP_COLLECT_STRING, "cgcsgid", &cgcsgid,
								G_MARKUP_COLLECT_STRING, "display", &display,
								G_MARKUP_COLLECT_INVALID);
		if(*error)
		{
			return;
		}

		if(host)
		{
			g_free(info->host);
			info->host = g_strdup(host);
		}

		if(display)
		{
			g_free(info->display);
			info->display = g_strdup(display);
		}

		if(cgcsgid) {

			if(!g_str_has_prefix(cgcsgid,"0x"))
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Invalid cgcsgid value" ));
				return;
			}

			if(sscanf(cgcsgid + 2, "%lx", &info->cgcsgid) != 1) {
				*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't parse cgcsgid value" ));
				return;
			}

		}

	}
 	else if(!g_ascii_strcasecmp(element_name,"char"))
 	{
 		if(info->len >= G_N_ELEMENTS(info->map)) {
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Too many remaps" ));
			return;
 		}

		const gchar *ebc		= NULL;
		const gchar *iso		= NULL;
		const gchar *scope		= NULL;
		const gchar *oneway		= NULL;

		g_markup_collect_attributes(element_name,names,values,error,
								G_MARKUP_COLLECT_STRING, "ebc", &ebc,
								G_MARKUP_COLLECT_STRING, "iso", &iso,
								G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "scope", &scope,
								G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "one-way", &oneway,
								G_MARKUP_COLLECT_INVALID);

		if(*error)
		{
			return;
		}

		if(!scope)
		{
			scope = "both";
		}

		if(!oneway)
		{
			oneway = "no";
		}

		info->map[info->len].ebc 	= getChar(ebc,error);
		info->map[info->len].iso 	= getChar(iso,error);
		info->map[info->len].scope	= getRemapScope(scope,error);

 		info->len++;

 	}
 }


 static void element_end(G_GNUC_UNUSED GMarkupParseContext *context, G_GNUC_UNUSED const gchar *element_name, G_GNUC_UNUSED struct parse *info, G_GNUC_UNUSED GError **error)
 {
 }

 static gchar * get_filename(const gchar *name)
 {
 	size_t ix;
	const gchar * const * system_data_dirs = g_get_system_data_dirs();

#ifdef _WIN32
	{
		char wc_fn[MAX_PATH];

		if(GetModuleFileName(NULL, wc_fn, MAX_PATH))
		{
			char *p;

			if((p = strrchr(wc_fn, '\\')) != NULL)
				*p = '\0';

			if((p = strrchr(wc_fn, '/')) != NULL)
				*p = '\0';

			gchar *filename = g_build_filename(wc_fn,G_STRINGIFY(PRODUCT_NAME),"remap",name,NULL);

			debug("%s.win32=%s",__FUNCTION__,filename);
			if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
				return filename;

			g_free(filename);
		}

	}
#endif // _WIN32

	for(ix=0;system_data_dirs[ix];ix++)
	{
        gchar * filename = g_build_filename(system_data_dirs[ix],G_STRINGIFY(PRODUCT_NAME),"remap",name,NULL);

        debug("%s(%s)",__FUNCTION__,filename);

        if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			return filename;

		g_free(filename);
	}

	return g_build_filename("remap",name,NULL);

 }

 LIB3270_EXPORT	const char * v3270_get_remap_filename(GtkWidget *widget)
 {
 	return GTK_V3270(widget)->remap_filename;
 }

 LIB3270_EXPORT	void v3270_set_remap_filename(GtkWidget *widget, const gchar *path)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
				element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
				element_end,
//		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
		NULL,

//		(void (*)(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error))
		NULL,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

 	GError				* error		= NULL;
 	g_autofree gchar	* text 		= NULL;
	v3270 				* terminal = GTK_V3270(widget);
	struct parse		  cfg;

	debug("%s(%s)",__FUNCTION__,path);

	g_return_if_fail(GTK_IS_V3270(widget));

	if(terminal->remap_filename)
	{
		g_free(terminal->remap_filename);
		terminal->remap_filename = NULL;
	}

	if(!(path && *path))
	{
		debug("%s: No remap file, resetting charset",__FUNCTION__);
		lib3270_set_host_charset(terminal->host,NULL);
		return;
	}

	memset(&cfg,0,sizeof(cfg));

	if(g_file_test(path,G_FILE_TEST_IS_REGULAR))
	{
        terminal->remap_filename = g_strdup(path);
	}
	else if(g_str_has_suffix(path,".xml"))
	{
        terminal->remap_filename = get_filename(path);
	}
	else
	{
        g_autofree gchar *name = g_strconcat(path,".xml",NULL);
        terminal->remap_filename = get_filename(name);
	}

	debug("Loading \"%s\"",terminal->remap_filename);
	g_file_get_contents(terminal->remap_filename,&text,NULL,&error);

	if(text && !error)
	{
		GMarkupParseContext	* context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT|G_MARKUP_PREFIX_ERROR_POSITION,&cfg,NULL);
		g_markup_parse_context_parse(context,text,strlen(text),&error);
		g_markup_parse_context_free(context);
	}

	debug("error=%p",error);

	lib3270_reset_charset(terminal->host,NULL,NULL,LIB3270_DEFAULT_CGEN | LIB3270_DEFAULT_CSET);

	if(error)
	{
		v3270_popup_gerror(
				widget,
				error,
				_( "Remap Failed" ),
				_(  "Can't parse %s" ), path
		);

		g_error_free(error);

	} else {

		unsigned int i;

		for(i=0;i < cfg.len; i++)
		{
			lib3270_remap_char(terminal->host,cfg.map[i].ebc,cfg.map[i].iso, BOTH, 0);
		}

	}

	g_free(cfg.host);
	g_free(cfg.display);

	g_object_notify_by_pspec(G_OBJECT(widget), GTK_V3270_GET_CLASS(widget)->properties.remap_file);

 }

