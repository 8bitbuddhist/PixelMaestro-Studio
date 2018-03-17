#include <QSerialPortInfo>
#include <QSettings>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

namespace PixelMaestroStudio {

	QString PreferencesDialog::separate_window_option = QStringLiteral("Displays/SeparateWindow");
	QString PreferencesDialog::main_window_option = QStringLiteral("Displays/MainWindow");
	QString PreferencesDialog::num_sections = QStringLiteral("Maestro/NumSections");
	QString PreferencesDialog::output_enabled = QStringLiteral("Enabled");
	QString PreferencesDialog::pause_on_start = QStringLiteral("Interface/PauseOnStart");
	QString PreferencesDialog::pixel_padding = QStringLiteral("Interface/Padding");
	QString PreferencesDialog::pixel_shape = QStringLiteral("Interface/Shape");
	QString PreferencesDialog::refresh_rate = QStringLiteral("Maestro/Refresh");
	QString PreferencesDialog::serial_ports = QStringLiteral("SerialPorts");
	QString PreferencesDialog::serial_port = QStringLiteral("Port");

	PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog) {
		ui->setupUi(this);

		// Interface settings
		ui->paddingComboBox->setCurrentIndex(settings_.value(pixel_padding, 0).toInt());		// Default to no padding
		ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());		// Default to square pixels

		// Maestro settings
		ui->numSectionsSpinBox->setValue(settings_.value(num_sections, 1).toInt());				// Default to 1 Section
		ui->refreshSpinBox->setValue(settings_.value(refresh_rate, 50).toInt());				// Default to 50 ms
		ui->pauseOnStartCheckBox->setChecked(settings_.value(pause_on_start, false).toBool());	// Default to run on start

		// Add available serial devices to combo box
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (QSerialPortInfo port : ports) {
			ui->serialOutputComboBox->addItem(port.systemLocation());
		}

		// Add saved serial devices to output selection box.
		int num_serial_devices = settings_.beginReadArray(serial_ports);
		for (int device = 0; device < num_serial_devices; device++) {
			settings_.setArrayIndex(device);
			QListWidgetItem* item = new QListWidgetItem(settings_.value(serial_port).toString());
			ui->serialOutputListWidget->addItem(item);
		}
		settings_.endArray();

		// Check off activated displays
		ui->separateWindowCheckBox->setChecked(settings_.value(separate_window_option, false).toBool());
		ui->mainWindowCheckBox->setChecked(settings_.value(main_window_option, true).toBool());
	}

	void PreferencesDialog::on_addSerialDevicePushButton_clicked() {
		if (!ui->serialOutputComboBox->currentText().isEmpty()) {
			ui->serialOutputListWidget->addItem(ui->serialOutputComboBox->currentText());
		}
	}

	void PreferencesDialog::on_buttonBox_accepted() {
		// Save Maestro settings
		settings_.setValue(refresh_rate, ui->refreshSpinBox->value());
		settings_.setValue(num_sections, ui->numSectionsSpinBox->value());
		settings_.setValue(pause_on_start, ui->pauseOnStartCheckBox->isChecked());

		// Save selected output devices
		settings_.beginWriteArray(serial_ports);
		for (int i = 0; i < ui->serialOutputListWidget->count(); i++) {
			settings_.setArrayIndex(i);
			QListWidgetItem* item = ui->serialOutputListWidget->item(i);
			settings_.setValue(serial_port, item->text());
		}
		settings_.endArray();

		// Save interface settings
		settings_.setValue(pixel_padding, ui->paddingComboBox->currentIndex());
		settings_.setValue(pixel_shape, ui->pixelShapeComboBox->currentIndex());

		// Save display settings
		settings_.setValue(separate_window_option, ui->separateWindowCheckBox->isChecked());
		settings_.setValue(main_window_option, ui->mainWindowCheckBox->isChecked());
	}

	void PreferencesDialog::on_removeSerialDevicePushButton_clicked() {
		if (ui->serialOutputListWidget->selectedItems().count() > 0) {
			ui->serialOutputListWidget->takeItem(ui->serialOutputListWidget->currentRow());
		}
	}

	void PreferencesDialog::on_serialOutputComboBox_editTextChanged(const QString &arg1) {
		ui->addSerialDevicePushButton->setEnabled(arg1.length() > 0);
	}

	void PreferencesDialog::on_serialOutputListWidget_currentRowChanged(int currentRow) {
		ui->removeSerialDevicePushButton->setEnabled(currentRow > -1);
	}

	PreferencesDialog::~PreferencesDialog() {
		delete ui;
	}
}
