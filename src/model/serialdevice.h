/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QMessageBox>
#include <QSharedPointer>
#include <QSerialPort>
#include <QString>
#include <QThread>

namespace PixelMaestroStudio {
	class SerialDevice {
		public:
			SerialDevice();
			explicit SerialDevice(QString port_name);
			bool connect();
			bool disconnect();
			QSerialPort* get_device();
			int get_capacity() const;
			QString get_port_name() const;
			bool get_real_time_refresh_enabled() const;
			void set_capacity(int capacity);
			void set_real_time_update(bool enabled);

		private:
			/// The maximum number of bytes the device's ROM can hold.
			int capacity_ = 1024;

			/// The full path to the device (QSerialPortInfo::systemLocation()).
			QString port_name_;

			/// If true, commands will be sent to the device in real-time.
			bool real_time_updates_ = false;

			/// The actual QSerialPort object.
			QSharedPointer<QSerialPort> serial_device_ = QSharedPointer<QSerialPort>(new QSerialPort());

	};

	class SerialWriteThread : public QThread {
		Q_OBJECT

		public:
			SerialWriteThread(SerialDevice* device, const char* out, uint16_t size) : QThread(nullptr) {
				this->output_.append(out, size);
				this->device_ = device;
			}

			void run() override {
				/*
				 * How this works:
				 *
				 * When sending data to an Arduino, the Arduino fails to process large chunks even when using a low baud rate.
				 * The Arduino's serial buffer overflows, causing data loss.
				 * This tends to happen at the 64 byte mark.
				 * As a workaround, we break up the output into 64 byte chunks and give the Arduino a few milliseconds between each chunk to catch up.
				 *
				 * Resources:
				 * http://forum.arduino.cc/index.php?topic=124158.15
				 * https://forum.arduino.cc/index.php?topic=234151.0
				 */

				emit progress_changed(0);
				int current_index = 0;
				int chunk_size = 64;	// Size of each chunk in bytes
				int sleep_period = 250;	// Time in milliseconds between chunks

				try {
					do {
						QByteArray out_addr = output_.mid(current_index, chunk_size);	// TODO: Segfault on write when opening PMS with a saved device attached
						if (current_index + chunk_size > output_.size()) {
							chunk_size = output_.size() - current_index;
						}
						device_->get_device()->write(out_addr);
						device_->get_device()->flush();
						current_index += chunk_size;
						msleep(sleep_period);
						emit progress_changed((current_index / (float)output_.size()) * 100);
					}
					while (current_index < output_.size());
				}
				catch (std::exception& ex) {
					QMessageBox::critical(nullptr, QString("Device Error"), QString("Unable to write to device: " + QString::fromLatin1(ex.what())));
				}
				emit progress_changed(100);
			}

		signals:
			void progress_changed(int progress);

		private:
			SerialDevice* device_ = nullptr;
			QByteArray output_;
	};
}

#endif // SERIALDEVICE_H
