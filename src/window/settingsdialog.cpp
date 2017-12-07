#include <QSerialPortInfo>
#include <QSettings>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

// Initialize strings
QString SettingsDialog::detached_window_option = QStringLiteral("Detached window");
QString SettingsDialog::main_window_option = QStringLiteral("Main window");
QString SettingsDialog::num_sections = QStringLiteral("maestro/num_sections");
QString SettingsDialog::pixel_padding = QStringLiteral("interface/padding");
QString SettingsDialog::pixel_shape = QStringLiteral("interface/shape");
QString SettingsDialog::output_devices = QStringLiteral("serial/outputs");
QString SettingsDialog::output_enabled = QStringLiteral("enabled");
QString SettingsDialog::output_name = QStringLiteral("port");
QString SettingsDialog::refresh_rate = QStringLiteral("maestro/refresh");
QString SettingsDialog::serial_enabled = QStringLiteral("serial/enabled");
QString SettingsDialog::serial_port = QStringLiteral("serial/port");

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
	/*
	 * FIXME: If a device is found in settings but not in the list, add the device to the list, uncheck it, and make it unselectable.
	 */
	int num_devices = settings_.beginReadArray(output_devices);
	for (int list_item = 0; list_item < ui->outputListWidget->count(); list_item++) {
		for (int device = 0; device < num_devices; device++) {
			settings_.setArrayIndex(device);
			QListWidgetItem* item = ui->outputListWidget->item(list_item);
			if (settings_.contains(output_name) && settings_.value(output_name).toString().compare(item->text(), Qt::CaseInsensitive) == 0) {
				item->setCheckState((Qt::CheckState)settings_.value(output_enabled).toInt());
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
}

SettingsDialog::~SettingsDialog() {
	delete ui;
}
