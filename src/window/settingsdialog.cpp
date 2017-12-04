#include <QSerialPortInfo>
#include <QSettings>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

// Initialize strings
QString SettingsDialog::num_sections = QStringLiteral("maestro/num_sections");
QString SettingsDialog::pixel_padding = QStringLiteral("interface/padding");
QString SettingsDialog::pixel_shape = QStringLiteral("interface/shape");
QString SettingsDialog::output_devices = QStringLiteral("serial/outputs");
QString SettingsDialog::output_enabled = QStringLiteral("enabled");
QString SettingsDialog::output_name = QStringLiteral("port");
QString SettingsDialog::refresh_rate = QStringLiteral("maestro/refresh");
QString SettingsDialog::screen_option = QStringLiteral("screen");
QString SettingsDialog::serial_enabled = QStringLiteral("serial/enabled");
QString SettingsDialog::serial_port = QStringLiteral("serial/port");
QString SettingsDialog::virtual_device_option = QStringLiteral("Simulated Device");
QString SettingsDialog::virtual_device_width = QStringLiteral("serial/simulated_device/width");
QString SettingsDialog::virtual_device_height = QStringLiteral("serial/simulated_device/height");

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog) {
	ui->setupUi(this);

	// Interface settings
	ui->paddingComboBox->setCurrentIndex(settings_.value(pixel_padding, 0).toInt());	// Default to no padding
	ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());	// Default to square pixels

	// Maestro settings
	ui->numSectionsSpinBox->setValue(settings_.value(num_sections, 1).toInt());			// Default to 1 Section
	ui->refreshSpinBox->setValue(settings_.value(refresh_rate, 50).toInt());			// Default to 50 ms

	// Add serial devices to output selection box
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	for (QSerialPortInfo port : ports) {
		QListWidgetItem* item = new QListWidgetItem(port.systemLocation());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
		ui->outputListWidget->addItem(item);
	}

	// Check to see if each entry in the output selection box has already been configured.
	int num_devices = settings_.beginReadArray(output_devices);
	for (int list_item = 0; list_item < ui->outputListWidget->count(); list_item++) {
		for (int device = 0; device < num_devices; device++) {
			settings_.setArrayIndex(device);
			QListWidgetItem* item = ui->outputListWidget->item(list_item);
			if (settings_.contains(output_name) && settings_.value(output_name).toString().compare(item->text(), Qt::CaseInsensitive) == 0) {
				item->setCheckState((Qt::CheckState)settings_.value(output_enabled).toInt());
			}

			// If this is the Simulated Device, toggle the simulated device controls
			if (item->text().compare(virtual_device_option, Qt::CaseInsensitive) == 0) {
				set_simulated_device_options_enabled(item->checkState() == Qt::Checked);
			}
		}
	}
	settings_.endArray();
}

void SettingsDialog::on_buttonBox_accepted() {
	// Save Maestro settings
	settings_.setValue(refresh_rate, ui->refreshSpinBox->value());
	settings_.setValue(num_sections, ui->numSectionsSpinBox->value());

	// Save selected output devices
	settings_.beginWriteArray(output_devices);
	for (int i = 0; i < ui->outputListWidget->count(); i++) {
		settings_.setArrayIndex(i);
		QListWidgetItem* item = ui->outputListWidget->item(i);
		settings_.setValue(output_name, item->text());
		settings_.setValue(output_enabled, item->checkState());
	}
	settings_.endArray();

	// Save interface settings
	settings_.setValue(pixel_padding, ui->paddingComboBox->currentIndex());
	settings_.setValue(pixel_shape, ui->pixelShapeComboBox->currentIndex());

	// Save virtual device size
	settings_.setValue(virtual_device_width, ui->simulatedWidthSpinBox->value());
	settings_.setValue(virtual_device_height, ui->simulatedHeightSpinBox->value());
}

void SettingsDialog::on_outputListWidget_itemChanged(QListWidgetItem *item) {
	// For Simulated Devices, set the serial device size options enabled/disabled.
	if (item->text().compare(virtual_device_option, Qt::CaseInsensitive) == 0) {
		set_simulated_device_options_enabled(item->checkState() == Qt::Checked);
	}
}

void SettingsDialog::set_simulated_device_options_enabled(bool enabled) {
	ui->simulatedDeviceWidthLabel->setEnabled(enabled);
	ui->simulatedDeviceHeightLabel->setEnabled(enabled);
	ui->simulatedWidthSpinBox->setEnabled(enabled);
	ui->simulatedHeightSpinBox->setEnabled(enabled);
}

SettingsDialog::~SettingsDialog() {
	delete ui;
}
