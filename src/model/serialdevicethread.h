#ifndef SERIALDEVICETHREAD_H
#define SERIALDEVICETHREAD_H

#include <QByteArray>
#include <QThread>
#include "serialdevice.h"

namespace PixelMaestroStudio {
	class SerialDeviceThread : public QThread {
		Q_OBJECT

		public:
			SerialDeviceThread(SerialDevice* serial_device, const char* out, uint16_t size);
			void run() override;

		signals:
			void progress_changed(int progress);

		private:
			SerialDevice* serial_device_ = nullptr;
			QByteArray output_;
	};
}

#endif // SERIALDEVICETHREAD_H
