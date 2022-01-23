#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QSettings>

#ifndef NO_SERIALPORT
#include <QSerialPortInfo>
#endif

#include <QSysInfo>
#include "adddevicedialog.h"
#include "ui_adddevicedialog.h"
#include "dialog/preferencesdialog.h"
#include "dialog/sectionmapdialog.h"
#include "controller/devicecontroller.h"

namespace PixelMaestroStudio {
	AddDeviceDialog::AddDeviceDialog(QVector<DeviceController>* devices, DeviceController* device, QWidget *parent) :
		QDialog(parent),
		ui(new Ui::AddDeviceDialog) {
		ui->setupUi(this);

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		this->devices_ = devices;
		this->device_ = device;

		ui->mapSectionsButton->setEnabled(false);

		QString placeholder_text;

#ifndef NO_SERIALPORT
		if (QSysInfo::productType() == "windows") {
			placeholder_text += "COM1 / ";
		}
		else {
			placeholder_text += "/dev/ttyACM0 / ";
		}
#endif
		placeholder_text += "192.168.1.100:8077";
		ui->portComboBox->lineEdit()->setPlaceholderText(placeholder_text);

		if (device != nullptr) {
			ui->autoConnectCheckBox->setChecked(device->get_autoconnect());
			ui->deviceTypeComboBox->setCurrentIndex(device->get_device_type());
			ui->liveUpdatesCheckBox->setChecked(device->get_real_time_refresh_enabled());
			ui->portComboBox->setCurrentText(device->get_port_name());
		}
		else {
			// No device detected, i.e. the user clicked "New"
			// FIXME: only disable the SectionMapdialog, not this checkbox
			ui->liveUpdatesCheckBox->setEnabled(false);
		}

		populate_serial_devices();
	}

	bool AddDeviceDialog::is_device_already_added(QString port_name) {
		bool already_added = false;
		for (int i = 0; i < devices_->size(); i++) {
			if (devices_->at(i).get_port_name() == port_name) {
				already_added = true;
			}
		}

		return already_added;
	}

	void AddDeviceDialog::on_buttonBox_accepted() {
		// If serial is disabled, check to make sure this isn't a serial device.
#ifdef NO_SERIALPORT
		if ((DeviceController::DeviceType)ui->deviceTypeComboBox->currentIndex() == DeviceController::DeviceType::Serial) {
			QMessageBox::warning(this, "Unsupported", "USB / Serial devices are not supported on this platform.");
			return;
		}
#endif

		// If device == nullptr, we need to first create a new device
		if (!device_) {
			if (is_device_already_added(ui->portComboBox->currentText())) {
				QMessageBox::warning(this, "Device Already Added", "This device has already been added.");
				return;
			}

			devices_->push_back(DeviceController());
			this->device_ = const_cast<DeviceController*>(&devices_->at(devices_->size() - 1));
		}

		// Save settings to the device
		device_->set_device_type((DeviceController::DeviceType)ui->deviceTypeComboBox->currentIndex());
		device_->set_port_name(ui->portComboBox->currentText());
		device_->set_real_time_update(ui->liveUpdatesCheckBox->isChecked());
		device_->set_autoconnect(ui->autoConnectCheckBox->isChecked());

		// Finally, save all devices to settings
		// FIXME: On WebAssembly, QSettings is async. See example at https://github.com/msorvig/qt-webassembly-examples/blob/master/gui_settings/main.cpp
		QSettings settings;
		settings.remove(PreferencesDialog::devices);
		settings.beginWriteArray(PreferencesDialog::devices);
		for (int i = 0; i < devices_->size(); i++) {
			settings.setArrayIndex(i);

			DeviceController* current_device = const_cast<DeviceController*>(&devices_->at(i));
			settings.setValue(PreferencesDialog::device_port, current_device->get_port_name());
			settings.setValue(PreferencesDialog::device_real_time_refresh, current_device->get_real_time_refresh_enabled());
			settings.setValue(PreferencesDialog::device_autoconnect, current_device->get_autoconnect());
			settings.setValue(PreferencesDialog::device_connectiontype, current_device->get_device_type());

			// Save the device's model->
			SectionMapModel* model = current_device->section_map_model;
			if (model != nullptr) {
				settings.beginWriteArray(PreferencesDialog::section_map);

				QModelIndex parent = QModelIndex();
				for (int row = 0; row < model->rowCount(parent); row++) {
					settings.setArrayIndex(row);

					QStandardItem* local_section = model->item(row, 0);
					settings.setValue(PreferencesDialog::section_map_local, local_section->text().toInt());

					QStandardItem* remote_section = model->item(row, 1);
					settings.setValue(PreferencesDialog::section_map_remote, remote_section->text().toInt());
				}
				settings.endArray();
			}
		}
		settings.endArray();
	}

	void AddDeviceDialog::on_liveUpdatesCheckBox_stateChanged(int arg1) {
		ui->mapSectionsButton->setEnabled(arg1 > 0);
	}

	void AddDeviceDialog::on_mapSectionsButton_clicked() {
		SectionMapDialog dialog(
			*device_,
			this
		);
		dialog.exec();
	}

	/// Displays all available serial devices in the serial output combobox.
	void AddDeviceDialog::populate_serial_devices() {
		ui->portComboBox->clear();
#ifndef NO_SERIALPORT
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (const QSerialPortInfo& port : ports) {
			ui->portComboBox->addItem(port.systemLocation());
		}
#endif
	}

	AddDeviceDialog::~AddDeviceDialog() {
		delete ui;
	}
}
