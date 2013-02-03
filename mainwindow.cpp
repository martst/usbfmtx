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

/*
 * Some v4l2 examples taken from http://credentiality2.blogspot.co.uk/2010/04/v4l2-example.html
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

QString devPath;
int fd = -1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    connect(ui->FreqScrollBar, SIGNAL(valueChanged(int)), this, SLOT(FreqScrollBarChanged(int)));
    connect(ui->freqSpinBox, SIGNAL(valueChanged(double)), this, SLOT(FreqSpinBoxChanged(double)));
    connect(ui->stereoCheckBox, SIGNAL(toggled(bool)), this, SLOT(stereoChanged(bool)));
    connect(ui->muteCheckBox, SIGNAL(toggled(bool)), this, SLOT(muteChanged(bool)));
    connect(ui->audioSlider, SIGNAL(valueChanged(int)), this, SLOT(audioChanged(int)));
    connect(ui->powerSlider, SIGNAL(valueChanged(int)), this, SLOT(powerChanged(int)));
    connect(ui->preemp75Button, SIGNAL(toggled(bool)), this, SLOT(preemphasisChanged(bool)));

    lookForAdaptor();
    openDev();
    readDev();
}



void MainWindow::errorBox(QString const& errorMsg)
{
    QMessageBox messageBox;

    messageBox.setText(errorMsg);
    messageBox.exec();

    exit(1);
}

int MainWindow::sliderPos(int frequ)
{
    int freqp;

    freqp = frequ - 1400000;
    freqp = freqp / 800;

    return freqp;
}

int MainWindow::freqFromSliderPos(int freqp)
{
    int frequ;

    frequ = freqp * 800;
    frequ = frequ + 1400000;

    return frequ;
}


/*
 * Many of the routines below were copied from a tutorial by Aan Ott
 * See http://www.signal11.us/oss/udev/ for the original source
 *
 */

void MainWindow::lookForAdaptor()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    char tryDevicePath [30];

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        qFatal("Can't create udev\n");
        errorBox("Can't find udev");
        exit(1);
    }

    /* Create a list of the devices in the 'video4linux' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* For each item enumerated, print out its information.
       udev_list_entry_foreach is a macro which expands to
       a loop. The loop will be executed for each member in
       devices, setting dev_list_entry to a list entry
       which contains the device's path in /sys.
   */
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it
        */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* usb_device_get_devnode() returns the path to the device node
           itself in /dev. */
        strcpy(tryDevicePath, udev_device_get_devnode(dev));
        qDebug("Device Node Path: %s", tryDevicePath);

        /* The device pointed to by dev contains information about
           the USB device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.
        */
        dev = udev_device_get_parent_with_subsystem_devtype(
                    dev,
                    "usb",
                    "usb_device");
        if (!dev) {
            qDebug() << "Unable to find parent usb device.";
            continue;
        }
        else if (strcmp(udev_device_get_sysattr_value(dev,"idVendor") , "046d") == 0 &&
                 strcmp(udev_device_get_sysattr_value(dev, "idProduct") , "0a0e") == 0)
        {
            /* conversion from char to qstring */
            devPath.append(tryDevicePath);
            /* leave the udev loop as we have found our device */
            break;
        }

        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    qDebug() << "Device Path is " << qPrintable(devPath);

    if (devPath.isEmpty()) {
        errorBox("No suitable radio adaptor found");
    }
    ui->adaptorDev->setText(devPath);

}

void MainWindow::openDev()
{
    struct stat st;
    struct v4l2_capability cap;

    char devChar[30];

    strcpy(devChar, devPath.toStdString().c_str());

    if (-1 == stat (devChar, &st)) {
        qCritical() << "Cannot identify " << qPrintable(devPath) << errno << strerror (errno);
        errorBox("Cant identify device");
        exit (EXIT_FAILURE);
    }

    if (!S_ISCHR (st.st_mode)) {
        qCritical() << qPrintable(devPath) << "is not a device";
        errorBox("Device found is not correct");
        exit (EXIT_FAILURE);
    }

    fd = open (devChar, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        qCritical()<<"Cannot open "<<qPrintable(devPath)<<  errno << strerror (errno);
        errorBox("Cannot open device");
        exit (EXIT_FAILURE);
    }

    if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            qCritical() << qPrintable(devPath) << "is not a V4L2 device";
            errorBox("Device is not a V4L2 device");
            exit (EXIT_FAILURE);       
        } else {
            qCritical() << qPrintable(devPath) << "error testing capability" << errno;
            errorBox("Device is not responding to testing correctly");
            exit (EXIT_FAILURE);

        }
    }
}

