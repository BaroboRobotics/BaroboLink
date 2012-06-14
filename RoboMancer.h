#ifndef _ROBOMANCER_H_
#define _ROBOMANCER_H_

#include <gtk/gtk.h>
#include "RobotManager.h"

#define PLAT_GTK 1
#define GTK
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include <math.h>
#define RAD2DEG(x) ((x)*180.0/M_PI)
#define DEG2RAD(x) ((x)*M_PI/180.0)

#ifdef __cplusplus
extern "C" {
#endif

/* Global Funcs */
void initialize();
int getIterModelFromTreeSelection(GtkTreeView *treeView, GtkTreeModel **model, GtkTreeIter *iter);

/* Global Vars */
extern GtkBuilder *g_builder;
extern CRobotManager *g_robotManager;
extern GtkWidget *g_scieditor;
extern GtkWidget *g_window;
extern ScintillaObject *g_sci;

/* Connect Dialog */
G_MODULE_EXPORT void on_button_connect_addRobot_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_remove_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveUpAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_moveDownAvailable_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_connect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connect_disconnect_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_button_connectFailedOk_clicked(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_liststore_availableRobots_row_deleted(
    GtkTreeModel* model,
    GtkTreePath* path,
    gpointer user_data);
G_MODULE_EXPORT void on_liststore_availableRobots_row_inserted(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data);
void refreshConnectDialog();
int getConnectSelectedIndex();

/* Program Dialog */
G_MODULE_EXPORT void on_imagemenuitem_cut_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_copy_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_paste_activate(GtkWidget* widget, gpointer data);
G_MODULE_EXPORT void on_imagemenuitem_open_activate(GtkWidget* widget, gpointer data);

/* Control Dialog */
G_MODULE_EXPORT void on_button_motor1back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4back_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor1stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4stop_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor1forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor2forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor3forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT void on_button_motor4forward_clicked(GtkWidget*w, gpointer data); 
G_MODULE_EXPORT gboolean on_vscale_motorPos1_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos2_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos3_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos4_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos1_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos2_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos3_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorPos4_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);

G_MODULE_EXPORT gboolean on_vscale_motorspeed1_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed2_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed3_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed4_button_press_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed1_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed2_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed3_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT gboolean on_vscale_motorspeed4_button_release_event(GtkWidget*w, GdkEvent*event, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_combobox_connectedRobots_changed(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_forward_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_rotateLeft_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_stop_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_rotateRight_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_backward_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_moveToZero_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_move_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_moveTo_clicked(GtkWidget* w, gpointer data);
G_MODULE_EXPORT void on_button_playGait_clicked(GtkWidget* w, gpointer data);
void initControlDialog(void);
void* controllerHandlerThread(void* arg);

/* Teaching Dialog */
G_MODULE_EXPORT void on_button_clearRecordedPositions_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_recordPos_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_addDelay_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_deleteRecordedPos_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_saveToProgram_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_playRecorded_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_button_stopRecorded_clicked(GtkWidget*w, gpointer data);
G_MODULE_EXPORT void on_notebook1_switch_page(GtkNotebook* notebook, gpointer page, guint page_num, gpointer userdata);
G_MODULE_EXPORT void on_liststore_recordedMotions_rows_reordered(
    GtkTreeModel* model, 
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer new_order,
    gpointer user_data);
G_MODULE_EXPORT void on_cellrenderertext_recordedMotionName_edited(
    GtkCellRendererText* renderer,
    gchar* path,
    gchar* new_text,
    gpointer user_data);
G_MODULE_EXPORT void on_liststore_recordedMotions_row_deleted(
    GtkTreeModel* model,
    GtkTreePath* path,
    gpointer user_data);
G_MODULE_EXPORT void on_liststore_recordedMotions_row_inserted(
    GtkTreeModel* model,
    GtkTreePath* path,
    GtkTreeIter* iter,
    gpointer user_data);

#ifdef __cplusplus
}
#endif



#endif