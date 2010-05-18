#ifndef __BAMF_TAB_SOURCE_H__
#define __BAMF_TAB_SOURCE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define BAMF_TYPE_TAB_SOURCE		(bamf_tab_source_get_type ())
#define BAMF_TAB_SOURCE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSource))
#define BAMF_TAB_SOURCE_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSource const))
#define BAMF_TAB_SOURCE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))
#define BAMF_IS_TAB_SOURCE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAMF_TYPE_TAB_SOURCE))
#define BAMF_IS_TAB_SOURCE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), BAMF_TYPE_TAB_SOURCE))
#define BAMF_TAB_SOURCE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), BAMF_TYPE_TAB_SOURCE, BamfTabSourceClass))

typedef struct _BamfTabSource		BamfTabSource;
typedef struct _BamfTabSourceClass	BamfTabSourceClass;
typedef struct _BamfTabSourcePrivate	BamfTabSourcePrivate;

struct _BamfTabSource {
	GObject parent;
	
	BamfTabSourcePrivate *priv;
};

struct _BamfTabSourceClass {
	GObjectClass parent_class;
	
	/*< methods >*/
	void      (*show_tab)      (BamfTabSource *source, char *tab_id);
	char   ** (*tab_ids)       (BamfTabSource *source);
	GArray  * (*tab_preview)   (BamfTabSource *source, char *tab_id);
	char    * (*tab_uri)       (BamfTabSource *source, char *tab_id);
	guint32   (*tab_xid)       (BamfTabSource *source, char *tab_id);
};

GType bamf_tab_source_get_type (void) G_GNUC_CONST;

void           bamf_tab_source_show_tab        (BamfTabSource *source, 
                                                char *tab_id);

char        ** bamf_tab_source_get_tab_ids     (BamfTabSource *source);

GArray       * bamf_tab_source_get_tab_preview (BamfTabSource *source,
                                                char *tab_id);

char         * bamf_tab_source_get_tab_uri     (BamfTabSource *source,
                                                char *tab_id);

guint32        bamf_tab_source_get_tab_xid     (BamfTabSource *source,
                                                char *tab_id);

G_END_DECLS

#endif /* __BAMF_TAB_SOURCE_H__ */
