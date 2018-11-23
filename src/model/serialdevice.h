/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QSharedPointer>
#include <QSerialPort>
#include <QString>

namespace PixelMaestroStudio {
	class SerialDevice {
		public:
			SerialDevice() = default;
			explicit SerialDevice(const QString& port_name);
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
}

#endif // SERIALDEVICE_H
