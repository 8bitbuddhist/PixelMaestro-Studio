#include "utility/cueinterpreter.h"
#include "showcontroller.h"

namespace PixelMaestroStudio {

	/**
	 * Constructor.
	 * @param maestro_controller Show's MaestroController.
	 */
	ShowController::ShowController(MaestroController* maestro_controller) {
		event_queue_.clear();
	}

	/**
	 * Adds an Event to the Show.
	 * @param time Event's start time.
	 * @param cue Event command.
	 * @return New Event.
	 */
	Event* ShowController::add_event(uint32_t time, uint8_t *cue) {
		if (cue != nullptr) {
			event_queue_.push_back(Event(time, cue));
		}

		return &event_queue_[event_queue_.size() - 1];
	}

	/**
	 * Deletes all events.
	 */
	void ShowController::clear() {
		event_queue_.clear();
	}

	/**
	 * Returns the index of the specified Event.
	 * @param event The Event to test.
	 * @return Event index (or -1 if not found)
	 */
	int ShowController::get_event_index(Event* event) {
		for (int i = 0; i < event_queue_.size(); i++) {
			Event test_event = event_queue_.at(i);
			if (test_event == *event) {
				return i;
			}
		}

		return -1;
	}

	/**
	 * Returns the list of Events.
	 * @return Event list.
	 */
	QVector<Event>* ShowController::get_events() {
		return &event_queue_;
	}

	/**
	 * Changes the location of an Event in the list.
	 * @param from Event's original location.
	 * @param to Event's final location.
	 */
	void ShowController::move(uint16_t from, uint16_t to) {
		event_queue_.move(from, to);
	}

	/**
	 * Removes the Event at the specified index.
	 * @param index Event index.
	 */
	void ShowController::remove_event(uint16_t index) {
		event_queue_.erase(event_queue_.begin() + index);
	}
}
