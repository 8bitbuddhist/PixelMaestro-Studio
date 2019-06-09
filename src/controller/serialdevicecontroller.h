/*
 * SerialDevice - Utility class for managing devices connected via USB/Bluetooth.
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QSharedPointer>
#include <QSerialPort>
#include <QString>
#include "model/sectionmapmodel.h"

namespace PixelMaestroStudio {
	class SerialDeviceController {
		public:
			SerialDeviceController() = default;
			explicit SerialDeviceController(const QString& port_name);
			bool connect();
			bool disconnect();
			QSerialPort* get_device();
			int get_capacity() const;
			QString get_port_name() const;
			bool get_autoconnect() const;
			bool get_real_time_refresh_enabled() const;
			void set_autoconnect(const bool autoconnect);
			void set_capacity(const int capacity);
			void set_port_name(const QString &port_name);
			void set_real_time_update(const bool enabled);

			/// Custom mapping of local Sections to remote Sections. Made public because of weird pointer issues. Fix later.
			SectionMapModel* section_map_model = nullptr;

		private:
			/// Whether to connect to the device on startup.
			bool autoconnect_ = false;

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
