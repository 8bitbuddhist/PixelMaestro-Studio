#ifndef SERIALDEVICETHREAD_H
#define SERIALDEVICETHREAD_H

#include <QByteArray>
#include <QThread>
#include "serialdevicecontroller.h"

namespace PixelMaestroStudio {
	class SerialDeviceThreadController : public QThread {
		Q_OBJECT

		public:
			SerialDeviceThreadController(SerialDeviceController* serial_device, const char* out, int size);
			void run() override;

		signals:
			void progress_changed(int progress);

		private:
			/// Size of each chunk in bytes
			const uint8_t CHUNK_SIZE = 64;

			/// Time in milliseconds between chunks. Default is 250.
			const uint8_t SLEEP_INTERVAL = 250;

			SerialDeviceController* serial_device_ = nullptr;
			QByteArray output_;
	};
}

#endif // SERIALDEVICETHREAD_H
