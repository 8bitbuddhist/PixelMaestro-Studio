#include "controller/cueinterpreter.h"
#include "showcontroller.h"

namespace PixelMaestroStudio {

	/**
	 * Constructor.
	 * @param maestro_controller Show's MaestroController.
	 */
	ShowController::ShowController(MaestroController* maestro_controller) {
		this->maestro_controller_ = maestro_controller;

		if (maestro_controller_->get_show() == nullptr) {
			this->show_ = maestro_controller_->get_maestro()->set_show(nullptr, 0);
		}
		else {
			this->show_ = maestro_controller_->get_maestro()->get_show();
		}

		events_.clear();
	}

	/**
	 * Adds an Event to the Show.
	 * @param time Event's start time.
	 * @param cue Event command.
	 * @return New Event.
	 */
	Event* ShowController::add_event(uint32_t time, uint8_t *cue) {
		if (cue != nullptr) {
			events_.push_back(Event(time, cue));
		}

		return &events_[events_.size() - 1];
	}

	/**
	 * Gets the Event at the specified index.
	 * @param index Event index.
	 * @return Event.
	 */
	Event* ShowController::get_event(uint16_t index) {
		if (index < events_.size()) {
			return &events_[index];
		}

		return nullptr;
	}

	/**
	 * Gets the list of Events.
	 * @return Event list.
	 */
	QVector<Event> ShowController::get_events() {
		return events_;
	}

	/**
	 * Applies the current Event list to the Maestro's Show.
	 */
	void ShowController::initialize_events() {
		show_->set_events(&events_[0], events_.size());
	}

	/**
	 * Removes the Event at the specified index.
	 * @param index Event index.
	 */
	void ShowController::remove_event(uint16_t index) {
		events_.erase(events_.begin() + index);
	}
}