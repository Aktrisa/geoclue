/*
 * Geoclue
 * geoclue-connectivity.c
 *
 * Author: Jussi Kukkonen <jku@o-hand.com>
 * Copyright 2007 by Garmin Ltd. or its subsidiaries
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include <config.h>

#include <glib.h>
#include "connectivity.h"

#ifdef HAVE_NETWORK_MANAGER
#include "connectivity-networkmanager.h"
#else
#ifdef HAVE_CONIC
#include "connectivity-conic.h"
#endif
#endif

enum {
	STATUS_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
geoclue_connectivity_base_init (gpointer klass)
{
	static gboolean initialized = FALSE;
	
	if (initialized) {
		return;
	}
	
	initialized = TRUE;
	signals[STATUS_CHANGED] = g_signal_new ("status-changed",
	                          G_OBJECT_CLASS_TYPE (klass),
	                          G_SIGNAL_RUN_LAST,
	                          G_STRUCT_OFFSET (GeoclueConnectivityInterface, 
	                                           status_changed),
	                          NULL, NULL,
	                          g_cclosure_marshal_VOID__INT,
	                          G_TYPE_NONE, 1, G_TYPE_INT);
}

GType
geoclue_connectivity_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		const GTypeInfo info = {
			sizeof (GeoclueConnectivityInterface),
			geoclue_connectivity_base_init,
			NULL,
		};
		
		type = g_type_register_static (G_TYPE_INTERFACE,
		                               "GeoclueConnectivity", 
		                               &info, 0);
	}
	
	return type;
}

GeoclueConnectivity *
geoclue_connectivity_new (void)
{
	GeoclueConnectivity *connectivity = NULL;

#ifdef HAVE_NETWORK_MANAGER
	connectivity = GEOCLUE_CONNECTIVITY (g_object_new (GEOCLUE_TYPE_NETWORKMANAGER, NULL));
#else
#ifdef HAVE_CONIC
	connectivity = GEOCLUE_CONNECTIVITY (g_object_new (GEOCLUE_TYPE_CONIC, NULL));
#endif
#endif
	return connectivity;
}

GeoclueNetworkStatus
geoclue_connectivity_get_status (GeoclueConnectivity *self)
{
	return GEOCLUE_CONNECTIVITY_GET_INTERFACE (self)->get_status (self);
}

char *
geoclue_connectivity_get_ap_mac (GeoclueConnectivity *self)
{
	if (GEOCLUE_CONNECTIVITY_GET_INTERFACE (self)->get_ap_mac != NULL)
		return GEOCLUE_CONNECTIVITY_GET_INTERFACE (self)->get_ap_mac (self);
	return NULL;
}

GHashTable *
geoclue_connectivity_get_aps (GeoclueConnectivity *self)
{
	char *ap;
	GHashTable *ht;

	if (GEOCLUE_CONNECTIVITY_GET_INTERFACE (self)->get_aps != NULL)
		return GEOCLUE_CONNECTIVITY_GET_INTERFACE (self)->get_aps (self);

	ap = geoclue_connectivity_get_ap_mac (self);
	if (ap == NULL)
		return NULL;
	ht = g_hash_table_new_full (g_str_hash, g_str_equal,
				    (GDestroyNotify) g_free, NULL);
	/* FIXME, remove hard-coded dBm value */
	g_hash_table_insert (ht, ap, GINT_TO_POINTER (-50));
	return NULL;
}

void
geoclue_connectivity_emit_status_changed (GeoclueConnectivity *self,
                                          GeoclueNetworkStatus status)
{
	g_signal_emit (self, signals[STATUS_CHANGED], 0, status);
}
