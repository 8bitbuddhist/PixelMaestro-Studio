#include "sectioncontrolwidget.h"
#include "ui_sectioncontrolwidget.h"
#include "utility/canvasutility.h"

namespace PixelMaestroStudio {
	SectionControlWidget::SectionControlWidget(QWidget *parent) :
			QWidget(parent),
			ui(new Ui::SectionControlWidget),
			maestro_control_widget_(*dynamic_cast<MaestroControlWidget*>(parent)) {
		ui->setupUi(this);
	}

	void SectionControlWidget::on_addLayerButton_clicked() {
		// Get the number of Layers (and the index of the last Layer) in the Section
		Section* base_section = maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(get_section_index());
		int num_layers = 0;
		while (base_section->get_layer() != nullptr) {
			base_section = base_section->get_layer()->section;
			num_layers++;
		}
		uint8_t layer_index = ui->layerListWidget->count() - 1;

		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_layer(
				get_section_index(),
				layer_index,
				Colors::MixMode::None,
				0
			)
		);

		ui->layerListWidget->addItem(QString("Layer ") + QString::number(layer_index + 1));
		ui->layerListWidget->setCurrentRow(layer_index + 1);
	}

	/**
	 * Returns the Section currently being edited.
	 * @return Selected Section.
	 */
	Section& SectionControlWidget::get_active_section() {
		return *active_section_;
	}

	/**
	 * Returns the index of the active Layer.
	 * @return Layer index.
	 */
	uint8_t SectionControlWidget::get_layer_index() {
		return get_layer_index(*active_section_);
	}

	/**
	 * Returns the index of the given Layer.
	 * @param section Section belonging to the Layer.
	 * @return Layer index.
	 */
	uint8_t SectionControlWidget::get_layer_index(Section& section) {
		/*
		 * Find the depth of the current Section by traversing parent_section_.
		 * Once parent_section == nullptr, we know we've hit the base Section.
		 */
		uint8_t level = 0;
		Section* test_section = &section;
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
	uint8_t SectionControlWidget::get_num_layers(Section& section) {
		uint8_t count = 0;
		Section::Layer* layer = section.get_layer();
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
		return get_section_index(*active_section_);
	}

	/**
	 * Returns the index of the specified Section.
	 * @param section Section to ID.
	 * @return Section index.
	 */
	uint8_t SectionControlWidget::get_section_index(Section& section) {
		Section* target_section = &section;

		// If this is an Layer, iterate until we find the parent ID
		while (target_section->get_parent_section() != nullptr) {
			target_section = target_section->get_parent_section();
		}

		// Iterate until we find the Section that active_section_ points to
		uint8_t index = 0;
		Section* test_section = maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(0);
		while (test_section != target_section) {
			index++;
			test_section = maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(index);
		}

		return index;
	}

	void SectionControlWidget::initialize() {
		ui->alphaSpinBox->blockSignals(true);
		ui->alphaSpinBox->clear();
		ui->alphaSpinBox->blockSignals(false);

		// Build Section list
		ui->sectionListWidget->blockSignals(true);
		ui->sectionListWidget->clear();
		for (uint16_t section = 0; section < maestro_control_widget_.get_maestro_controller()->get_maestro().get_num_sections(); section++) {
			ui->sectionListWidget->addItem(QString("Section ") + QString::number(section + 1));
		}
		ui->sectionListWidget->blockSignals(false);

		set_active_section(maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(0));
		populate_layer_list();
	}

	/**
	 * Sets the Layer's transparency level.
	 */
	void SectionControlWidget::on_alphaSpinBox_editingFinished() {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_layer(
				get_section_index(),
				get_layer_index(*active_section_->get_parent_section()),
				active_section_->get_parent_section()->get_layer()->mix_mode,
				ui->alphaSpinBox->value()
			)
		);
	}

	void SectionControlWidget::on_brightnessSlider_valueChanged(int value) {
		ui->brightnessSpinBox->blockSignals(true);
		ui->brightnessSpinBox->setValue(value);
		ui->brightnessSpinBox->blockSignals(false);

		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_brightness(
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

		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_brightness(
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

	void SectionControlWidget::on_layerListWidget_currentRowChanged(int currentRow) {
		/*
		 * If we selected an Layer, iterate through the Section's nested Layers until we find it.
		 * If we selected 'None', use the base Section as the active Section.
		 */
		Section* layer_section = maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(get_section_index());
		for (int i = 0; i < currentRow; i++) {
			layer_section = layer_section->get_layer()->section;
		}

		// Show Layer controls
		set_layer_controls_enabled(currentRow > 0);

		// Set active Section to Layer Section
		set_active_section(layer_section);
	}

	void SectionControlWidget::on_mirrorXCheckBox_toggled(bool checked) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_mirror(
				get_section_index(),
				get_layer_index(),
				checked,
				ui->mirrorYCheckBox->isChecked()
			)
		);
	}

	void SectionControlWidget::on_mirrorYCheckBox_toggled(bool checked) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_mirror(
				get_section_index(),
				get_layer_index(),
				ui->mirrorXCheckBox->isChecked(),
				checked
			)
		);
	}

	/**
	 * Changes the Layer's mix mode.
	 * @param index Index of new mix mode.
	 */
	void SectionControlWidget::on_mixModeComboBox_currentIndexChanged(int index) {
		// Only continue if the active Section is a Layer.
		if (get_layer_index() == 0) return ;

		if ((Colors::MixMode)index != active_section_->get_parent_section()->get_layer()->mix_mode) {
			maestro_control_widget_.run_cue(
				maestro_control_widget_.section_handler->set_layer(
					get_section_index(),
					get_layer_index(*active_section_->get_parent_section()),
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

	void SectionControlWidget::on_scaleXSpinBox_editingFinished() {
		set_scale();
	}

	void SectionControlWidget::on_scaleYSpinBox_editingFinished() {
		set_scale();
	}

	/// Recalculates scroll when horizontal scroll rate changes.
	void SectionControlWidget::on_scrollXSpinBox_editingFinished() {
		set_scroll();
	}

	/// Recalculates scroll when vertical scroll rate changes.
	void SectionControlWidget::on_scrollYSpinBox_editingFinished() {
		set_scroll();
	}

	void SectionControlWidget::on_removeLayerButton_clicked() {
		// Don't allow user to delete the base Section
		if (ui->layerListWidget->count() == 1) return;

		uint8_t target_layer = ui->layerListWidget->count() - 1;

		ui->layerListWidget->takeItem(target_layer);

		// Change the active Layer to the previous available one
		if (ui->layerListWidget->currentRow() == target_layer) {
			ui->layerListWidget->setCurrentRow(target_layer - 1);
		}

		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->remove_layer(
				get_section_index(),
				target_layer - 1
			)
		);
	}

	void SectionControlWidget::on_sectionListWidget_currentRowChanged(int currentRow) {
		set_layer_controls_enabled(false);

		set_active_section(maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(currentRow));
	}

	void SectionControlWidget::on_wrapCheckBox_stateChanged(int arg1) {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_wrap(
				get_section_index(),
				get_layer_index(),
				arg1
			)
		);
	}

	/**
	 * Updates UI controls based on the current active Section.
	 */
	void SectionControlWidget::refresh() {
		// Update the Layer list, but only if the refresh was triggered by a Section, not a Layer
		if (!get_layer_index()) {
			populate_layer_list();
		}

		// Set dimensions
		ui->gridSizeXSpinBox->blockSignals(true);
		ui->gridSizeYSpinBox->blockSignals(true);
		ui->gridSizeXSpinBox->setValue(active_section_->get_dimensions().x);
		ui->gridSizeYSpinBox->setValue(active_section_->get_dimensions().y);
		ui->gridSizeXSpinBox->blockSignals(false);
		ui->gridSizeYSpinBox->blockSignals(false);

		// Set offset
		ui->offsetXSpinBox->blockSignals(true);
		ui->offsetYSpinBox->blockSignals(true);
		ui->offsetXSpinBox->setValue(active_section_->get_offset().x);
		ui->offsetYSpinBox->setValue(active_section_->get_offset().y);
		ui->offsetXSpinBox->blockSignals(false);
		ui->offsetYSpinBox->blockSignals(false);

		// Set scroll
		ui->scrollXSpinBox->blockSignals(true);
		ui->scrollYSpinBox->blockSignals(true);
		int32_t interval_x = 0;
		int32_t interval_y = 0;
		if (active_section_->get_scroll() != nullptr) {
			Section::Scroll* scroll = active_section_->get_scroll();
			uint16_t refresh = maestro_control_widget_.get_maestro_controller()->get_maestro().get_timer().get_interval();
			// x axis
			if (scroll->timer_x != nullptr) {
				float x = refresh / (float)scroll->timer_x->get_interval();
				interval_x = (active_section_->get_dimensions().x * refresh) / x;
			}
			else {
				if (scroll->step_x > 0) {
					interval_x = (active_section_->get_dimensions().x * refresh) / (float)scroll->step_x;
				}
			}
			if (scroll->reverse_x) interval_x *= -1;

			// y axis
			if (scroll->timer_y != nullptr) {
				float y = refresh / (float)scroll->timer_y->get_interval();
				interval_y = (active_section_->get_dimensions().y * refresh) / y;
			}
			else {
				if (scroll->step_y > 0) {
					interval_y = (active_section_->get_dimensions().y * refresh) / (float)scroll->step_y;
				}
			}
			if (scroll->reverse_y) interval_y *= -1;
		}
		ui->scrollXSpinBox->setValue(interval_x);
		ui->scrollYSpinBox->setValue(interval_y);
		ui->scrollXSpinBox->blockSignals(false);
		ui->scrollYSpinBox->blockSignals(false);

		// Set mirror
		ui->mirrorXCheckBox->blockSignals(true);
		ui->mirrorYCheckBox->blockSignals(true);
		if (active_section_->get_mirror()) {
			ui->mirrorXCheckBox->setChecked(active_section_->get_mirror()->x);
			ui->mirrorYCheckBox->setChecked(active_section_->get_mirror()->y);
		}
		else {
			ui->mirrorXCheckBox->setChecked(false);
			ui->mirrorYCheckBox->setChecked(false);
		}
		ui->mirrorXCheckBox->blockSignals(false);
		ui->mirrorYCheckBox->blockSignals(false);

		// Set wrap
		ui->wrapCheckBox->blockSignals(true);
		ui->wrapCheckBox->setChecked(active_section_->get_wrap());
		ui->wrapCheckBox->blockSignals(false);

		// Set scale
		ui->scaleXSpinBox->blockSignals(true);
		ui->scaleYSpinBox->blockSignals(true);
		ui->scaleXSpinBox->setValue(active_section_->get_scale().x);
		ui->scaleYSpinBox->setValue(active_section_->get_scale().y);
		ui->scaleYSpinBox->blockSignals(false);
		ui->scaleXSpinBox->blockSignals(false);

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
	 * Rebuilds the Layer list using the current active Section.
	 */
	void SectionControlWidget::populate_layer_list() {
		ui->layerListWidget->blockSignals(true);
		ui->layerListWidget->clear();
		ui->layerListWidget->addItem("Base Layer");

		for (uint8_t layer = 0; layer < get_num_layers(*maestro_control_widget_.get_maestro_controller()->get_maestro().get_section(get_section_index())); layer++) {
			ui->layerListWidget->addItem(QString("Layer ") + QString::number(layer + 1));
		}

		ui->layerListWidget->blockSignals(false);

		// Enable/disable layer buttons
		ui->addLayerButton->setEnabled(ui->layerListWidget->count() <= 256);
		ui->removeLayerButton->setEnabled(ui->layerListWidget->count() > 1);
	}

	/**
	 * Changes the active Section.
	 * @param section New active Section.
	 */
	void SectionControlWidget::set_active_section(Section* section) {
		active_section_ = section;

		ui->sectionListWidget->blockSignals(true);
		ui->sectionListWidget->setCurrentRow(get_section_index(*active_section_));
		ui->sectionListWidget->blockSignals(false);

		maestro_control_widget_.refresh_section_settings();
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
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_offset(
				get_section_index(),
				get_layer_index(),
				ui->offsetXSpinBox->value(),
				ui->offsetYSpinBox->value()
			)
		);
	}

	void SectionControlWidget::set_scale() {
		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_scale(
				get_section_index(),
				get_layer_index(),
				ui->scaleXSpinBox->value(),
				ui->scaleYSpinBox->value()
			)
		);
	}

	/**
	 * Sets the Section's scrolling pattern.
	 */
	void SectionControlWidget::set_scroll() {
		int new_x = ui->scrollXSpinBox->value();
		int new_y = ui->scrollYSpinBox->value();

		maestro_control_widget_.run_cue(
			maestro_control_widget_.section_handler->set_scroll(
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
			ui->offsetXSpinBox->setValue(active_section_->get_offset().x);
			ui->offsetYSpinBox->blockSignals(false);
		}
		if (new_y == 0) {
			ui->offsetYSpinBox->blockSignals(true);
			ui->offsetYSpinBox->setValue(active_section_->get_offset().y);
			ui->offsetYSpinBox->blockSignals(false);
		}
	}

	/**
	 * Sets the size of the active Section and its Layers.
	 */
	void SectionControlWidget::set_section_size() {
		// Only resize if the dimensions are different
		Point new_dimensions(ui->gridSizeXSpinBox->value(), ui->gridSizeYSpinBox->value());
		if (new_dimensions != active_section_->get_dimensions()) {
			/*
			 * Check for a Canvas.
			 * The old Canvas gets deleted on resize.
			 * If one is set, copy its contents to a temporary buffer, then copy it back once the new Canvas is created.
			 */
			Canvas* canvas = active_section_->get_canvas();
			if (canvas != nullptr) {
				Point frame_bounds(active_section_->get_dimensions().x, active_section_->get_dimensions().y);

				uint8_t** frames = new uint8_t*[canvas->get_num_frames()];
				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					frames[frame] = new uint8_t[frame_bounds.size()];
				}
				CanvasUtility::copy_from_canvas(*canvas, frames, frame_bounds.x, frame_bounds.y);

				maestro_control_widget_.run_cue(
					maestro_control_widget_.section_handler->set_dimensions(
						get_section_index(),
						0,
						new_dimensions.x,
						new_dimensions.y
					)
				);

				CanvasUtility::copy_to_canvas(*canvas, frames, frame_bounds.x, frame_bounds.y, maestro_control_widget_);

				for (uint16_t frame = 0; frame < canvas->get_num_frames(); frame++) {
					delete[] frames[frame];
				}
				delete[] frames;
			}
			else {	// No Canvas set
				maestro_control_widget_.run_cue(
					maestro_control_widget_.section_handler->set_dimensions(
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
