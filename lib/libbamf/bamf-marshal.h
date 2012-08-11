
#ifndef ___bamf_marshal_MARSHAL_H__
#define ___bamf_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,STRING (./bamf-marshal.list:20) */
extern void _bamf_marshal_VOID__STRING_STRING (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:STRING,STRING,STRING (./bamf-marshal.list:21) */
extern void _bamf_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* VOID:OBJECT,OBJECT (./bamf-marshal.list:22) */
extern void _bamf_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:INT,INT (./bamf-marshal.list:23) */
extern void _bamf_marshal_VOID__INT_INT (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

/* VOID:STRING,BOXED,POINTER (./bamf-marshal.list:24) */
extern void _bamf_marshal_VOID__STRING_BOXED_POINTER (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

G_END_DECLS

#endif /* ___bamf_marshal_MARSHAL_H__ */

