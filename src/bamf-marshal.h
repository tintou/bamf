
#ifndef __bamf_marshal_MARSHAL_H__
#define __bamf_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:BOOL,STRING,STRING (./bamf-marshal.list:20) */
extern void bamf_marshal_VOID__BOOLEAN_STRING_STRING (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);
#define bamf_marshal_VOID__BOOL_STRING_STRING	bamf_marshal_VOID__BOOLEAN_STRING_STRING

/* VOID:STRING,STRING (./bamf-marshal.list:21) */
extern void bamf_marshal_VOID__STRING_STRING (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

/* VOID:STRING,STRING,STRING (./bamf-marshal.list:22) */
extern void bamf_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* VOID:OBJECT,OBJECT (./bamf-marshal.list:23) */
extern void bamf_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

/* VOID:STRING,INT,STRING,STRING,STRING (./bamf-marshal.list:24) */
extern void bamf_marshal_VOID__STRING_INT_STRING_STRING_STRING (GClosure     *closure,
                                                                GValue       *return_value,
                                                                guint         n_param_values,
                                                                const GValue *param_values,
                                                                gpointer      invocation_hint,
                                                                gpointer      marshal_data);

G_END_DECLS

#endif /* __bamf_marshal_MARSHAL_H__ */

