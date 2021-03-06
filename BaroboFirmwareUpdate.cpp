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

#include "split.hpp"
#include "arraylen.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cassert>
#include <gtk/gtk.h>
#define PLAT_GTK 1
#define GTK
#ifdef __MACH__
#include <sys/types.h>
#include <unistd.h>
#include <gtk-mac-integration.h>
#endif
#include <sys/stat.h>
#include <mobot.h>
#include <libstkcomms.hpp>
#include "BaroboFirmwareUpdate.h"
#include "thread_macros.h"

#include <string>

#define MAX_THREADS 40

#ifdef __MACH__
#undef strdup
#endif

typedef enum dongleSearchStatus_e
{
  DONGLE_NIL,
  DONGLE_FOUND,
  DONGLE_NOTFOUND,
  DONGLE_SEARCHING,
} dongleSearchStatus_t;

GtkBuilder *g_builder;
GtkWidget *g_window;
dongleSearchStatus_t g_dongleSearchStatus = DONGLE_NIL;
MUTEX_T g_giant_lock;
COND_T g_giant_cond;
THREAD_T g_mainSearchThread;

mobot_t g_mobot;

int g_numThreads = 0;

char g_comport[80];
CStkComms *g_stkComms;

const char *g_interfaceFiles[] = {
  "interface/mobotfirmwareupdateinterface.glade",
  "mobotfirmwareupdateinterface.glade ",
  "../share/BaroboLink/mobotfirmwareupdateinterface.glade"
};

std::string g_hexfilename;

#if 0
void* findDongleWorkerThread(void* arg)
{
  /* The argument is a pointer to an int */
  int num = *(int*)arg;
  char buf[80];
  mobot_t* mobot;
  int rc;
#if defined (_WIN32) or defined (_MSYS)
  sprintf(buf, "\\\\.\\COM%d", num);
#else
  sprintf(buf, "/dev/ttyACM%d", num);
#endif
  /* Try to connect to it */
  mobot = (mobot_t*)malloc(sizeof(mobot_t));
  Mobot_init(mobot);
  rc = Mobot_connectWithTTY(mobot, buf);
  if(rc != 0) {
    rc = Mobot_connectWithTTYBaud(mobot, buf, 500000);
  }
  if(rc == 0) {
    /* We found the Mobot */
    MUTEX_LOCK(&g_giant_lock);
    /* Only update g_mobot pointer if no one else has updated it already */
    if(g_mobot == NULL) {
      g_mobot = mobot;
      g_dongleSearchStatus = DONGLE_FOUND;
      strcpy(g_comport, buf);
      COND_SIGNAL(&g_giant_cond);
      MUTEX_UNLOCK(&g_giant_lock);
    } else {
      MUTEX_UNLOCK(&g_giant_lock);
      Mobot_disconnect(mobot);
      free(mobot);
    }
    Mobot_getStatus(mobot);
  } else {
    free(mobot);
  }
  MUTEX_LOCK(&g_giant_lock);
  g_numThreads--;
  COND_SIGNAL(&g_giant_cond);
  MUTEX_UNLOCK(&g_giant_lock);
  return NULL;
}
#endif

void* findDongleThread(void* arg)
{
  g_dongleSearchStatus = DONGLE_SEARCHING;

  if (-1 == Mobot_dongleGetTTY(g_comport, ARRAYLEN(g_comport)) ||
      -1 == Mobot_connectWithTTY(&g_mobot, g_comport)) {
    g_dongleSearchStatus = DONGLE_NOTFOUND;
  }
  else {
    g_dongleSearchStatus = DONGLE_FOUND;
  }

  return NULL;
}

gboolean findDongleTimeout(gpointer data)
{
  char buf[80];
  /* Check to see if the dongle is found. If it is, proceed to the next page.
   * If not, reset state vars and stay on the first page, pop up a warning
   * dialog. */
  gboolean rc = FALSE;
  MUTEX_LOCK(&g_giant_lock);
  if(g_dongleSearchStatus == DONGLE_SEARCHING) {
    rc = TRUE;
  } else if (g_dongleSearchStatus == DONGLE_FOUND) {
    /* Set up the labels and stuff */
    switch(g_mobot.formFactor) {
      case MOBOTFORM_I:
        sprintf(buf, "Linkbot-I");
        break;
      case MOBOTFORM_L:
        sprintf(buf, "Linkbot-L");
        break;
      default:
        sprintf(buf, "Unkown");
        break;
    }
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_formFactor")),
        buf);
    sprintf(buf, "%d", Mobot_getVersion(&g_mobot));
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_firmwareVersion")),
        buf);
    sprintf(buf, "%d", Mobot_protocolVersion());
    gtk_label_set_text(
        GTK_LABEL(gtk_builder_get_object(g_builder, "label_upgradeVersion")),
        buf);
    gtk_entry_set_text(
        GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_serialID")),
        g_mobot.serialID);
    /* Go to next notebook page */
    gtk_notebook_next_page(
        GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
    rc = FALSE;
  } else if (g_dongleSearchStatus == DONGLE_NOTFOUND) {
    /* Renable the button */
    gtk_widget_set_sensitive(
        GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p1_next")),
        TRUE);
    /* Pop up a dialog box */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_OK,
        "No attached Linkbot was detected. Please make sure there is a Linkbot connected to the computer with a Micro-USB cable and that the Linkbot is powered on.");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_hide(GTK_WIDGET(d));
    rc = FALSE;
  } else {
    /* Something very weird happened */
    fprintf(stderr, "Fatal error: %s:%d\n", __FILE__, __LINE__);
    exit(-1);
  }
  MUTEX_UNLOCK(&g_giant_lock);
  return rc;
}

