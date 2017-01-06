/*
 * Goodvibes Radio Player
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

#include <glib.h>
#include <glib-object.h>

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/gv-feature.h"
#include "framework/gv-framework.h"
#include "framework/gv-framework-enum-types.h"
#include "framework/gv-list.h"
#include "framework/gv-param-specs.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_STATE,
	PROP_ENABLED,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _GvFeaturePrivate {
	/* Properties */
	GvFeatureState state;
	gboolean        enabled;
	/* Pending enable/disable task */
	guint           when_idle_id;
};

typedef struct _GvFeaturePrivate GvFeaturePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(GvFeature, gv_feature, G_TYPE_OBJECT)

/*
 * Private methods
 */

static void gv_feature_set_state(GvFeature *self, GvFeatureState state);

static void
gv_feature_disable(GvFeature *self)
{
	GvFeatureClass *feature_class = GV_FEATURE_GET_CLASS(self);

	INFO("Disabling feature '%s'...", G_OBJECT_TYPE_NAME(self));

	/* Disable */
	if (feature_class->disable)
		feature_class->disable(self);

	/* Set state property */
	gv_feature_set_state(self, GV_FEATURE_STATE_DISABLED);
}

static void
gv_feature_enable(GvFeature *self)
{
	GvFeatureClass *feature_class = GV_FEATURE_GET_CLASS(self);

	INFO("Enabling feature '%s'...", G_OBJECT_TYPE_NAME(self));

	/* Enable */
	if (feature_class->enable)
		feature_class->enable(self);

	/* Set state property */
	gv_feature_set_state(self, GV_FEATURE_STATE_ENABLED);
}

/*
 * Callbacks
 */

static gboolean
when_idle_enable_feature(gpointer user_data)
{
	GvFeature *self = GV_FEATURE(user_data);
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	g_assert(priv->state == GV_FEATURE_STATE_ENABLING);

	gv_feature_enable(self);

	priv->when_idle_id = 0;

	return G_SOURCE_REMOVE;
}

static gboolean
when_idle_disable_feature(gpointer user_data)
{
	GvFeature *self = GV_FEATURE(user_data);
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	g_assert(priv->state == GV_FEATURE_STATE_DISABLING);

	gv_feature_disable(self);

	priv->when_idle_id = 0;

	return G_SOURCE_REMOVE;
}

/*
 * Property accessors
 */

GvFeatureState
gv_feature_get_state(GvFeature *self)
{
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	return priv->state;
}

static void
gv_feature_set_state(GvFeature *self, GvFeatureState state)
{
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	if (priv->state == state)
		return;

	priv->state = state;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
}

gboolean
gv_feature_get_enabled(GvFeature *self)
{
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	return priv->enabled;
}

void
gv_feature_set_enabled(GvFeature *self, gboolean enabled)
{
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);
	GvFeatureState new_state;

	/* Bail out if needed */
	if (priv->enabled == enabled)
		return;

	/* Check if we really need to do something.
	 *
	 * Enabling and disabling the feature is not done immediately,
	 * but scheduled for a later idle moment. There are several
	 * reasons for that.
	 *
	 * At first, features might be enabled very early at init time:
	 * - for features that are enabled by default, this code will be
	 *   run just after the object is created.
	 * - for features that are enabled by the used, this code will be
	 *   run when the configuration is applied.
	 * In both cases, it's too early to run the feature enable code.
	 *
	 * Moreover, if a feature is enabled by default, and disabled in the
	 * configuration, this code will be invoked twice successively, first
	 * to enable, then to disable. Thanks to the delay, in this particular
	 * case the two calls cancel each other, and no action is performed.
	 *
	 * So, enabling/disabling is a delayed operation, and therefore we
	 * have two properties: one for the setting, one for the state.
	 *
	 * In here, we just set the setting, and schedule a callback if needed.
	 *
	 * Notice that if the state is *ING, it just means that an operation
	 * is pending, but nothing was done yet. In such case, it would be an
	 * error to schedule a callback. The only thing to do is to remove the
	 * pending callback, and to change the state.
	 */

	new_state = priv->state;

	if (enabled == TRUE) {
		switch (priv->state) {
		case GV_FEATURE_STATE_DISABLING:
			g_source_remove(priv->when_idle_id);
			priv->when_idle_id = 0;
			new_state = GV_FEATURE_STATE_ENABLED;
			break;

		case GV_FEATURE_STATE_DISABLED:
			priv->when_idle_id = g_idle_add(when_idle_enable_feature, self);
			new_state = GV_FEATURE_STATE_ENABLING;
			break;

		default:
			/* We shouldn't pass here */
			WARNING("Unexpected state value %d", priv->state);
			break;
		}

	} else {
		switch (priv->state) {
		case GV_FEATURE_STATE_ENABLING:
			g_source_remove(priv->when_idle_id);
			priv->when_idle_id = 0;
			new_state = GV_FEATURE_STATE_DISABLED;
			break;

		case GV_FEATURE_STATE_ENABLED:
			priv->when_idle_id = g_idle_add(when_idle_disable_feature, self);
			new_state = GV_FEATURE_STATE_DISABLING;
			break;

		default:
			/* We shouldn't pass here */
			WARNING("Unexpected state value %d", priv->state);
			break;
		}
	}

	/* Save and notify */
	priv->enabled = enabled;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ENABLED]);

	gv_feature_set_state(self, new_state);
}

static void
gv_feature_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	GvFeature *self = GV_FEATURE(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATE:
		g_value_set_enum(value, gv_feature_get_state(self));
		break;
	case PROP_ENABLED:
		g_value_set_boolean(value, gv_feature_get_enabled(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
gv_feature_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	GvFeature *self = GV_FEATURE(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_ENABLED:
		gv_feature_set_enabled(self, g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public functions
 */

GvFeature *
gv_feature_new(GType object_type, gboolean enabled)
{
	return g_object_new(object_type,
	                    "enabled", enabled,
	                    NULL);
}

/*
 * GObject methods
 */

static void
gv_feature_finalize(GObject *object)
{
	GvFeature *self = GV_FEATURE(object);
	GvFeaturePrivate *priv = gv_feature_get_instance_private(self);

	TRACE("%p", object);

	/* Unload */
	if (priv->enabled)
		gv_feature_set_enabled(self, FALSE);

	/* There might be some task pending */
	if (priv->when_idle_id > 0) {
		if (priv->state == GV_FEATURE_STATE_DISABLING)
			when_idle_disable_feature(self);
		else
			g_source_remove(priv->when_idle_id);
	}

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_feature, object);
}

static void
gv_feature_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_feature, object);
}

static void
gv_feature_init(GvFeature *self)
{
	TRACE("%p", self);
}

static void
gv_feature_class_init(GvFeatureClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_feature_finalize;
	object_class->constructed = gv_feature_constructed;

	/* Properties */
	object_class->get_property = gv_feature_get_property;
	object_class->set_property = gv_feature_set_property;

	properties[PROP_STATE] =
	        g_param_spec_enum("state", "Feature State", NULL,
	                          GV_FEATURE_STATE_ENUM_TYPE,
	                          GV_FEATURE_STATE_DISABLED,
	                          GV_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_ENABLED] =
	        g_param_spec_boolean("enabled", "Enabled", NULL,
	                             FALSE,
	                             GV_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}