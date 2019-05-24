#include <QByteArray>
#include "serialdevicethreadcontroller.h"

namespace PixelMaestroStudio {
	SerialDeviceThreadController::SerialDeviceThreadController(SerialDeviceController& serial_device, const char *out, int size) : QThread(nullptr), serial_device_(serial_device) {
		this->output_.append(out, size);
	}

	void SerialDeviceThreadController::run() {
		/*
		 * How this works:
		 *
		 * When sending data to an Arduino, the Arduino fails to process large chunks even when using a low baud rate.
		 * The Arduino's serial buffer overflows, causing data loss.
		 * This tends to happen at the 64 byte mark.
		 * As a workaround, we break up the output into 64 byte chunks and give the Arduino a few milliseconds between each chunk to catch up.
		 *
		 * FIXME: Not working as intended
		 *	Might be a serial timeout issue: https://stackoverflow.com/questions/32429327/slow-serial-communication-with-arduino-latency-of-almost-1-sec
		 *	Could also be an interrupt issue: https://www.reddit.com/r/arduino/comments/bmltcr/ws2812_not_responding_to_input_received_from/
		 *
		 * Resources:
		 * http://forum.arduino.cc/index.php?topic=124158.15
		 * https://forum.arduino.cc/index.php?topic=234151.0
		 */

		emit progress_changed(0);

		int current_index = 0;
		int chunk_index = CHUNK_SIZE;

		do {
			if (current_index > 0) {
				msleep(SLEEP_INTERVAL);
			}

			QByteArray out_addr = output_.mid(current_index, chunk_index);
			if (current_index + chunk_index > output_.size()) {
				chunk_index = output_.size() - current_index;
			}
			serial_device_.get_device()->write(out_addr);
			serial_device_.get_device()->flush();
			current_index += chunk_index;
			emit progress_changed((current_index / (float)output_.size()) * 100);
		}
		while (current_index < output_.size());

		emit progress_changed(100);
	}
}