int main(int argc, char* argv[])
{
  GError *error = NULL;

#ifdef _WIN32
  /* Make sure there isn't another instance of BaroboLink running by checking
   * for the existance of a named mutex. */
  HANDLE hMutex;
  hMutex = CreateMutex(NULL, TRUE, TEXT("Global\\RoboMancerMutex"));
  DWORD dwerror = GetLastError();
#endif

 
  gtk_init(&argc, &argv);

  /* Create the GTK Builder */
  g_builder = gtk_builder_new();

#ifdef _WIN32
  if(dwerror == ERROR_ALREADY_EXISTS) {
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "An instance of BaroboLink is already running. Please terminate BaroboLink before running the Barobo Firmware Update Utility.");
    int rc = gtk_dialog_run(GTK_DIALOG(d));
    exit(0);
  }
#endif

  std::vector<std::string> interfaceFiles
    (g_interfaceFiles, g_interfaceFiles + ARRAYLEN(g_interfaceFiles));

  /* hlh: This used to be ifdef __MACH__, but XDG is not a BSD-specific platform. */
#ifndef _WIN32
  std::string datadir (getenv("XDG_DATA_DIRS"));

  if(!datadir.empty()) {
    std::vector<std::string> xdg_data_dirs = split_escaped(datadir, ':', '\\');
    for (std::vector<std::string>::iterator it = xdg_data_dirs.begin();
        xdg_data_dirs.end() != it; ++it) {
      interfaceFiles.push_back(*it + std::string("/BaroboLink/interface.glade"));
    }
  }
  else {
    interfaceFiles.push_back(std::string("/usr/share/BaroboLink/interface.glade"));
  }
#endif

  /* Load the UI */
  /* Find ther interface file */
  struct stat s;
  int err;
  bool iface_file_found = false;
  for (std::vector<std::string>::iterator it = interfaceFiles.begin();
      interfaceFiles.end() != it; ++it) {
    err = stat(it->c_str(), &s);
    if(err == 0) {
      if( ! gtk_builder_add_from_file(g_builder, it->c_str(), &error) )
      {
        g_warning("%s", error->message);
        //g_free(error);
        return -1;
      } else {
        iface_file_found = true;
        break;
      }
    }
  }

  if (!iface_file_found) {
    /* Could not find the interface file */
    g_warning("Could not find interface file.");
    return -1;
  }

  Mobot_init(&g_mobot);

  /* Get the main window */
  g_window = GTK_WIDGET( gtk_builder_get_object(g_builder, "window1"));
  /* Connect signals */
  gtk_builder_connect_signals(g_builder, NULL);

#ifdef __MACH__
  //g_signal_connect(GtkOSXMacmenu, "NSApplicationBlockTermination",
      //G_CALLBACK(app_should_quit_cb), NULL);
  GtkWidget* quititem = GTK_WIDGET(gtk_builder_get_object(g_builder, "imagemenuitem5"));
  //gtk_mac_menu_set_quit_menu_item(GTK_MENU_ITEM(quititem));
#endif

  /* Show the window */
  gtk_widget_show(g_window);
  gtk_main();
  return 0;
}

void on_button_p1_next_clicked(GtkWidget* widget, gpointer data)
{
  /* First, disable the widget so it cannot be clicked again */
  gtk_widget_set_sensitive(widget, false);

  /* Make sure thread is joined */
  /* hlh: pthread_join(NULL, ...) is undefined behavior, we need some way of
   * making sure that the thread is actually running... */
  if (DONGLE_NIL != g_dongleSearchStatus) {
    THREAD_JOIN(g_mainSearchThread);
  }

  /* Reset state vars */
  g_dongleSearchStatus = DONGLE_SEARCHING;
  
  /* Start the main search thread */
  THREAD_CREATE(&g_mainSearchThread, findDongleThread, NULL);

  /* Start the search timeout */
  g_timeout_add(200, findDongleTimeout, NULL);
}

