#include <chrono>
#include <exception>
#include <QList>
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

		// Disable Upload button by default
		ui->sendPushButton->setEnabled(false);

		// Add available serial devices to combo box
		populate_serial_devices();

		// Add saved serial devices to output selection box.
		QSettings settings;
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::serial_ports);
		for (int device = 0; device < num_serial_devices; device++) {
			settings.setArrayIndex(device);
			QString device_name = settings.value(PreferencesDialog::serial_port).toString();

			// If the saved port is an available port, connect to it
			for (QSerialPortInfo port : ports) {
				if (port.systemLocation() == device_name) {
					QListWidgetItem* item = new QListWidgetItem(device_name);
					ui->serialOutputListWidget->addItem(item);
					connect_to_serial_device(device_name);
					break;
				}
			}
		}
		settings.endArray();
	}

	/**
	 * Opens a serial connection to the specified port.
	 * @param port_name Port to connect to.
	 * @return True if the connection was successful.
	 */
	bool DeviceControlWidget::connect_to_serial_device(QString port_name) {
		/*
		 * Initialize the serial device
		 * These settings are currently only designed for Arduinos: https://stackoverflow.com/questions/13312869/serial-communication-with-arduino-fails-only-on-the-first-message-after-restart
		 */
		QSharedPointer<QSerialPort> serial_device(new QSerialPort());
		serial_device->setPortName(port_name);
		serial_device->setBaudRate(9600);

		// Set comm settings
		serial_device->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
		serial_device->setParity(QSerialPort::Parity::NoParity);
		serial_device->setDataBits(QSerialPort::DataBits::Data8);
		serial_device->setStopBits(QSerialPort::StopBits::OneStop);

		if (serial_device->open(QIODevice::WriteOnly)) {
			serial_devices_.push_back(serial_device);
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Closes the connection to the specified serial device.
	 * @param index Index of the device in the list.
	 */
	void DeviceControlWidget::disconnect_serial_device(int index) {
		serial_devices_[index]->flush();
		serial_devices_[index]->close();
		serial_devices_.remove(index);
	}

	/**
	 * Connects to the selected device.
	 */
	void DeviceControlWidget::on_addSerialDevicePushButton_clicked() {
		// Check to make sure the entry isn't blank or already in the list
		if (!ui->serialOutputComboBox->currentText().isEmpty() && ui->serialOutputComboBox->findText(ui->serialOutputComboBox->currentText()) >= 0) {
			QString device_name = ui->serialOutputComboBox->currentText();
			ui->serialOutputListWidget->addItem(device_name);
			connect_to_serial_device(device_name);

			save_device_list();
		}
	}

	/**
	 * Opens the CueInterpreter dialog for the selected device.
	 */
	void DeviceControlWidget::on_interpretCuePushButton_clicked() {
		CueInterpreterDialog dialog(this, (uint8_t*)maestro_cue_.data(), maestro_cue_.size());
		dialog.exec();
	}

	/**
	 * Disconnects from the selected device.
	 */
	void DeviceControlWidget::on_removeSerialDevicePushButton_clicked() {
		if (ui->serialOutputListWidget->selectedItems().count() > 0) {
			int device_index = ui->serialOutputListWidget->currentRow();
			disconnect_serial_device(device_index);
			ui->serialOutputListWidget->takeItem(device_index);
			save_device_list();
		}
	}

	/**
	 * Transmits the Maestro's Cuefile to the selected device.
	 */
	void DeviceControlWidget::on_sendPushButton_clicked() {
		/*
		 * This whole method sucks, but here's the deal:
		 *
		 * When sending a Cuefile to an Arduino, PixelMaestro Studio sends data way too fast, even with a low baud rate.
		 * The Arduino's serial buffer overflows and it starts dropping Cues. The reason it worked before is probably because we were sending individual Cues and not entire Cuefiles.
		 * Basically, the Cuefile gets trimmed and the entire thing fails.
		 * As a workaround, we break up the Cuefile into 64 byte chunks and give the Arduino a few milliseconds between each chunk to catch up.
		 *
		 * The final message looks like this:
		 *	1) "ROMBEG", which signals the Arduino to start writing to EEPROM.
		 *	2) The Cuefile itself. The entire file gets written to Serial.
		 *	3) "ROMEND", which signals the Arduino to stop writing to EEPROM.
		 *
		 * Resources:
		 * http://forum.arduino.cc/index.php?topic=124158.15
		 * https://forum.arduino.cc/index.php?topic=234151.0
		 *
		 */

		// TODO: Move to separate thread

		ui->uploadProgressBar->setValue(0);
		// Send start flag
		QByteArray out = QByteArray("ROMBEG", 6);
		write_to_devices((const char*)out, out.size());

		// Send Cuefile broken up into 64-bit chunks
		int index = 0;
		std::chrono::milliseconds sleep_period(250);	// Wait 250 milliseconds between chunks
		do {
			out = maestro_cue_.mid(index, 64);
			write_to_devices((const char*)out, out.size());
			index += 64;
			std::this_thread::sleep_for(sleep_period);
			ui->uploadProgressBar->setValue((index / (float)maestro_cue_.size()) * 100);
		}
		while (index < maestro_cue_.size());

		// Send stop flag
		out = QByteArray("ROMEND", 6);
		write_to_devices((const char*)out, out.size());
		ui->uploadProgressBar->setValue(100);
	}

	void DeviceControlWidget::on_serialOutputComboBox_editTextChanged(const QString &arg1) {
		ui->addSerialDevicePushButton->setEnabled(arg1.length() > 0);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		ui->removeSerialDevicePushButton->setEnabled(currentRow > -1);
		ui->sendPushButton->setEnabled(currentRow > -1);
		ui->uploadProgressBar->setValue(0);
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
		write_to_devices((const char*)cue, size);

		// Calculate and display the size of the current Maestro configuration
		QDataStream datastream(&maestro_cue_, QIODevice::Truncate);
		maestro_control_widget_->get_maestro_controller()->save_maestro_to_datastream(&datastream);
		ui->configSizeLineEdit->setText(QString::number(maestro_cue_.size()));
	}

	/**
	 * Saves the device list to settings.
	 */
	void DeviceControlWidget::save_device_list() {
		// Save devices
		QSettings settings;
		settings.beginWriteArray(PreferencesDialog::serial_ports);
		for (int i = 0; i < ui->serialOutputListWidget->count(); i++) {
			settings.setArrayIndex(i);
			QListWidgetItem* item = ui->serialOutputListWidget->item(i);
			settings.setValue(PreferencesDialog::serial_port, item->text());
		}
		settings.endArray();
	}

	/**
	 * Sends data to all connected devices.
	 * @param out The data to send.
	 * @param size The size of the data to send.
	 */
	void DeviceControlWidget::write_to_devices(const char *out, int size) {
		for (QSharedPointer<QSerialPort> device : serial_devices_) {
			device->write(out, size);
			device->flush();
		}
	}

	DeviceControlWidget::~DeviceControlWidget() {
		for (int i = 0; i < serial_devices_.size(); i++) {
			disconnect_serial_device(i);
		}
		delete ui;
	}
}
