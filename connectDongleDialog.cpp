/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   BaroboLink is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BaroboLink is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BaroboLink.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#endif
#include "mobot.h"
#include "BaroboLink.h"
#include "thread_macros.h"

#define MAX_COMPORT 256
#define MAX_THREADS 10

/* Multithreaded function to try and find a dongle. Returns 0 on success */
int numThreadsRunning = 0;
MUTEX_T numThreadsRunning_lock;
COND_T numThreadsRunning_cond;
int foundDongle = 0;
int foundDonglePort = 0;
char *foundDongleDevice = NULL;
MUTEX_T foundDongle_lock;
COND_T foundDongle_cond;
recordMobot_t* g_dongle;

int findDongle(void)
{
  char serialDevice[256];
  if (-1 == Mobot_dongleGetTTY(serialDevice, sizeof(serialDevice))) {
    return -1;
  }
  
  /* Try to connect to the port */
  recordMobot_t* mobot = (recordMobot_t*)malloc(sizeof(recordMobot_t));
  RecordMobot_init(mobot, "Dongle");
  if(!Mobot_connectWithTTY((mobot_t*)mobot, serialDevice)) {
    g_dongle = mobot;
  } else if (!Mobot_connectWithTTYBaud((mobot_t*)mobot, serialDevice, 500000)) {
    g_dongle = mobot;
  } else {
    Mobot_disconnect((mobot_t*)mobot);
    free(mobot);
    return -1;
  }
  g_mobotParent = g_dongle;
  /* We found the TTY port. */
  g_robotManager->addDongle(serialDevice);
  g_robotManager->write();
  Mobot_setDongleMobot((mobot_t*)g_mobotParent);
  /* Modify widgets in dongle dialog */
  GtkLabel *currentComPort = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectDongleCurrentPort"));
  gtk_label_set_text(currentComPort, serialDevice);
  GtkWidget *w;
  w = GTK_WIDGET(gtk_builder_get_object(g_builder, "image_dongleConnected"));
  gtk_image_set_from_stock(GTK_IMAGE(w), GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
  return 0;
}

void askConnectDongle(void)
{
  /* Before we ask, lets just see if we can find a connected dongle anywhere... */
  if(!findDongle()) {
    return;
  }

  /* Set up a question dialog */
  GtkWidget* d = gtk_message_dialog_new(
      GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK,
      "No attached dongle could be detected on this computer. Please check the following items to ensure the best connection to your Linkbot:\n"
      "- The Linkbot is turned on.\n"
      "- The robot's 4 digit serial ID has been entered correctly.\n"
      "- The device driver for the Linkbot has been installed.\n"
      "- The USB cable is firmly plugged into both the computer and the Linkbot.\n"
      "- The USB cable is in good working order.\n"
      "- If the previous five steps do not work, try restarting both the robot and BaroboLink.\n");
  int rc = gtk_dialog_run(GTK_DIALOG(d));
  gtk_widget_destroy(d);
}

void showConnectDongleDialog(void)
{
  GdkColor color;
  GtkWidget *w;
  /* First, see if the dongle is connected */
  if( (g_mobotParent != NULL) && (((mobot_t*)g_mobotParent)->connected != 0)) {
    /* The dongle is connected */
    /* Make the text entry green */
    gdk_color_parse("green", &color);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "image_dongleConnected"));
    gtk_image_set_from_stock(GTK_IMAGE(w), GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "label_connectDongleCurrentPort"));
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL, &color);
  } else {
    /* The dongle is not connected */
    gdk_color_parse("red", &color);
    w = GTK_WIDGET(gtk_builder_get_object(g_builder, "label_connectDongleCurrentPort"));
    gtk_widget_modify_fg(w, GTK_STATE_NORMAL, &color);
    gtk_entry_set_text(GTK_ENTRY(w), "<Not Connected>");
  }
  GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectDongle"));
  gtk_widget_show(window);
}

void hideConnectDongleDialog(void)
{
  GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectDongle"));
  gtk_widget_hide(window);
}

void on_button_connectDongleConnect_clicked(GtkWidget *widget, gpointer data)
{
  char buf[256];
  GtkWidget* w;
  GtkRadioButton *connectAutomaticallyButton = 
    GTK_RADIO_BUTTON(gtk_builder_get_object(g_builder, "radiobutton_connectDongleAuto"));
  GtkRadioButton *connectManuallyButton = 
    GTK_RADIO_BUTTON(gtk_builder_get_object(g_builder, "radiobutton_connectDongleManual"));
  GtkEntry *manualComPort = GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_connectDongleManual"));
  GtkLabel *currentComPort = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectDongleCurrentPort"));

  /* Check to see if the auto button is pressed */
  if(
      gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(connectAutomaticallyButton))
    )
  {
    /* If the auto-detect button is pressed */
    int i;
    bool dongleFound = false;
    for(i = 0; i < MAX_COMPORT; i++) {
#ifndef _WIN32
      sprintf(buf, "/dev/ttyACM%d", i);
#else
      sprintf(buf, "\\\\.\\COM%d", i);
#endif
      if(!Mobot_connectWithTTY((mobot_t*)g_mobotParent, buf)) {
        /* We found the TTY port. */
        gtk_label_set_text(currentComPort, buf);
        w = GTK_WIDGET(gtk_builder_get_object(g_builder, "image_dongleConnected"));
        gtk_image_set_from_stock(GTK_IMAGE(w), GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON);
        g_robotManager->addDongle(buf);
        g_robotManager->write();
        dongleFound = true;
        Mobot_setDongleMobot((mobot_t*)g_mobotParent);
        break;
      }
    }
    if(!dongleFound) {
      /* Display a warning/error dialog */
      GtkWidget* d = gtk_message_dialog_new(
          GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_WARNING,
          GTK_BUTTONS_CLOSE,
          "No dongle detected. Please make sure that a Linkbot is currently "
          "connected to the computer with a USB cable and turned on.");
      gtk_dialog_run(GTK_DIALOG(d));
      gtk_widget_destroy(d);
    }
  } else if (
      gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(connectManuallyButton))
      )
  {
    /* If the manual selection is pressed */
    /* Get the entry text */
    const char* port = gtk_entry_get_text(manualComPort);
    printf("Connecting to port %s\n", port);
    if(Mobot_connectWithTTY((mobot_t*)g_mobotParent, port)) {
      GtkLabel* errLabel = GTK_LABEL(gtk_builder_get_object(g_builder, "label_connectFailed"));
      sprintf(buf, "Error: Could not connect to dongle at %s.\n", port);
      gtk_label_set_text(errLabel, buf);
      GtkWidget* errWindow = GTK_WIDGET(gtk_builder_get_object(g_builder, "dialog_connectFailed"));
      gtk_widget_show(errWindow);
    } else {
      gtk_label_set_text(currentComPort, port);
      g_robotManager->addDongle(port);
      g_robotManager->write();
      Mobot_setDongleMobot((mobot_t*)g_mobotParent);
    }
  } else {
    /* Error */
    return;
  }
}

void on_button_connectDongleClose_clicked(GtkWidget *w, gpointer data)
{
  hideConnectDongleDialog();
  return;
}

void on_menuitem_DongleDialog_activate(GtkWidget *w, gpointer data)
{
  showConnectDongleDialog();
}