void MainWindow::readDev()
{
    struct v4l2_frequency freq;
    struct v4l2_modulator modulator;

    int freqUnits;
    double freqf, freqt;

// Query the dongle to find its current frequency

    freq.tuner = 0;
    freq.type = V4L2_TUNER_RADIO;

    if (-1 == ioctl (fd, VIDIOC_G_FREQUENCY, &freq)) {
        if (EINVAL == errno)
        {
          qCritical() << qPrintable(devPath) <<  " has a frequency problem";
        }

         exit (EXIT_FAILURE);
    }

/*
 * Frequency in V4L2 is in units of 62.5 hz so the following are important values
 *
 * steps of 50khz = steps of 800 units
 * 87.50 Mhz = 1,400,000
 * 108.0 Mhz = 1,728,000
 *
 * Internally the program will use freq units and only convert for spinbox display
 * The scroll bar gives values from 0 to 410  i.e 50khz increments
 */
    freqUnits = freq.frequency;
    freqf = (freqUnits * 62.5) / 1000000.0;

    qDebug () << "Dongle Frequency is " << freqf;

// When first started dongle will be at a random frequency .. convert to nearest 50khz

    freqt = freqUnits / 800.0;
    freqt = floor(freqt + 0.5);
    freqUnits = freqt * 800;
    freqf = (freqUnits * 62.5) / 1000000.0;

    qDebug () << "Dongle Frequency corrected is " << freqf;

//    ui->statusText->setNum(freqUnits);

    ui->FreqScrollBar->setValue(sliderPos(freqUnits));

    ui->freqSpinBox->setValue(freqf);

// Query the dongle to find if it is set to mono or stereo

    modulator.index = 0;

    if (-1 == ioctl (fd, VIDIOC_G_MODULATOR, &modulator)) {
        if (EINVAL == errno)
        {
            qCritical() << qPrintable(devPath) << " has a modulator problem";
        }
         exit (EXIT_FAILURE);
    }

    if (modulator.txsubchans == V4L2_TUNER_SUB_STEREO)
    {
        ui->stereoCheckBox->setChecked(true);
    }
}

void MainWindow::FreqScrollBarChanged(int freqp)
{
    struct v4l2_frequency freq;

    double freqf;
    int freqUnits;

    freqUnits = freqFromSliderPos(freqp);

    freqf = (freqUnits * 62.5) / 1000000.0;

    ui->freqSpinBox->setValue(freqf);

    freq.tuner = 0;
    freq.type = V4L2_TUNER_RADIO;
    freq.frequency = freqUnits;

    if (-1 == ioctl (fd, VIDIOC_S_FREQUENCY, &freq)) {
        if (EINVAL == errno) {
            qCritical() << "Dongle has a frequency setting problem";
            errorBox("Device has a frequency setting problem");
        }
        exit (EXIT_FAILURE);
    }
}

void MainWindow::FreqSpinBoxChanged(double freqf)
{
    int freqUnits;

    freqUnits = (freqf * 1000000.0) / 62.5;
    ui->FreqScrollBar->setValue(sliderPos(freqUnits));

}

void MainWindow::stereoChanged(bool checked)
{
    struct v4l2_modulator modulator;

    modulator.index = 0;

    if (checked) {
        modulator.txsubchans = V4L2_TUNER_SUB_STEREO;
    } else {
        modulator.txsubchans = V4L2_TUNER_SUB_MONO;
    }

    if (-1 == ioctl (fd, VIDIOC_S_MODULATOR, &modulator)) {
        if (EINVAL == errno) {
              qCritical() << "Dongle has a modulator setting problem";
        }
        exit (EXIT_FAILURE);
    }
}

void MainWindow::muteChanged(bool checked)
{
    struct v4l2_control ctrl;

    ctrl.id = V4L2_CID_AUDIO_MUTE;
    ctrl.value = checked;

    if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
        if (EINVAL == errno) {
            qCritical() << "Problem with using control function for mute";
        }
        exit (EXIT_FAILURE);
    }
}

void MainWindow::audioChanged(int level)
{
    struct v4l2_control ctrl;

    ctrl.id = V4L2_CID_AUDIO_COMPRESSION_GAIN;
    ctrl.value = level;

    if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
        if (EINVAL == errno) {
               qCritical() << "Problem with using control function for audio";
        }
         exit (EXIT_FAILURE);
    }
}

void MainWindow::powerChanged(int level)
{
    struct v4l2_control ctrl;

    ctrl.id = V4L2_CID_TUNE_POWER_LEVEL;
    ctrl.value = level;

    if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
        if (EINVAL == errno)
        {
            qCritical() << "Problem with using control function for power";
        }
         exit (EXIT_FAILURE);
    }
}

void MainWindow::preemphasisChanged(bool checked)
{
    struct v4l2_control ctrl;

    ctrl.id = V4L2_CID_TUNE_PREEMPHASIS;
    if (checked)
    {
        ctrl.value = V4L2_PREEMPHASIS_75_uS;
    }
    else
    {
        ctrl.value = V4L2_PREEMPHASIS_50_uS;
    }

    if (-1 == ioctl (fd, VIDIOC_S_CTRL, &ctrl)) {
        if (EINVAL == errno)
        {
            qCritical() << "Problem with using control function for preemphasis";
        }
        exit (EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{

    if (-1 == ::close (fd))
        exit (EXIT_FAILURE);

    fd = -1;

    delete ui;

}
