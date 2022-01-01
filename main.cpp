/*
 * Copyright (C) 2021 by LolikoX
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <QApplication>
#include <QMessageBox>

#include "qtxcord.h"
#include "mainwindow.h"
#include "xdiscord.h"

void xcord_exit();
void alert_discord_message_callback(char* message);

int main(int argc, char *argv[])
{
    /* simple command line handling / arg parsing */
    if (argc>1) {
        if (argv[1][0] == '-') { // trying to pass a flag
            if (!strncmp(argv[1], "-v", 2)
             || !strncmp(argv[1], "--version", 9)) {
                puts(VERSION_STRING);
                return EXIT_SUCCESS;
            } else {
                printf("USAGE: %s -v\n", argv[0]);
                return EXIT_FAILURE;
            }
        }
    }

    /* launch the Qt Application */
    XD_set_message_cb(alert_discord_message_callback);
    QApplication a(argc, argv);
    {
        XD_InitError ret = XD_init();
        atexit(xcord_exit);
        if (ret != XD_INIT_SUCCESS) {
            char err[64] = "";
            std::sprintf(err, "Failed to initialize Discord interface.\nCODE: %i", ret);
            QMessageBox::critical(NULL, "Error", err);
            return EXIT_FAILURE;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}

void aboutQtXcord()
{
    QMessageBox::about(NULL, "About QtXcord", VERSION_STRING);
}

void xcord_exit() {
    XD_quit();
}

void alert_discord_message_callback(char* message) {
    QMessageBox::information(NULL, "QtXcord", message);
}
