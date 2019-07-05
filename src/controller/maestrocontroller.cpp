#include "animation/fireanimation.h"
#include "animation/lightninganimation.h"
#include "animation/plasmaanimation.h"
#include "animation/radialanimation.h"
#include "animation/sparkleanimation.h"
#include "animation/waveanimation.h"
#include "canvas/canvas.h"
#include "core/maestro.h"
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/maestrocuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/show.h"
#include "cue/showcuehandler.h"
#include "maestrocontroller.h"
#include <QByteArray>
#include <QSettings>
#include "dialog/preferencesdialog.h"

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	/**
	 * Initializes the MaestroController.
	 * @param maestro_control_widget The widget responsible for controlling this MaestroController.
	 */
	MaestroController::MaestroController(MaestroControlWidget& maestro_control_widget) : timer_(this), maestro_control_widget_(maestro_control_widget) {
		initialize_maestro();

		// Initialize timers
		timer_.setTimerType(Qt::PreciseTimer);
		connect(&timer_, SIGNAL(timeout()), this, SLOT(update()));
	}

	/**
	 * Adds a DrawingArea to the list of DrawingAreas that the MaestroController renders to.
	 * @param drawing_area New DrawingArea.
	 */
	void MaestroController::add_drawing_area(MaestroDrawingArea& drawing_area) {
		drawing_areas_.push_back(&drawing_area);

		// Initialize the DrawingArea's SectionDrawingAreas
		for (uint8_t section = 0; section < this->num_sections_; section++) {
			drawing_area.add_section_drawing_area(sections_[section], section);
		}

		// Refresh the DrawingArea on each timeout
		connect(&timer_, SIGNAL(timeout()), &drawing_area, SLOT(update()));
	}

	/**
	 * Returns the Maestro handled by this MaestroController.
	 * @return Underlying Maestro.
	 */
	Maestro& MaestroController::get_maestro() {
		return *maestro_.data();
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
		uint64_t elapsed = last_pause_;

		if (get_running()) {
			elapsed += elapsed_timer_.elapsed();
		}

		return elapsed;
	}

	/**
	 * Resets the Maestro.
	 */
	void MaestroController::initialize_maestro() {
		if (!maestro_.isNull()) {
			maestro_.reset();
		}

		// Initalize the Maestro with the specified number of Sections
		maestro_ = QSharedPointer<Maestro>(new Maestro(nullptr, 0));
		QSettings settings;
		set_sections(settings.value(PreferencesDialog::num_sections, 1).toInt());

		/*
		 * Set the Maestro's refresh timer.
		 * If the user hasn't set a custom timer, default to 50ms (20fps).
		 */
		int refresh = settings.value(PreferencesDialog::refresh_rate, QVariant(50)).toInt();
		maestro_->set_timer(refresh);

		// Enable the Maestro's CueController and CueHandlers
		CueController& controller = maestro_->set_cue_controller(UINT16_MAX);
		controller.enable_animation_cue_handler();
		controller.enable_canvas_cue_handler();
		controller.enable_maestro_cue_handler();
		controller.enable_section_cue_handler();
		controller.enable_show_cue_handler();
	}

	/**
	 * Removes a DrawingArea from the controller's render list. Automatically called in the DrawingArea's destructor.
	 * @param drawing_area DrawingArea to remove.
	 */
	void MaestroController::remove_drawing_area(MaestroDrawingArea& drawing_area) {
		disconnect(&timer_, SIGNAL(timeout()), &drawing_area, SLOT(update()));
		drawing_areas_.removeOne(&drawing_area);
	}

	/**
	 * Saves Maestro settings to a DataStream as Cues.
	 * @param datastream Stream to save Cues to.
	 * @param save_handlers CueHandlers that are enabled for saving.
	 */
	void MaestroController::save_maestro_to_datastream(QDataStream& datastream, QVector<CueController::Handler>* save_handlers) {
		MaestroCueHandler* maestro_handler = dynamic_cast<MaestroCueHandler*>(maestro_->get_cue_controller().get_handler(CueController::Handler::MaestroCueHandler));

		// Maestro-specific Cues
		if (save_handlers == nullptr || save_handlers->contains(CueController::Handler::MaestroCueHandler)) {
			QSettings settings;
			uint16_t interval = static_cast<uint16_t>(settings.value(PreferencesDialog::refresh_rate, 50).toUInt());
			if (maestro_->get_timer().get_interval() != interval) {
				write_cue_to_stream(datastream, maestro_handler->set_timer(interval));
			}
		}

		// Show-specific Cues
		if (save_handlers == nullptr || save_handlers->contains(CueController::Handler::ShowCueHandler)) {
			Show* show = maestro_->get_show();
			if (show != nullptr) {
				write_cue_to_stream(datastream, maestro_handler->set_show());
				ShowCueHandler* show_handler = dynamic_cast<ShowCueHandler*>(maestro_->get_cue_controller().get_handler(CueController::Handler::ShowCueHandler));
				if (show->get_events() != nullptr) {
					write_cue_to_stream(datastream, show_handler->set_events(show->get_events(), show->get_num_events(), false));
				}
				write_cue_to_stream(datastream, show_handler->set_looping(show->get_looping()));
				write_cue_to_stream(datastream, show_handler->set_timing_mode(show->get_timing()));
			}
		}

		// Call Sections
		if (save_handlers == nullptr || save_handlers->contains(CueController::Handler::SectionCueHandler)) {
			for (uint8_t section = 0; section < num_sections_; section++) {
				save_section_to_datastream(datastream, section, 0, save_handlers);
			}
		}
	}

	/**
	 * Saves Section settings to a DataStream as Cues.
	 * @param datastream Stream to save Cues to.
	 * @param section_id The index of the Section to save.
	 * @param layer_id The index of the Layer to save.
	 * @param save_handlers CueHandlers that are enabled for saving. If null, save all Cues
	 */
	void MaestroController::save_section_to_datastream(QDataStream& datastream, uint8_t section_id, uint8_t layer_id, QVector<CueController::Handler>* save_handlers) {

		Section* section = maestro_->get_section(section_id);
		if (section == nullptr) return;

		if (layer_id > 0) {
			for (uint8_t i = 0; i < layer_id; i++) {
				section = section->get_layer()->section;
			}
		}

		SectionCueHandler* section_handler = dynamic_cast<SectionCueHandler*>(maestro_->get_cue_controller().get_handler(CueController::Handler::SectionCueHandler));

		// Global Section settings
		if (section->get_brightness() != 255) {
			write_cue_to_stream(datastream, section_handler->set_brightness(section_id, layer_id, section->get_brightness()));
		}
		write_cue_to_stream(datastream, section_handler->set_dimensions(section_id, layer_id, section->get_dimensions().x, section->get_dimensions().y));

		// Animation & Colors
		if (save_handlers == nullptr || save_handlers->contains(CueController::Handler::AnimationCueHandler)) {
			Animation* animation = section->get_animation();
			if (animation != nullptr) {
				write_cue_to_stream(datastream, section_handler->set_animation(section_id, layer_id, animation->get_type()));

				AnimationCueHandler* animation_handler = dynamic_cast<AnimationCueHandler*>(maestro_->get_cue_controller().get_handler(CueController::Handler::AnimationCueHandler));
				if (animation->get_palette() != nullptr) {
					write_cue_to_stream(datastream, animation_handler->set_palette(section_id, layer_id, *animation->get_palette()));
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
							FireAnimation* fa = dynamic_cast<FireAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_fire_options(section_id, layer_id, fa->get_multiplier()));
						}
						break;
					case AnimationType::Lightning:
						{
							LightningAnimation* la = dynamic_cast<LightningAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_lightning_options(section_id, layer_id, la->get_bolt_count(), la->get_drift(), la->get_fork_chance()));
						}
						break;
					case AnimationType::Plasma:
						{
							PlasmaAnimation* pa = dynamic_cast<PlasmaAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_plasma_options(section_id, layer_id, pa->get_size(), pa->get_resolution()));
						}
						break;
					case AnimationType::Radial:
						{
							RadialAnimation* ra = dynamic_cast<RadialAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_radial_options(section_id, layer_id, ra->get_resolution()));
						}
						break;
					case AnimationType::Sparkle:
						{
							SparkleAnimation* sa = dynamic_cast<SparkleAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_sparkle_options(section_id, layer_id, sa->get_threshold()));
						}
						break;
					case AnimationType::Wave:
						{
							WaveAnimation* wa = dynamic_cast<WaveAnimation*>(animation);
							write_cue_to_stream(datastream, animation_handler->set_wave_options(section_id, layer_id, wa->get_skew()));
						}
						break;
					default:
						break;
				}
			}
		}

		// Scrolling, offset, and mirroring
		Point& offset = section->get_offset();
		if (offset.x != 0 || offset.y != 0) {
			write_cue_to_stream(datastream, section_handler->set_offset(section_id, layer_id, offset.x, offset.y));
		}
		Section::Scroll* scroll = section->get_scroll();
		if (scroll != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_scroll(section_id, layer_id, scroll->interval_x, scroll->interval_y, scroll->reverse_x, scroll->reverse_y));
		}
		Section::Mirror* mirror = section->get_mirror();
		if (mirror != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_mirror(section_id, layer_id, mirror->x, mirror->y));
		}

		// Save Canvas settings
		if (save_handlers == nullptr || save_handlers->contains(CueController::Handler::CanvasCueHandler)) {
			Canvas* canvas = section->get_canvas();
			if (canvas != nullptr) {
				write_cue_to_stream(datastream, section_handler->set_canvas(section_id, layer_id, canvas->get_num_frames()));

				CanvasCueHandler* canvas_handler = dynamic_cast<CanvasCueHandler*>(maestro_->get_cue_controller().get_handler(CueController::Handler::CanvasCueHandler));

				if (canvas->get_frame_timer()) {
					write_cue_to_stream(datastream, canvas_handler->set_frame_timer(section_id, layer_id, canvas->get_frame_timer()->get_interval()));
				}

				if (canvas->get_palette() != nullptr) {
					write_cue_to_stream(datastream, canvas_handler->set_palette(section_id, layer_id, *canvas->get_palette()));
				}

				// Draw and save each frame
				write_cue_to_stream(datastream, canvas_handler->set_num_frames(section_id, layer_id, canvas->get_num_frames()));
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					write_cue_to_stream(datastream, canvas_handler->draw_frame(section_id, layer_id, frame, section->get_dimensions().x, section->get_dimensions().y, canvas->get_frame(frame)));
				}
				write_cue_to_stream(datastream, canvas_handler->set_current_frame_index(section_id, layer_id, canvas->get_current_frame_index()));
			}
		}

		// Layers
		Section::Layer* layer = section->get_layer();
		if (layer != nullptr) {
			write_cue_to_stream(datastream, section_handler->set_layer(section_id, layer_id, layer->mix_mode, layer->alpha));
			save_section_to_datastream(datastream, section_id, layer_id + 1, save_handlers);
		}
	}

	/**
	 * Initializes the Maestro's Sections.
	 * @param num_sections Number of Sections to apply.
	 * @param dimensions The size of each Section.
	 * @return Array of new Sections.
	 */
	Section* MaestroController::set_sections(uint8_t num_sections, Point dimensions) {
		delete [] this->sections_;
		this->sections_ = new Section[num_sections];
		this->num_sections_ = num_sections;

		// Sets the size of each Section
		for (uint8_t section = 0; section < num_sections; section++) {
			sections_[section].set_dimensions(dimensions.x, dimensions.y);
		}

		maestro_->set_sections(sections_, num_sections_);

		// Reset each drawing area's Sections
		for (MaestroDrawingArea* drawing_area : this->drawing_areas_) {
			drawing_area->remove_section_drawing_areas();
			for (uint8_t section = 0; section < num_sections; section++) {
				drawing_area->add_section_drawing_area(this->sections_[section], section);
			}
		}

		// Reset the MaestroControlWidget's active section

		return this->sections_;
	}

	void MaestroController::start() {
		elapsed_timer_.restart();
		timer_.start(this->maestro_->get_timer().get_interval());
	}

	void MaestroController::stop() {
		last_pause_ += elapsed_timer_.elapsed();
		timer_.stop();
	}

	void MaestroController::update() {
		maestro_->update(get_total_elapsed_time(), false);
	}

	/**
	 * Appends a Cue to a stream.
	 * @param stream Stream to append to.
	 * @param cue Cue to append.
	 */
	void MaestroController::write_cue_to_stream(QDataStream& stream, uint8_t* cue) {
		if (cue != nullptr) {
			stream.writeRawData((const char*)cue, maestro_->get_cue_controller().get_cue_size(cue));
		}
	}

	MaestroController::~MaestroController() {
		// If automatic session saving is enabled, save Maestro configuration
		QSettings settings;
		if (settings.value(PreferencesDialog::save_session).toBool()) {
			QByteArray maestro_config;
			QDataStream maestro_datastream(&maestro_config, QIODevice::Truncate);
			save_maestro_to_datastream(maestro_datastream);
			settings.setValue(PreferencesDialog::last_session, maestro_config);
		}


		delete [] sections_;
	}
}
