#include <QList>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include "dialog/preferencesdialog.h"
#include "devicecontrolwidget.h"
#include "ui_devicecontrolwidget.h"

namespace PixelMaestroStudio {
	DeviceControlWidget::DeviceControlWidget(MaestroControlWidget* maestro_control_widget, QWidget *parent) : QWidget(parent), ui(new Ui::DeviceControlWidget) {
		ui->setupUi(this);
		this->maestro_control_widget_ = maestro_control_widget;

		ui->sendPushButton->setEnabled(false);

		// Add available serial devices to combo box
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (QSerialPortInfo port : ports) {
			ui->serialOutputComboBox->addItem(port.systemLocation());
		}

		// Add saved serial devices to output selection box.
		QSettings settings;
		int num_serial_devices = settings.beginReadArray(PreferencesDialog::serial_ports);
		for (int device = 0; device < num_serial_devices; device++) {
			QString device_name = settings.value(PreferencesDialog::serial_port).toString();
			settings.setArrayIndex(device);
			QListWidgetItem* item = new QListWidgetItem(device_name);
			ui->serialOutputListWidget->addItem(item);
			connect_to_serial_device(device_name);
		}
		settings.endArray();
	}

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

	bool DeviceControlWidget::disconnect_serial_device(QString port_name) {
		// Search through each connected device until we find our target device
		for (QSharedPointer<QSerialPort> device : serial_devices_) {
			if (device->portName() == port_name) {
				return serial_devices_.removeAll(device);
			}
		}

		return false;
	}

	void DeviceControlWidget::on_addSerialDevicePushButton_clicked() {
		if (!ui->serialOutputComboBox->currentText().isEmpty()) {
			QString device_name = ui->serialOutputComboBox->currentText();
			ui->serialOutputListWidget->addItem(device_name);
			connect_to_serial_device(device_name);
		}
	}

	void DeviceControlWidget::on_removeSerialDevicePushButton_clicked() {
		if (ui->serialOutputListWidget->selectedItems().count() > 0) {
			ui->serialOutputListWidget->takeItem(ui->serialOutputListWidget->currentRow());
		}
	}

	void DeviceControlWidget::on_sendPushButton_clicked() {
		QByteArray out = QByteArray("PMCEEP", 6);
		out += maestro_cue_ + QByteArray("EEPEND", 6);
		for (QSharedPointer<QSerialPort> device : serial_devices_) {
			device->write((const char*)out, out.size());
		}
	}

	void DeviceControlWidget::on_serialOutputComboBox_editTextChanged(const QString &arg1) {
		ui->addSerialDevicePushButton->setEnabled(arg1.length() > 0);
	}

	void DeviceControlWidget::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		ui->removeSerialDevicePushButton->setEnabled(currentRow > -1);
		ui->sendPushButton->setEnabled(currentRow > -1);
	}

	void DeviceControlWidget::run_cue(uint8_t *cue, int size) {
		for (QSharedPointer<QSerialPort> device : serial_devices_) {
			device->write((const char*)cue, size);
		}

		// Calculate and display the size of the current Maestro configuration
		QDataStream datastream(&maestro_cue_, QIODevice::Truncate);
		maestro_control_widget_->get_maestro_controller()->save_maestro_to_datastream(&datastream);
		ui->configSizeLineEdit->setText(QString::number(maestro_cue_.size()));
	}

	DeviceControlWidget::~DeviceControlWidget() {
		for (QSharedPointer<QSerialPort> device : serial_devices_) {
			disconnect_serial_device(device->portName());
		}
		delete ui;
	}
}
