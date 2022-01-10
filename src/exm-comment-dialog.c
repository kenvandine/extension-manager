#include "exm-comment-dialog.h"

#include "web/exm-comment-provider.h"
#include "web/model/exm-comment.h"

struct _ExmCommentDialog
{
    GtkDialog parent_instance;

    ExmCommentProvider *comment_provider;

    GtkListBox *list_box;

    int web_id;
};

G_DEFINE_FINAL_TYPE (ExmCommentDialog, exm_comment_dialog, GTK_TYPE_DIALOG)

enum {
    PROP_0,
    PROP_WEB_ID,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void exm_comment_dialog_constructed (GObject *object);

ExmCommentDialog *
exm_comment_dialog_new (int web_id)
{
    return g_object_new (EXM_TYPE_COMMENT_DIALOG,
                         "web-id", web_id,
                         NULL);
}

static void
exm_comment_dialog_finalize (GObject *object)
{
    ExmCommentDialog *self = (ExmCommentDialog *)object;

    G_OBJECT_CLASS (exm_comment_dialog_parent_class)->finalize (object);
}

static void
exm_comment_dialog_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    ExmCommentDialog *self = EXM_COMMENT_DIALOG (object);

    switch (prop_id)
    {
    case PROP_WEB_ID:
        g_value_set_int (value, self->web_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
exm_comment_dialog_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    ExmCommentDialog *self = EXM_COMMENT_DIALOG (object);

    switch (prop_id)
    {
    case PROP_WEB_ID:
        self->web_id = g_value_get_int (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
exm_comment_dialog_class_init (ExmCommentDialogClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = exm_comment_dialog_finalize;
    object_class->get_property = exm_comment_dialog_get_property;
    object_class->set_property = exm_comment_dialog_set_property;
    object_class->constructed = exm_comment_dialog_constructed;

    properties [PROP_WEB_ID]
        = g_param_spec_int ("web-id",
                            "Web ID",
                            "Web ID",
                            0, G_MAXINT, 0,
                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/mattjakeman/ExtensionManager/exm-comment-dialog.ui");
    // gtk_widget_class_bind_template_child (widget_class, ExmWindow, header_bar);
    gtk_widget_class_bind_template_child (widget_class, ExmCommentDialog, list_box);
}

static GtkWidget *
comment_factory (ExmComment* comment)
{
    GtkWidget *box;
    GtkWidget *comment_label;

    gchar *text;
    g_object_get (comment,
                  "comment", &text,
                  NULL);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class (box, "content");

    comment_label = gtk_label_new (text);
    gtk_label_set_xalign (GTK_LABEL (comment_label), 0);
    gtk_label_set_wrap (GTK_LABEL (comment_label), GTK_WRAP_WORD);
    gtk_widget_add_css_class (comment_label, "description");
    gtk_box_append (GTK_BOX (box), comment_label);

    return box;
}

static void
on_get_comments (GObject          *source,
                 GAsyncResult     *res,
                 ExmCommentDialog *self)
{
    GError *error = NULL;

    GListModel *model = exm_comment_provider_get_comments_finish (EXM_COMMENT_PROVIDER (source), res, &error);

    // TODO: Can we make this less verbose? Maybe use UI/BLP files?
    gtk_list_box_bind_model (self->list_box, model,
                             (GtkListBoxCreateWidgetFunc) comment_factory,
                             g_object_ref (self), g_object_unref);
}

static void
exm_comment_dialog_constructed (GObject *object)
{
    ExmCommentDialog *self = EXM_COMMENT_DIALOG (object);

    exm_comment_provider_get_comments_async (self->comment_provider,
                                             19,
                                             NULL,
                                             (GAsyncReadyCallback) on_get_comments,
                                             self);
}

static void
exm_comment_dialog_init (ExmCommentDialog *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_CLOSE);

    self->comment_provider = exm_comment_provider_new ();
}