/*
 * Copyright (c) 2013 Martin Stubbs <martinstubbs@gmail.com>
 *
 * Usbfttx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Usbfmtx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <math.h>

#include <libudev.h>
#include <fcntl.h>           /* open */
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>          /* close */
#include <linux/videodev2.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void FreqScrollBarChanged(int);
    void FreqSpinBoxChanged(double);
    void stereoChanged(bool);
    void muteChanged(bool);
    void audioChanged(int);
    void powerChanged(int);
    void preemphasisChanged(bool);

private:
    Ui::MainWindow *ui;

    void errorBox(QString const&);
    int sliderPos(int);
    int freqFromSliderPos(int);

    void lookForAdaptor();
    void openDev();
    void readDev();

};

#endif // MAINWINDOW_H
