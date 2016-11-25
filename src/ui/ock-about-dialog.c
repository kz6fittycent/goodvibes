/*
 * Overcooked Radio Player
 *
 * Copyright (C) 2015-2016 Arnaud Rebillout
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include "core/ock-core.h"
#include "ui/ock-ui.h"

static const gchar *authors[] = {
	PACKAGE_AUTHOR_NAME " " PACKAGE_AUTHOR_EMAIL,
	NULL
};

void
ock_show_about_dialog(GtkWindow *parent)
{
	// WISHED "license-type" shouldn't be hardcoded

	gchar *comments;

	comments = g_strdup_printf("Audio Backend: %s\n"
	                           "GUI Toolkit: %s",
	                           ock_core_audio_backend_runtime_version_string(),
	                           ock_ui_toolkit_runtime_version_string());

	gtk_show_about_dialog(parent,
	                      "authors", authors,
	                      "comments", comments,
	                      "copyright", "(C) 2015-2016 " PACKAGE_AUTHOR_NAME,
	                      "license-type", GTK_LICENSE_GPL_3_0,
	                      "logo-icon-name", PACKAGE_NAME,
	                      "program-name", PACKAGE_LONG_NAME,
	                      "version", PACKAGE_VERSION,
	                      "website", PACKAGE_WEBSITE,
	                      NULL);

	g_free(comments);
}
