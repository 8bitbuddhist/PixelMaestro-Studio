#include "sectioncontrolwidget.h"
#include "ui_sectioncontrolwidget.h"
#include "utility/canvasutility.h"

namespace PixelMaestroStudio {
	SectionControlWidget::SectionControlWidget(QWidget *parent) : QWidget(parent), ui(new Ui::SectionControlWidget) {
		ui->setupUi(this);
		this->maestro_control_widget_ = dynamic_cast<MaestroControlWidget*>(parent);
	}


	/**
	 * Returns the Section currently being edited.
	 * @return Selected Section.
	 */
	Section* SectionControlWidget::get_active_section() {
		return active_section_;
	}

	/**
	 * Returns the index of the active Layer.
	 * @return Layer index.
	 */
	uint8_t SectionControlWidget::get_layer_index() {
		return get_layer_index(active_section_);
	}

	/**
	 * Returns the index of the given Layer.
	 * @param section Section belonging to the Layer.
	 * @return Layer index.
	 */
	uint8_t SectionControlWidget::get_layer_index(Section* section) {
		/*
		 * Find the depth of the current Section by traversing parent_section_.
		 * Once parent_section == nullptr, we know we've hit the base Section.
		 */
		uint8_t level = 0;
		Section* test_section = section;
		while (test_section->get_parent_section() != nullptr) {
			test_section = test_section->get_parent_section();
			level++;
		}
		return level;
	}

	/**
	 * Gets the number of Layers belonging to the Section.
	 * @param section Section to scan.
	 * @return Number of Layers.
	 */
	uint8_t SectionControlWidget::get_num_layers(Section *section) {
		uint8_t count = 0;
		Section::Layer* layer = section->get_layer();
		while (layer != nullptr) {
			layer = layer->section->get_layer();
			count++;
		}
		return count;
	}

	/**
	 * Returns the index of the current Section.
	 * @return Current Section index (or 0 if not found).
	 */
	uint8_t SectionControlWidget::get_section_index() {
		return get_section_index(active_section_);
	}

	/**
	 * Returns the index of the specified Section.
	 * @param section Section to ID.
	 * @return Section index.
	 */
	uint8_t SectionControlWidget::get_section_index(Section* section) {
		Section* target_section = section;

		// If this is an Layer, iterate until we find the parent ID
		while (target_section->get_parent_section() != nullptr) {
			target_section = target_section->get_parent_section();
		}

		// Iterate until we find the Section that active_section_ points to
		uint8_t index = 0;
		Section* test_section = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(0);
		while (test_section != target_section) {
			index++;
			test_section = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(index);
		}

		return index;
	}

	void SectionControlWidget::initialize() {
		ui->activeSectionComboBox->blockSignals(true);
		ui->activeLayerComboBox->blockSignals(true);
		ui->alphaSpinBox->blockSignals(true);

		ui->activeSectionComboBox->clear();
		ui->activeLayerComboBox->clear();
		ui->alphaSpinBox->clear();

		// Set Section/Layer
		for (uint8_t section = 0; section < maestro_control_widget_->get_maestro_controller()->get_maestro()->get_num_sections(); section++) {
			ui->activeSectionComboBox->addItem(QString("Section ") + QString::number(section));
		}
		ui->activeSectionComboBox->setCurrentIndex(0);

		// Unblock signals (*grumble grumble*)
		ui->activeSectionComboBox->blockSignals(false);
		ui->activeLayerComboBox->blockSignals(false);
		ui->alphaSpinBox->blockSignals(false);

		set_active_section(maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(0));
		populate_layer_combobox();
	}

	/**
	 * Changes the active Layer.
	 * @param index Index of the desired Layer.
	 */
	void SectionControlWidget::on_activeLayerComboBox_currentIndexChanged(int index) {
		/*
		 * If we selected an Layer, iterate through the Section's nested Layers until we find it.
		 * If we selected 'None', use the base Section as the active Section.
		 */
		Section* layer_section = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(get_section_index());
		for (int i = 0; i < index; i++) {
			layer_section = layer_section->get_layer()->section;
		}

		// Show Layer controls
		set_layer_controls_enabled(index > 0);

		// Set active Section to Layer Section
		set_active_section(layer_section);
	}

	/**
	 * Changes the current Section.
	 * @param index Index of the target Section.
	 */
	void SectionControlWidget::on_activeSectionComboBox_currentIndexChanged(int index) {
		set_layer_controls_enabled(false);

		Section* section = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(index);

		set_active_section(section);
	}

