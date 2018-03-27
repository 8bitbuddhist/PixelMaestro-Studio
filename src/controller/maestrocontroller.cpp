#include "animation/fireanimation.h"
#include "animation/lightninganimation.h"
#include "animation/plasmaanimation.h"
#include "animation/radialanimation.h"
#include "animation/sparkleanimation.h"
#include "animation/waveanimation.h"
#include "canvas/animationcanvas.h"
#include "canvas/canvas.h"
#include "canvas/canvastype.h"
#include "canvas/colorcanvas.h"
#include "canvas/palettecanvas.h"
#include "core/maestro.h"
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/maestrocuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/show.h"
#include "cue/showcuehandler.h"
#include "maestrocontroller.h"
#include <memory>
#include <QByteArray>
#include <QFile>
#include <QSettings>
#include "dialog/preferencesdialog.h"

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	/**
	 * Empty constructor.
	 */
	MaestroController::MaestroController() : QObject(), timer_(this) {
		// Initalize the Maestro
		maestro_ = QSharedPointer<Maestro>(new Maestro(nullptr, 0));
		QSettings settings;

		// Enable the Maestro's CueController
		CueController* controller = maestro_->set_cue_controller(UINT16_MAX);
		controller->enable_animation_cue_handler();
		controller->enable_canvas_cue_handler();
		controller->enable_maestro_cue_handler();
		controller->enable_section_cue_handler();
		controller->enable_show_cue_handler();

		// Initialize timers
		timer_.setTimerType(Qt::PreciseTimer);

		/*
		 * Set timer's refresh rate to the user's settings.
		 * If we can't load the configured refresh rate, default to 50ms (20fps)
		 */
		int refresh = settings.value(PreferencesDialog::refresh_rate, QVariant(50)).toInt();
		maestro_->set_timer(refresh);

		connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
	}

	/**
	 * Adds a DrawingArea to the list of DrawingAreas that the MaestroController renders to.
	 * @param drawing_area New DrawingArea.
	 */
	void MaestroController::add_drawing_area(MaestroDrawingArea *drawing_area) {
		drawing_areas_.push_back(drawing_area);

		// Refresh the DrawingArea on each timeout
		connect(&timer_, SIGNAL(timeout()), drawing_area, SLOT(update()));
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
	 * Removes a DrawingArea from the controller's render list. Automatically called in the DrawingArea's destructor.
	 * @param drawing_area DrawingArea to remove.
	 */
	void MaestroController::remove_drawing_area(MaestroDrawingArea *drawing_area) {
		disconnect(&timer_, SIGNAL(timeout()), drawing_area, SLOT(refresh()));
		drawing_areas_.removeOne(drawing_area);
	}

	/**
	 * Saves the Maestro to a Cuefile.
	 * @param filename Name of the Cuefile to save to.
	 */
	void MaestroController::save_cuefile(QString filename) {
		if (!filename.endsWith(".pmc", Qt::CaseInsensitive)) {
			filename.append(".pmc");
		}
		QFile file(filename);
		if (file.open(QFile::WriteOnly)) {
			QDataStream datastream(&file);
			save_maestro_to_datastream(&datastream);
			file.close();
		}
	}

	/**
	 * Saves Maestro settings to a DataStream as Cues.
	 * @param datastream Stream to save Cues to.
	 */
	void MaestroController::save_maestro_to_datastream(QDataStream *datastream) {
		// Timer
		MaestroCueHandler* maestro_handler = (MaestroCueHandler*)maestro_->get_cue_controller()->get_handler(CueController::Handler::MaestroCueHandler);
		write_cue_to_stream(datastream, maestro_handler->set_timer(maestro_->get_timer()->get_interval()));

		// Show
		Show* show = maestro_->get_show();
		if (show != nullptr) {
			write_cue_to_stream(datastream, maestro_handler->set_show());
			ShowCueHandler* show_handler = (ShowCueHandler*)maestro_->get_cue_controller()->get_handler(CueController::Handler::ShowCueHandler);
			write_cue_to_stream(datastream, show_handler->set_events(show->get_events(), show->get_num_events(), false));
			write_cue_to_stream(datastream, show_handler->set_looping(show->get_looping()));
			write_cue_to_stream(datastream, show_handler->set_timing_mode(show->get_timing()));
		}

		// Sections
		for (uint8_t section = 0; section < num_sections_; section++) {
			save_section_to_datastream(datastream, section, 0);
		}
	}

	/**
	 * Saves Section settings to a DataStream as Cues.
	 * @param datastream Stream to save Cues to.
	 * @param section_id The index of the Section to save.
	 * @param layer_id The index of the Layer to save.
	 */
	void MaestroController::save_section_to_datastream(QDataStream* datastream, uint8_t section_id, uint8_t layer_id) {

		Section* section = maestro_->get_section(section_id);

		if (layer_id > 0) {
			for (uint8_t i = 0; i < layer_id; i++) {
				section = section->get_layer()->section;
			}
		}

		SectionCueHandler* section_handler = (SectionCueHandler*)maestro_->get_cue_controller()->get_handler(CueController::Handler::SectionCueHandler);

		// Dimensions
		write_cue_to_stream(datastream, section_handler->set_dimensions(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y));

		// Animation & Colors
		Animation* animation = section->get_animation();
		if (animation != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_animation(section_id, layer_id, animation->get_type()));

			AnimationCueHandler* animation_handler = (AnimationCueHandler*)maestro_->get_cue_controller()->get_handler(CueController::Handler::AnimationCueHandler);
			if (animation->get_palette() != nullptr) {
				write_cue_to_stream(datastream, animation_handler->set_palette(section_id, layer_id, animation->get_palette()));
			}
			write_cue_to_stream(datastream, animation_handler->set_orientation(section_id, layer_id, animation->get_orientation()));
			write_cue_to_stream(datastream, animation_handler->set_reverse(section_id, layer_id, animation->get_reverse()));
			write_cue_to_stream(datastream, animation_handler->set_fade(section_id, layer_id, animation->get_fade()));

			if (animation->get_timer() != nullptr) {
				write_cue_to_stream(datastream, animation_handler->set_timer(section_id, layer_id, animation->get_timer()->get_interval(), animation->get_timer()->get_delay()));
			}

			// Save Animation-specific settings
			switch(animation->get_type()) {
				case AnimationType::Fire:
					{
						FireAnimation* fa = static_cast<FireAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_fire_options(section_id, layer_id, fa->get_multiplier()));
					}
					break;
				case AnimationType::Lightning:
					{
						LightningAnimation* la = static_cast<LightningAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_lightning_options(section_id, layer_id, la->get_bolt_count(), la->get_drift(), la->get_fork_chance()));
					}
					break;
				case AnimationType::Plasma:
					{
						PlasmaAnimation* pa = static_cast<PlasmaAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_plasma_options(section_id, layer_id, pa->get_size(), pa->get_resolution()));
					}
					break;
				case AnimationType::Radial:
					{
						RadialAnimation* ra = static_cast<RadialAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_radial_options(section_id, layer_id, ra->get_resolution()));
					}
					break;
				case AnimationType::Sparkle:
					{
						SparkleAnimation* sa = static_cast<SparkleAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_sparkle_options(section_id, layer_id, sa->get_threshold()));
					}
					break;
				case AnimationType::Wave:
					{
						WaveAnimation* wa = static_cast<WaveAnimation*>(animation);
						write_cue_to_stream(datastream, animation_handler->set_wave_options(section_id, layer_id, wa->get_mirror(), wa->get_skew()));
					}
				default:
					break;
			}
		}

		// Scrolling and offset
		write_cue_to_stream(datastream, section_handler->set_offset(section_id, layer_id, section->get_offset()->x, section->get_offset()->y));
		if (section->get_scroll()) {
			// FIXME: This is the scroll, not the actual interval. You can only get the interval by reversing the step or timer calculations.
			write_cue_to_stream(datastream, section_handler->set_scroll(section_id, layer_id, section->get_scroll()->step_x, section->get_scroll()->step_y));
		}

		// Save Canvas settings
		Canvas* canvas = section->get_canvas();
		if (canvas != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_canvas(section_id, layer_id, canvas->get_type(), canvas->get_num_frames()));

			CanvasCueHandler* canvas_handler = (CanvasCueHandler*)maestro_->get_cue_controller()->get_handler(CueController::Handler::CanvasCueHandler);

			if (canvas->get_frame_timer()) {
				write_cue_to_stream(datastream, canvas_handler->set_frame_timer(section_id, layer_id, canvas->get_frame_timer()->get_interval()));
			}

			// Save the PaletteCanvas' palette
			if (canvas->get_type() == CanvasType::PaletteCanvas) {
				write_cue_to_stream(datastream, canvas_handler->set_palette(section_id, layer_id, static_cast<PaletteCanvas*>(canvas)->get_palette()));
			}

			// Draw and save each frame
			for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
				switch (canvas->get_type()) {
					case CanvasType::AnimationCanvas:
						write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<AnimationCanvas*>(canvas)->get_frame(frame)));
						break;
					case CanvasType::ColorCanvas:
						write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<ColorCanvas*>(canvas)->get_frame(frame)));
						break;
					case CanvasType::PaletteCanvas:
						write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, section->get_dimensions()->x, section->get_dimensions()->y, static_cast<PaletteCanvas*>(canvas)->get_frame(frame)));
				}
				if (canvas->get_current_frame_index() != canvas->get_num_frames() - 1) {
					write_cue_to_stream(datastream, canvas_handler->next_frame(section_id, layer_id));
				}
			}
			write_cue_to_stream(datastream, canvas_handler->set_current_frame_index(section_id, layer_id, 0));
		}

		// Layers
		Section::Layer* layer = section->get_layer();
		if (layer != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_layer(section_id, layer_id, layer->mix_mode, layer->alpha));
			save_section_to_datastream(datastream, section_id, layer_id + 1);
		}
	}

	/**
	 * Initializes the Maestro's Sections.
	 * @param num_sections Number of Sections to apply.
	 * @param dimensions The size of each Section.
	 * @return Array of new Sections.
	 */
	Section* MaestroController::set_sections(uint8_t num_sections, Point dimensions) {
		this->sections_ = new Section[num_sections];
		this->num_sections_ = num_sections;

		// Sets the size of each Section
		for (uint8_t section = 0; section < num_sections; section++) {
			sections_[section].set_dimensions(dimensions.x, dimensions.y);
		}

		// Add a SectionDrawingArea to each MaestroDrawingArea
		for (MaestroDrawingArea* drawing_area : drawing_areas_) {
			for (uint8_t section = 0; section < num_sections; section++) {
				drawing_area->add_section_drawing_area(&sections_[section]);
			}
		}

		maestro_->set_sections(sections_, num_sections_);

		return this->sections_;
	}

	void MaestroController::start() {
		elapsed_timer_.restart();
		timer_.start(this->maestro_->get_timer()->get_interval());
	}

	void MaestroController::stop() {
		last_pause_ += elapsed_timer_.elapsed();
		timer_.stop();
	}

	void MaestroController::update() {
		// Force the Maestro to update
		maestro_->update(get_total_elapsed_time(), true);
	}

	/**
	 * Appends a Cue to a stream.
	 * @param stream Stream to append to.
	 * @param cue Cue to append.
	 */
	void MaestroController::write_cue_to_stream(QDataStream* stream, uint8_t* cue) {
		if (cue == nullptr) {
			return;
		}
		stream->writeRawData((const char*)cue, maestro_->get_cue_controller()->get_cue_size(cue));
	}

	MaestroController::~MaestroController() {
		delete [] sections_;
	}
}