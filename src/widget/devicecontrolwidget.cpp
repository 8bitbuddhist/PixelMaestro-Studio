/*
 * DeviceControlWidget - Widget for managing USB/serial devices.
 */

#include <chrono>
#include <exception>
#include <QList>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <thread>
#include "dialog/cueinterpreterdialog.h"
#include "dialog/preferencesdialog.h"
#include "devicecontrolwidget.h"
#include "ui_devicecontrolwidget.h"

namespace PixelMaestroStudio {
	DeviceControlWidget::DeviceControlWidget(MaestroControlWidget* maestro_control_widget, QWidget *parent) : QWidget(parent), ui(new Ui::DeviceControlWidget) {
		ui->setupUi(this);
		this->maestro_control_widget_ = maestro_control_widget;

		// Disable device buttons by default
		ui->deviceSettingsGroupBox->setEnabled(false);

		// Add available serial devices to combo box
		populate_serial_devices();

		// Add saved serial devices to output selection box.
		QSettings settings;
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::devices);
		for (int device = 0; device < num_serial_devices; device++) {
			settings.setArrayIndex(device);

			QString device_name = settings.value(PreferencesDialog::device_port).toString();
			serial_devices_.push_back(SerialDevice(device_name));

			// If the saved port is an available port, connect to it
			for (QSerialPortInfo port : ports) {
				if (port.systemLocation() == device_name) {
					QListWidgetItem* item = new QListWidgetItem(device_name);
					ui->serialOutputListWidget->addItem(item);

					bool connected = serial_devices_.last().connect();

					if (!connected) {
						QMessageBox::warning(this, "Unable to Connect", QString("Unable to connect to device on port " + serial_devices_.last().get_port_name() + "."));
					}
					break;
				}
			}
		}
		settings.endArray();
	}

	/**
	 * Compares the Cuefile size to the selected device's ROM capacity, and highlights the Cuesize textbox accordingly.
	 */
	void DeviceControlWidget::check_device_rom_capacity() {
		if (ui->serialOutputListWidget->currentRow() > -1) {
			int capacity = serial_devices_[ui->serialOutputListWidget->currentRow()].get_capacity();
			int capacity_75 = capacity * 0.75;
			if (this->maestro_cue_.size() >= capacity) {
				ui->configSizeLineEdit->setStyleSheet("border: 1px solid red");
			}
			else if (this->maestro_cue_.size() >= capacity_75) {
				ui->configSizeLineEdit->setStyleSheet("border: 1px solid orange");
			}
			else {
				ui->configSizeLineEdit->setStyleSheet("border: 1px solid green");
			}
		}
	}

	QByteArray* DeviceControlWidget::get_maestro_cue() {
		return &maestro_cue_;
	}

	/**
	 * Connects to the selected device.
	 */
	void DeviceControlWidget::on_connectPushButton_clicked() {
		// Check to make sure the entry isn't blank or already in the list
		if (!ui->serialOutputComboBox->currentText().isEmpty() && ui->serialOutputListWidget->findItems(ui->serialOutputComboBox->currentText(), Qt::MatchFixedString).count() >= 0) {

			// Initialize and try connecting to the device.
			SerialDevice device(ui->serialOutputComboBox->currentText());
			bool connected = device.connect();
			if (connected) {
				ui->serialOutputListWidget->addItem(device.get_port_name());

				serial_devices_.push_back(device);
				save_devices();
			}
			else {
				QMessageBox::warning(this, "Unable to Connect", QString("Unable to connect to device on port " + device.get_port_name() + "."));
			}
		}
		else {
			QMessageBox::information(this, "Device Already Connected", "This device is already connected.");
		}
	}

	/**
	 * Saves the new capacity value to settings.
	 */
	void DeviceControlWidget::on_capacityLineEdit_editingFinished() {
		serial_devices_[ui->serialOutputListWidget->currentRow()].set_capacity(ui->capacityLineEdit->text().toInt());
		save_devices();
		check_device_rom_capacity();
	}

	/**
	 * Opens the CueInterpreter dialog for the selected device.
	 */
	void DeviceControlWidget::on_interpretCuePushButton_clicked() {
		CueInterpreterDialog dialog(this, (uint8_t*)maestro_cue_.data(), maestro_cue_.size());
		dialog.exec();
	}

	/**
	 * Toggles real-time serial updates.
	 * @param arg1 If checked, update the device in real-time.
	 */
	void DeviceControlWidget::on_realTimeCheckBox_stateChanged(int arg1) {
		serial_devices_[ui->serialOutputListWidget->currentRow()].set_real_time_update(arg1);
		save_devices();
	}

	/**
	 * Disconnects from the selected device.
	 */
	void DeviceControlWidget::on_disconnectPushButton_clicked() {
		if (ui->serialOutputListWidget->selectedItems().count() > 0) {
			int device_index = ui->serialOutputListWidget->currentRow();
			ui->serialOutputListWidget->takeItem(device_index);
			serial_devices_.removeAt(device_index);
			save_devices();
		}
	}

	/**
	 * Transmits the Maestro's Cuefile to the selected device.
	 */
	void DeviceControlWidget::on_sendPushButton_clicked() {
		// Run the upload process in a separate thread
		UploadThread* thread = new UploadThread(this, &serial_devices_[ui->serialOutputListWidget->currentRow()]);
		connect(thread, &UploadThread::finished, thread, &QObject::deleteLater);
		connect(thread, &UploadThread::progress_changed, this, &DeviceControlWidget::set_progress_bar);
		connect(thread, &UploadThread::upload_in_progress, this, &DeviceControlWidget::set_upload_controls_enabled);
		thread->start();
	}

	void DeviceControlWidget::on_serialOutputComboBox_editTextChanged(const QString &arg1) {
		ui->connectPushButton->setEnabled(arg1.length() > 0);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		ui->disconnectPushButton->setEnabled(currentRow > -1);
		ui->deviceSettingsGroupBox->setEnabled(currentRow > -1);
		ui->uploadProgressBar->setValue(0);

		if (currentRow > -1) {
			ui->capacityLineEdit->setText(QString::number(serial_devices_[currentRow].get_capacity()));
			ui->realTimeCheckBox->setChecked(serial_devices_[currentRow].get_real_time_refresh_enabled());
			check_device_rom_capacity();
		}
	}

	/// Displays all available serial devices in the serial output combobox.
	void DeviceControlWidget::populate_serial_devices() {
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (QSerialPortInfo port : ports) {
			ui->serialOutputComboBox->addItem(port.systemLocation());
		}
	}

	/**
	 * Updates the Maestro's Cuefile.
	 * This also sends the Cue to all connected devices.
	 * @param cue The Cue to execute.
	 * @param size The size of the Cue.
	 */
	void DeviceControlWidget::run_cue(uint8_t *cue, int size) {
		for (int i = 0; i < serial_devices_.size(); i++) {
			SerialDevice device = serial_devices_.at(i);
			if (device.get_device()->isOpen() && device.get_real_time_refresh_enabled() == true) {
				device.write((const char*)cue, size);
			}
		}

		update_cuefile_size();
	}

	/**
	 * Saves the device list to settings.
	 */
	void DeviceControlWidget::save_devices() {
		// Save devices
		QSettings settings;
		settings.beginWriteArray(PreferencesDialog::devices);
		for (int i = 0; i < ui->serialOutputListWidget->count(); i++) {
			settings.setArrayIndex(i);

			SerialDevice device = serial_devices_.at(i);
			settings.setValue(PreferencesDialog::device_capacity, device.get_capacity());
			settings.setValue(PreferencesDialog::device_port, device.get_port_name());
			settings.setValue(PreferencesDialog::device_real_time_refresh, device.get_real_time_refresh_enabled());
		}
		settings.endArray();
	}

	/**
	 * Sets the state of the progress bar.
	 * @param val Value to set the progress to.
	 */
	void DeviceControlWidget::set_progress_bar(int val) {
		ui->uploadProgressBar->setValue(val);
	}

	/**
	 * Sets whether the user can click the upload button.
	 * @param enabled If true, button is enabled.
	 */
	void DeviceControlWidget::set_upload_controls_enabled(bool enabled) {
		ui->sendPushButton->setEnabled(enabled);
	}

	/**
	 * Regenerates the Maestro Cuefile and updates the size in the UI.
	 */
	void DeviceControlWidget::update_cuefile_size() {
		if (!maestro_control_widget_->loading_cue_) {
			// Calculate and display the size of the current Maestro configuration
			QDataStream datastream(&maestro_cue_, QIODevice::Truncate);
			maestro_control_widget_->get_maestro_controller()->save_maestro_to_datastream(&datastream);
			ui->configSizeLineEdit->setText(QString::number(maestro_cue_.size()));
			check_device_rom_capacity();
		}
	}

	DeviceControlWidget::~DeviceControlWidget() {
		for (int i = 0; i < serial_devices_.size(); i++) {
			serial_devices_[i].disconnect();
		}
		delete ui;
	}
}
