/*
 * DeviceControlWidget - Widget for managing USB/serial devices.
 */

#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QBuffer>
#include <QSharedPointer>
#include <QThread>
#include <QVector>
#include <QWidget>
#include "utility/serialdevice.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class DeviceControlWidget;
}

namespace PixelMaestroStudio {
	class MaestroControlWidget;
	class DeviceControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit DeviceControlWidget(MaestroControlWidget* maestro_control_widget, QWidget *parent = 0);
			~DeviceControlWidget();
			QByteArray* get_maestro_cue();
			void run_cue(uint8_t* cue, int size);
			void update_cuefile_size();

		private slots:
			void on_addSerialDevicePushButton_clicked();
			void on_interpretCuePushButton_clicked();
			void on_realTimeCheckBox_stateChanged(int arg1);
			void on_removeSerialDevicePushButton_clicked();
			void on_sendPushButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);
			void update_progress_bar(int val);

			void on_capacityLineEdit_editingFinished();

		private:
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			Ui::DeviceControlWidget *ui;

			/// Stores the current Maestro configuration in Cue form.
			QByteArray maestro_cue_;

			/// List of activated USB devices.
			QVector<SerialDevice> serial_devices_;

			void check_device_rom_capacity();
			bool connect_to_serial_device(QString port_name);
			void disconnect_serial_device(int index);
			void populate_serial_devices();
			void save_devices();
			void write_to_devices(const char* out, int size);
	};

	class UploadThread : public QThread {
		Q_OBJECT

		public:
			UploadThread(QObject* parent, SerialDevice* device) : QThread(parent) {
				this->target_device_ = device;
			}
			void run() override {
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

				DeviceControlWidget* parent = static_cast<DeviceControlWidget*>(this->parent());

				// If the device isn't connected, connect to it first. If we fail to connect, exit.
				if (!target_device_->get_device()->isOpen() && !target_device_->connect()) return;

				// Send start flag
				QByteArray out = QByteArray("ROMBEG", 6);
				target_device_->write((const char*)out, out.size());

				// Send Cuefile broken up into 64-bit chunks
				int index = 0;
				int sleep_period = 250;	// Wait 250 milliseconds between chunks
				do {
					out = parent->get_maestro_cue()->mid(index, 64);
					target_device_->write((const char*)out, out.size());
					index += 64;
					msleep(sleep_period);
					emit progress_changed((index / (float)parent->get_maestro_cue()->size()) * 100);
				}
				while (index < parent->get_maestro_cue()->size());

				// Send stop flag
				out = QByteArray("ROMEND", 6);
				target_device_->write((const char*)out, out.size());
				emit progress_changed(100);
			}

		signals:
			void progress_changed(int progress);

		private:
			SerialDevice* target_device_;
	};
}

#endif // DEVICECONTROLWIDGET_H
