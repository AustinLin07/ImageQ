/*
* Copyright (C) 2012 Jorge Aparicio <jorge.aparicio.r@gmail.com>
*
* This file is part of ImageQ.
*
* ImageQ is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* ImageQ is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with ImageQ. If not, see <http://www.gnu.org/licenses/>.
*/

#include "thresholdwindow.h"
#include "ui_thresholdwindow.h"

ThresholdWindow::ThresholdWindow(cv::Mat const& image,
                                 cv::Mat const& backup,
                                 QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::ThresholdWindow),
  image(image),
  backup(backup),
  histogram(image, 256),
  yAxis(0),
  abort(true)
{
  ui->setupUi(this);

  histogram.attach(ui->histogramPlot);

  ui->histogramPlot->setAxisAutoScale(QwtPlot::xBottom, false);
  ui->histogramPlot->setAxisScale(QwtPlot::xBottom, 0, 256);

  ui->histogramPlot->enableAxis(QwtPlot::xBottom, false);
  ui->histogramPlot->enableAxis(QwtPlot::yLeft, false);

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setFixedSize(this->size());

  this->show();

  threshold();
}

ThresholdWindow::~ThresholdWindow()
{
  delete ui;
}

void ThresholdWindow::closeEvent(QCloseEvent *)
{
  if (abort)
    backup.copyTo(image);

  emit updatedImage();
}

void ThresholdWindow::on_cancelPushButton_clicked()
{
  this->close();
}

void ThresholdWindow::on_okPushButton_clicked()
{
  abort = false;

  this->close();
}

void ThresholdWindow::on_thresholdSlider_sliderMoved(int value)
{
  if (yAxis)
    delete yAxis;

  yAxis = new QwtPlotMarker;

  yAxis->setLineStyle(QwtPlotMarker::VLine);
  yAxis->setLinePen(QPen(Qt::red));
  yAxis->setXValue(value + 0.5);
  yAxis->attach(ui->histogramPlot);

  ui->histogramPlot->replot();

  threshold();
}

void ThresholdWindow::on_invertedCheckBox_toggled(bool)
{
  threshold();
}

void ThresholdWindow::on_otsuCheckBox_toggled(bool checked)
{
  ui->thresholdSlider->setDisabled(checked);

  threshold();
}

void ThresholdWindow::on_normalRadioButton_toggled(bool checked)
{
  if (checked)
    threshold();
}

void ThresholdWindow::on_toZeroRadioButton_toggled(bool checked)
{
  if (checked)
    threshold();
}

void ThresholdWindow::on_truncateRadioButton_toggled(bool checked)
{
  ui->invertedCheckBox->setDisabled(checked);

  if (checked) {
    ui->invertedCheckBox->setChecked(false);

    threshold();
  }
}

void ThresholdWindow::threshold()
{
  int type = 0;

  if (ui->normalRadioButton->isChecked()) {
    if (ui->invertedCheckBox->isChecked())
      type = cv::THRESH_BINARY_INV;
    else
      type = cv::THRESH_BINARY;
  } else if (ui->truncateRadioButton->isChecked()) {
    type = cv::THRESH_TRUNC;
  } else if (ui->toZeroRadioButton->isChecked()) {
    if (ui->invertedCheckBox->isChecked())
      type = cv::THRESH_TOZERO_INV;
    else
      type = cv::THRESH_TOZERO;
  }

  if (ui->otsuCheckBox->isChecked())
    type |= cv::THRESH_OTSU;

  cv::threshold(backup,
                image,
                ui->thresholdSlider->value(),
                255.0,
                type);

  emit updatedImage();
}