gboolean programming_progress_timeout(gpointer data)
{
  /* Set the progress bar */
  gtk_progress_bar_set_fraction(
      GTK_PROGRESS_BAR(gtk_builder_get_object(g_builder, "progressbar1")),
      g_stkComms->getProgress());
  /* See if programming is completed */
  if(g_stkComms->isProgramComplete()) {
    g_stkComms->disconnect();
    /* Switch to next page */
    gtk_notebook_next_page(
        GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
  } else {
    return TRUE;
  }
}

gboolean switch_to_p3_timeout(gpointer data)
{
  /* Renable the button */
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p2_yes")),
      true);
  /* Switch to next page */
  gtk_notebook_next_page(
      GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")));
  g_stkComms = new CStkComms();
  g_stkComms->connectWithTTY(g_comport);
  g_stkComms->programAllAsync(g_hexfilename.c_str());
#ifndef _WIN32
  sleep(2);
#else
  Sleep(2000);
#endif
  /* Start the programming progress timeout */
  g_timeout_add(500, programming_progress_timeout, NULL);
  return FALSE;
}

int fileExists(const char* filename)
{
  struct stat s;
  return (stat(filename, &s) == 0);
}

void on_button_p2_yes_clicked(GtkWidget* widget, gpointer data)
{
  int i;
  gtk_widget_set_sensitive(widget, false);
  /* First, reprogram the serial ID */
  const char* text;
  char buf[5];
  text = gtk_entry_get_text(
      GTK_ENTRY(gtk_builder_get_object(g_builder, "entry_serialID")));
  if(strlen(text)==4) {
    for(i = 0; i < 5; i++) {
      buf[i] = toupper(text[i]);
    }
    if(strcmp(g_mobot.serialID, buf)) {
      Mobot_setID(&g_mobot, buf);
    }
  } else {
    /* Pop up warning dialog */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "The Serial ID does not have a valid format. The serial ID should consist of four alpha-numeric characters.");
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_hide(d);
    gtk_widget_set_sensitive(widget, true);
    return;
  }

#ifndef _WIN32
  g_hexfilename = std::string("hexfiles/linkbot_latest.hex");
#else
  /* Get the install path of BaroboLink from the registry */
  DWORD size;
  HKEY key;
  int rc;

  rc = RegOpenKeyEx(
      HKEY_LOCAL_MACHINE,
      "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\BaroboLink.exe",
      0,
      KEY_QUERY_VALUE,
      &key);

  if (ERROR_SUCCESS != rc) {
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
	"Unable to find BaroboLink location in registry.\nTry re-installing BaroboLink.");
    gtk_dialog_run(GTK_DIALOG(d));
    return;
  }

  /* Find out how much memory to allocate. */
  rc = RegQueryValueEx(
      key,
      "PATH",
      NULL,
      NULL,
      NULL,
      &size);
  assert(ERROR_SUCCESS == rc);

  /* hlh: FIXME this should probably be TCHAR instead, and we should support
   * unicode or whatever */
  char* path = new char [size + 1];

  rc = RegQueryValueEx(
      key,
      "PATH",
      NULL,
      NULL,
      (LPBYTE)path,
      &size);
  assert(ERROR_SUCCESS == rc);

  path[size] = '\0';

  g_hexfilename = std::string(path) + "\\hexfiles\\linkbot_latest.hex";
  delete [] path;
  path = NULL;
#endif
 
#if 0 
  /* Make sure a file is selected and exists */
  g_hexfilename = gtk_file_chooser_get_filename(
      GTK_FILE_CHOOSER(gtk_builder_get_object(g_builder, "filechooserbutton_hexfile")));
#endif
  if(!fileExists(g_hexfilename.c_str())) {
    /* Pop up a warning dialog and abort */
    GtkWidget* d = gtk_message_dialog_new(
        GTK_WINDOW(gtk_builder_get_object(g_builder, "window1")),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "File \"%s\" does not exist. \nPlease select a valid hex file to flash to the Linkbot.",
        g_hexfilename.c_str());
    int rc = gtk_dialog_run(GTK_DIALOG(d));
    return;
  }

  /* Next, reboot the module and go on to the next page */
  Mobot_reboot(&g_mobot);
  Mobot_disconnect(&g_mobot);
  //free(g_mobot);
  //g_mobot = NULL;
  g_timeout_add(3000, switch_to_p3_timeout, NULL);
}

void on_button_flashAnother_clicked(GtkWidget* widget, gpointer data)
{
  gtk_widget_set_sensitive(
      GTK_WIDGET(gtk_builder_get_object(g_builder, "button_p1_next")),
      true);
  gtk_notebook_set_current_page(
      GTK_NOTEBOOK(gtk_builder_get_object(g_builder, "notebook1")),
      0);
}

