#include <QSerialPortInfo>
#include <QSettings>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

namespace PixelMaestroStudio {
	// Initialize strings
	QString PreferencesDialog::detached_window_option = QStringLiteral("Detached window");
	QString PreferencesDialog::main_window_option = QStringLiteral("Main window");
	QString PreferencesDialog::num_sections = QStringLiteral("maestro/num_sections");
	QString PreferencesDialog::pause_on_start = QStringLiteral("interface/pauseonstart");
	QString PreferencesDialog::pixel_padding = QStringLiteral("interface/padding");
	QString PreferencesDialog::pixel_shape = QStringLiteral("interface/shape");
	QString PreferencesDialog::output_devices = QStringLiteral("serial/outputs");
	QString PreferencesDialog::output_enabled = QStringLiteral("enabled");
	QString PreferencesDialog::output_name = QStringLiteral("port");
	QString PreferencesDialog::refresh_rate = QStringLiteral("maestro/refresh");
	QString PreferencesDialog::serial_enabled = QStringLiteral("serial/enabled");
	QString PreferencesDialog::serial_port = QStringLiteral("serial/port");

	PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog) {
		ui->setupUi(this);

		// Interface settings
		ui->paddingComboBox->setCurrentIndex(settings_.value(pixel_padding, 0).toInt());	// Default to no padding
		ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());	// Default to square pixels

		// Maestro settings
		ui->numSectionsSpinBox->setValue(settings_.value(num_sections, 1).toInt());			// Default to 1 Section
		ui->refreshSpinBox->setValue(settings_.value(refresh_rate, 50).toInt());			// Default to 50 ms
		ui->pauseOnStartCheckBox->setChecked(settings_.value(pause_on_start, false).toBool());	// Default to run on start

		// Add serial devices to output selection box
		QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
		for (QSerialPortInfo port : ports) {
			QListWidgetItem* item = new QListWidgetItem(port.systemLocation());
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
			ui->outputListWidget->addItem(item);
		}

		/*
		 * Add saved devices to output selection box.
		 * For devices that are already listed, either check them or uncheck them based on the user's saved preferences.
		 * If the device isn't found, make it unselectable.
		 */
		int num_devices = settings_.beginReadArray(output_devices);
		for (int device = 0; device < num_devices; device++) {
			bool found= false;
			for (int list_item = 0; list_item < ui->outputListWidget->count(); list_item++) {
				settings_.setArrayIndex(device);
				QListWidgetItem* item = ui->outputListWidget->item(list_item);
				if (settings_.contains(output_name) && settings_.value(output_name).toString().compare(item->text(), Qt::CaseInsensitive) == 0) {
					item->setCheckState((Qt::CheckState)settings_.value(output_enabled).toInt());
					found = true;
				}
			}

			if (!found) {
				QListWidgetItem* saved_item = new QListWidgetItem(settings_.value(output_name).toString());
				// Make the item un-interactable
				saved_item->setCheckState((Qt::CheckState)0);
				saved_item->setFlags(Qt::NoItemFlags);
				ui->outputListWidget->addItem(saved_item);
			}
		}
		settings_.endArray();
	}

	void PreferencesDialog::on_buttonBox_accepted() {
		// Save Maestro settings
		settings_.setValue(refresh_rate, ui->refreshSpinBox->value());
		settings_.setValue(num_sections, ui->numSectionsSpinBox->value());
		settings_.setValue(pause_on_start, ui->pauseOnStartCheckBox->isChecked());

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

	PreferencesDialog::~PreferencesDialog() {
		delete ui;
	}
}