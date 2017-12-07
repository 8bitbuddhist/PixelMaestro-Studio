#include "core/maestro.h"
#include "cue/show.h"
#include "maestrocontroller.h"
#include <memory>
#include <QSettings>
#include "window/settingsdialog.h"

using namespace PixelMaestro;

/**
 * Empty constructor.
 */
MaestroController::MaestroController() : QObject(), timer_(this) {
	maestro_ = QSharedPointer<Maestro>(new Maestro(nullptr, 0));

	// Initialize timers
	timer_.setTimerType(Qt::PreciseTimer);
	/*
	 * Set timer's refresh rate to the Maestro's refresh rate.
	 * If we can't load the configured refresh rate, default to 40 (25fps)
	 */
	QSettings settings;
	int refresh = settings.value(SettingsDialog::refresh_rate, QVariant(40)).toInt();

	// Start timers
	timer_.start(refresh);
	elapsed_timer_.start();

	connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * Adds a DrawingArea to the list of DrawingAreas that the MaestroController renders to.
 * @param drawing_area New DrawingArea.
 */
void MaestroController::add_drawing_area(MaestroDrawingArea *drawing_area) {
	drawing_areas_.push_back(drawing_area);

	// Refresh the DrawingArea on each timeout
	connect(&timer_, SIGNAL(timeout()), drawing_area, SLOT(refresh()));
}

/**
 * Returns the Maestro handled by this MaestroController.
 * @return Underlying Maestro.
 */
Maestro* MaestroController::get_maestro() {
	return maestro_.data();
}

/**
 * Returns whether the Maestro is running.
 * @return True if Maestro is running.
 */
bool MaestroController::get_running() {
	return timer_.isActive();
}

/**
 * Returns the Show managed in this Maestro (if applicable)
 * @return Show managed by this Maestro.
 */
Show *MaestroController::get_show() {
	return maestro_->get_show();
}

/**
 * Gets the total elapsed time since the Maestro start.
 * @return Total elapsed time.
 */
uint64_t MaestroController::get_total_elapsed_time() {
	if (get_running()) {
		return last_pause_ + elapsed_timer_.elapsed();
	}
	else {
		return last_pause_;
	}
}

/**
 * Re-initializes the Section array.
 */
void MaestroController::reset_sections() {
	delete [] sections_;
	set_sections(num_sections_);
}

/**
 * Initializes the Maestro's Sections.
 * @param num_sections Number of Sections to apply.
 * @param dimensions The size of each Section.
 * @return List of new Sections.
 */
Section* MaestroController::set_sections(uint8_t num_sections, Point dimensions) {
	this->sections_ = new Section[num_sections];
	this->num_sections_ = num_sections;

	// Sets the size of each Section
	for (uint8_t section = 0; section < num_sections; section++) {
		sections_[section].set_dimensions(dimensions.x, dimensions.y);
	}

	maestro_->set_sections(sections_, num_sections_);

	return this->sections_;
}

void MaestroController::start() {
	elapsed_timer_.restart();
	timer_.start();
}

void MaestroController::stop() {
	last_pause_ += elapsed_timer_.elapsed();
	timer_.stop();
}

void MaestroController::update() {
	// Force the Maestro to update
	maestro_->update(get_total_elapsed_time(), true);
}

MaestroController::~MaestroController() {
	delete [] sections_;
}