	/**
	 * Sets the Layer's transparency level.
	 */
	void SectionControlWidget::on_alphaSpinBox_editingFinished() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->section_handler->set_layer(
				get_section_index(),
				get_layer_index(active_section_->get_parent_section()),
				active_section_->get_parent_section()->get_layer()->mix_mode,
				ui->alphaSpinBox->value()
			)
		);
	}

	void SectionControlWidget::on_brightnessSlider_valueChanged(int value) {
		ui->brightnessSpinBox->blockSignals(true);
		ui->brightnessSpinBox->setValue(value);
		ui->brightnessSpinBox->blockSignals(false);

		maestro_control_widget_->run_cue(
			maestro_control_widget_->section_handler->set_brightness(
				get_section_index(),
				get_layer_index(),
				static_cast<uint8_t>(value)
			)
		);
	}

	void SectionControlWidget::on_brightnessSpinBox_editingFinished() {
		uint8_t brightness = ui->brightnessSpinBox->value();
		ui->brightnessSlider->blockSignals(true);
		ui->brightnessSlider->setValue(brightness);
		ui->brightnessSlider->blockSignals(false);

		maestro_control_widget_->run_cue(
			maestro_control_widget_->section_handler->set_brightness(
				get_section_index(),
				get_layer_index(),
				static_cast<uint8_t>(brightness)
			)
		);
	}

	/**
	 * Sets the number of columns in the grid.
	 */
	void SectionControlWidget::on_gridSizeXSpinBox_editingFinished() {
		set_section_size();
	}

	/**
	 * Sets the number of rows in the grid.
	 */
	void SectionControlWidget::on_gridSizeYSpinBox_editingFinished() {
		set_section_size();
	}

	/**
	* Sets the number of Layers for the Section.
	 */
	void SectionControlWidget::on_layerSpinBox_editingFinished() {
		Section* base_section = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(get_section_index());
		Section* last_section = base_section;

		// Get the number of Layers (and the index of the last Layer) in the Section
		int num_layers = 0;
		while (last_section->get_layer() != nullptr) {
			last_section = last_section->get_layer()->section;
			num_layers++;
		}
		int last_layer_index = num_layers;

		/*
		 * Get the difference between the current Layer count and the new Layer count.
		 * If diff is positive, add more Layers. Otherwise, remove Layers.
		 */
		int diff = ui->layerSpinBox->value() - num_layers;
		if (diff > 0) {
			while (diff > 0) {
				maestro_control_widget_->run_cue(
					maestro_control_widget_->section_handler->set_layer(
						get_section_index(),
						last_layer_index,
						Colors::MixMode::None,
						0
					)
				);
				last_layer_index++;
				diff--;
			}
		}
		// If the diff is negative, remove Layers
		else if (diff < 0) {
			while (diff < 0) {
				last_section = last_section->get_parent_section();
				maestro_control_widget_->run_cue(
					maestro_control_widget_->section_handler->remove_layer(
						get_section_index(),
						last_layer_index - 1
					)
				);
				last_layer_index--;
				diff++;
			}

			// If the active Layer no longer exists, jump to the last available Layer
			if (ui->layerSpinBox->value() >= last_layer_index) {
				set_active_section(last_section);
			}
		}

		// Refresh layer combobox
		populate_layer_combobox();
	}

	/**
	 * Changes the Layer's mix mode.
	 * @param index Index of new mix mode.
	 */
	void SectionControlWidget::on_mixModeComboBox_currentIndexChanged(int index) {
		// Only continue if the active Section is a Layer.
		if (get_layer_index() == 0) return ;

		if ((Colors::MixMode)index != active_section_->get_parent_section()->get_layer()->mix_mode) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->section_handler->set_layer(
					get_section_index(),
					get_layer_index(active_section_->get_parent_section()),
					(Colors::MixMode)index,
					ui->alphaSpinBox->value()
				)
			);
		}

		// Enable spin box for alpha only
		ui->alphaLabel->setEnabled((Colors::MixMode)index == Colors::MixMode::Alpha);
		ui->alphaSpinBox->setEnabled((Colors::MixMode)index == Colors::MixMode::Alpha);
	}

	/// Recalculates offset when horizontal offset changes.
	void SectionControlWidget::on_offsetXSpinBox_editingFinished() {
		set_offset();
	}

	/// Recalculates offset when vertical offset changes.
	void SectionControlWidget::on_offsetYSpinBox_editingFinished() {
		set_offset();
	}

	/// Recalculates scroll when horizontal scroll rate changes.
	void SectionControlWidget::on_scrollXSpinBox_editingFinished() {
		set_scroll();
	}

	/// Recalculates scroll when vertical scroll rate changes.
	void SectionControlWidget::on_scrollYSpinBox_editingFinished() {
		set_scroll();
	}

	/**
	 * Updates UI controls based on the current active Section.
	 */
	void SectionControlWidget::refresh() {
		// If the current Section doesn't match the Section drop-down, update the drop-down
		int section_index = get_section_index();
		int layer_index = get_layer_index();
		if (ui->activeSectionComboBox->currentIndex() != section_index) {
			ui->activeSectionComboBox->blockSignals(true);
			ui->activeSectionComboBox->setCurrentIndex(section_index);
			ui->activeSectionComboBox->blockSignals(false);
		}

		// Update the Layer combo box based on our selection
		if (layer_index == 0) {
			// Set Layer count
			ui->layerSpinBox->blockSignals(true);
			ui->layerSpinBox->setValue(get_num_layers(active_section_));
			ui->layerSpinBox->blockSignals(false);

			populate_layer_combobox();
		}

		// Handle Layer combobox
		if (ui->activeLayerComboBox->currentIndex() != layer_index) {
			ui->activeLayerComboBox->blockSignals(true);
			ui->activeLayerComboBox->setCurrentIndex(layer_index);
			ui->activeLayerComboBox->blockSignals(false);
		}

		// Set dimensions
		ui->gridSizeXSpinBox->blockSignals(true);
		ui->gridSizeYSpinBox->blockSignals(true);
		// Yes, these are reversed. No, I don't know why it works.
		ui->gridSizeXSpinBox->setValue(active_section_->get_dimensions()->y);
		ui->gridSizeYSpinBox->setValue(active_section_->get_dimensions()->x);
		ui->gridSizeXSpinBox->blockSignals(false);
		ui->gridSizeYSpinBox->blockSignals(false);

		// Set offset and scroll
		ui->offsetXSpinBox->blockSignals(true);
		ui->offsetYSpinBox->blockSignals(true);
		ui->offsetXSpinBox->setValue(active_section_->get_offset()->x);
		ui->offsetYSpinBox->setValue(active_section_->get_offset()->y);
		ui->offsetXSpinBox->blockSignals(false);
		ui->offsetYSpinBox->blockSignals(false);

		ui->scrollXSpinBox->blockSignals(true);
		ui->scrollYSpinBox->blockSignals(true);
		int32_t interval_x = 0;
		int32_t interval_y = 0;
		if (active_section_->get_scroll() != nullptr) {
			Section::Scroll* scroll = active_section_->get_scroll();
			uint16_t refresh = maestro_control_widget_->get_maestro_controller()->get_maestro()->get_timer()->get_interval();
			// x axis
			if (scroll->timer_x != nullptr) {
				float x = refresh / (float)scroll->timer_x->get_interval();
				interval_x = (active_section_->get_dimensions()->x * refresh) / x;
			}
			else {
				if (scroll->step_x > 0) {
					interval_x = (active_section_->get_dimensions()->x * refresh) / (float)scroll->step_x;
				}
			}
			if (scroll->reverse_x) interval_x *= -1;

			// y axis
			if (scroll->timer_y != nullptr) {
				float y = refresh / (float)scroll->timer_y->get_interval();
				interval_y = (active_section_->get_dimensions()->y * refresh) / y;
			}
			else {
				if (scroll->step_y > 0) {
					interval_y = (active_section_->get_dimensions()->y * refresh) / (float)scroll->step_y;
				}
			}
			if (scroll->reverse_y) interval_y *= -1;
		}
		ui->scrollXSpinBox->setValue(interval_x);
		ui->scrollYSpinBox->setValue(interval_y);

		ui->scrollXSpinBox->blockSignals(false);
		ui->scrollYSpinBox->blockSignals(false);

		// Update brightness
		uint8_t brightness = active_section_->get_brightness();
		ui->brightnessSlider->blockSignals(true);
		ui->brightnessSpinBox->blockSignals(true);
		ui->brightnessSlider->setValue(brightness);
		ui->brightnessSpinBox->setValue(brightness);
		ui->brightnessSlider->blockSignals(false);
		ui->brightnessSpinBox->blockSignals(false);

		// If this is a Layer, get the MixMode and alpha
		if (active_section_->get_parent_section() != nullptr) {
			set_layer_controls_enabled(true);
			Section::Layer* layer = active_section_->get_parent_section()->get_layer();

			ui->mixModeComboBox->blockSignals(true);
			ui->alphaSpinBox->blockSignals(true);
			ui->mixModeComboBox->setCurrentIndex((uint8_t)layer->mix_mode);
			ui->alphaSpinBox->setValue(layer->alpha);
			ui->alphaSpinBox->setEnabled((Colors::MixMode)ui->mixModeComboBox->currentIndex() == Colors::MixMode::Alpha);
			ui->alphaLabel->setEnabled((Colors::MixMode)ui->mixModeComboBox->currentIndex() == Colors::MixMode::Alpha);
			ui->mixModeComboBox->blockSignals(false);
			ui->alphaSpinBox->blockSignals(false);
		}
		else {
			set_layer_controls_enabled(false);
		}
	}

	/**
	 * Rebuilds the Layer combo box using the current active Section.
	 */
	void SectionControlWidget::populate_layer_combobox() {
		ui->activeLayerComboBox->blockSignals(true);
		ui->activeLayerComboBox->clear();
		ui->activeLayerComboBox->addItem("Base Section");

		for (uint8_t layer = 0; layer < get_num_layers(maestro_control_widget_->get_maestro_controller()->get_maestro()->get_section(get_section_index())); layer++) {
			ui->activeLayerComboBox->addItem(QString("Layer ") + QString::number(layer + 1));
		}

		ui->activeLayerComboBox->blockSignals(false);
	}

	/**
	 * Changes the active Section.
	 * @param section New active Section.
	 */
	void SectionControlWidget::set_active_section(Section *section) {
		if (section == nullptr) return;
		active_section_ = section;

		maestro_control_widget_->refresh_section_settings();
	}

	/**
	 * Toggles whether Layer-related controls are active.
	 * @param enabled If true, Layer controls are enabled.
	 */
	void SectionControlWidget::set_layer_controls_enabled(bool enabled) {
		ui->mixModeLabel->setEnabled(enabled);
		ui->mixModeComboBox->setEnabled(enabled);
		ui->alphaLabel->setEnabled(enabled);
		ui->alphaSpinBox->setEnabled(enabled);
	}

	/**
	 * Sets the Section's offset.
	 * Note that this is disabled while scrolling is enabled.
	 */
	void SectionControlWidget::set_offset() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->section_handler->set_offset(
				get_section_index(),
				get_layer_index(),
				ui->offsetXSpinBox->value(),
				ui->offsetYSpinBox->value()
			)
		);
	}

	/**
	 * Sets the Section's scrolling pattern.
	 */
	void SectionControlWidget::set_scroll() {
		int new_x = ui->scrollXSpinBox->value();
		int new_y = ui->scrollYSpinBox->value();

		maestro_control_widget_->run_cue(
			maestro_control_widget_->section_handler->set_scroll(
				get_section_index(),
				get_layer_index(),
				Utility::abs_int(new_x),
				Utility::abs_int(new_y),
				(new_x < 0),
				(new_y < 0)
			)
		);

		// Disable offset controls if scrolling is enabled
		ui->offsetXSpinBox->setEnabled(new_x == 0);
		ui->offsetYSpinBox->setEnabled(new_y == 0);

		if (new_x == 0) {
			ui->offsetXSpinBox->blockSignals(true);
			ui->offsetXSpinBox->setValue(active_section_->get_offset()->x);
			ui->offsetYSpinBox->blockSignals(false);
		}
		if (new_y == 0) {
			ui->offsetYSpinBox->blockSignals(true);
			ui->offsetYSpinBox->setValue(active_section_->get_offset()->y);
			ui->offsetYSpinBox->blockSignals(false);
		}
	}

	/**
	 * Sets the size of the active Section and its Layers.
	 */
	void SectionControlWidget::set_section_size() {
		// Only resize if the dimensions are different
		Point new_dimensions(ui->gridSizeXSpinBox->value(), ui->gridSizeYSpinBox->value());
		if (new_dimensions != *active_section_->get_dimensions()) {
			/*
			 * Check for a Canvas.
			 * The old Canvas gets deleted on resize.
			 * If one is set, copy its contents to a temporary buffer, then copy it back once the new Canvas is created.
			 */
			Canvas* canvas = active_section_->get_canvas();
			if (canvas != nullptr) {
				Point frame_bounds(active_section_->get_dimensions()->x, active_section_->get_dimensions()->y);

				uint8_t** frames = new uint8_t*[canvas->get_num_frames()];
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					frames[frame] = new uint8_t[frame_bounds.size()];
				}
				CanvasUtility::copy_from_canvas(canvas, frames, frame_bounds.x, frame_bounds.y);

				maestro_control_widget_->run_cue(
					maestro_control_widget_->section_handler->set_dimensions(
						get_section_index(),
						0,
						new_dimensions.x,
						new_dimensions.y
					)
				);

				CanvasUtility::copy_to_canvas(canvas, frames, frame_bounds.x, frame_bounds.y, maestro_control_widget_);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
			else {	// No Canvas set
				maestro_control_widget_->run_cue(
					maestro_control_widget_->section_handler->set_dimensions(
						get_section_index(),
						0,
						new_dimensions.x,
						new_dimensions.y
					)
				);
			}
		}
	}

	SectionControlWidget::~SectionControlWidget() {
		delete ui;
	}
}
