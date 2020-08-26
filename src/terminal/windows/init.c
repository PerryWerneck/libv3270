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

 #include <config.h>

 #include <winsock2.h>
 #include <windows.h>

 #include <internals.h>
 #include <locale.h>
 #include <libintl.h>

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwcallpurpose, LPVOID G_GNUC_UNUSED(lpvResvd))
{
    debug("%s starts",__FUNCTION__);

    switch(dwcallpurpose)
    {
    case DLL_PROCESS_ATTACH:
		debug("%s: DLL_PROCESS_ATTACH",__FUNCTION__);
    	{
			char lpFilename[4096];

			memset(lpFilename,0,sizeof(lpFilename));
			DWORD szPath = GetModuleFileName(hInstance,lpFilename,4095);
			lpFilename[szPath] = 0;

			char * ptr = strrchr(lpFilename,'\\');
			if(ptr)
				ptr[1] = 0;

			strncat(lpFilename,"locale",4095);

			bindtextdomain(GETTEXT_PACKAGE,lpFilename);
			bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

			debug("bindtextdomain(%s,\"%s\"",GETTEXT_PACKAGE,lpFilename);
    	}
		break;

	case DLL_PROCESS_DETACH:
		debug("%s: DLL_PROCESS_DETACH",__FUNCTION__);
		break;

    }

    debug("%s ends",__FUNCTION__);
    return TRUE;
}
