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
#include "model/serialdevice.h"
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
			void on_connectPushButton_clicked();
			void on_interpretCuePushButton_clicked();
			void on_realTimeCheckBox_stateChanged(int arg1);
			void on_disconnectPushButton_clicked();
			void on_sendPushButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);
			void on_capacityLineEdit_editingFinished();

			void set_progress_bar(int val);
			void set_upload_controls_enabled(bool enabled);

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
				 * When sending a Cuefile to an Arduino, Qt sends data way too fast, even with a low baud rate.
				 * The Arduino's serial buffer overflows and it starts dropping Cues.
				 * Basically, the Cuefile gets cut off at the 65+ byte mark and the entire thing fails.
				 * The reason it works for live mode is probably because we're sending individual Cues (which are typically 20-30 bytes) and not entire Cuefiles (which are 200+ bytes minimum).
				 * As a workaround, we break up the Cuefile into 64 byte chunks and give the Arduino a few milliseconds between each chunk to catch up.
				 *
				 * The final message looks like this:
				 *	1) "ROMBEG", which signals the Arduino to start writing to EEPROM.
				 *	2) The Cuefile itself split into 64 byte chunks.
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

				// Start upload process and send start flag
				emit upload_in_progress(true);
				QByteArray out = QByteArray("ROMBEG", 6);
				target_device_->write((const char*)out, out.size());

				// Send Cuefile size first
				IntByteConvert size(parent->get_maestro_cue()->size());
				uint8_t size_arr[] = {size.converted_0, size.converted_1};
				target_device_->write((const char*)size_arr, 2);

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

				// End upload process and send stop flag
				out = QByteArray("ROMEND", 6);
				target_device_->write((const char*)out, out.size());
				emit progress_changed(100);
				emit upload_in_progress(false);
			}

		signals:
			void progress_changed(int progress);
			void upload_in_progress(bool in_progress);

		private:
			SerialDevice* target_device_;
	};
}

#endif // DEVICECONTROLWIDGET_H
