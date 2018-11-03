#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>
#include <QKeyEvent>
#include <QSettings>
#include <QString>
#include "controller/maestrocontroller.h"
#include "core/section.h"
#include "drawingarea/maestrodrawingarea.h"
#include "maestrocontrolwidget.h"
#include "dialog/preferencesdialog.h"
#include "widget/palettecontrolwidget.h"
#include "ui_maestrocontrolwidget.h"
#include "utility/canvasutility.h"
#include "utility.h"

// TODO: Move Canvases to separate widgets
namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * @param parent The QWidget containing this controller.
	 */
	MaestroControlWidget::MaestroControlWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MaestroControlWidget) {
		ui->setupUi(this);

		// Capture button key presses
		qApp->installEventFilter(this);

		// Add control tabs
		animation_control_widget_ = QSharedPointer<AnimationControlWidget>(new AnimationControlWidget(this));
		ui->animationTab->findChild<QLayout*>("animationLayout")->addWidget(animation_control_widget_.data());

		device_control_widget_ = QSharedPointer<DeviceControlWidget>(new DeviceControlWidget(this));
		ui->deviceTab->findChild<QLayout*>("deviceLayout")->addWidget(device_control_widget_.data());

		section_control_widget_ = QSharedPointer<SectionControlWidget>(new SectionControlWidget(this));
		ui->sectionTab->findChild<QLayout*>("sectionLayout")->addWidget(section_control_widget_.data());

		show_control_widget_ = QSharedPointer<ShowControlWidget>(new ShowControlWidget(this));
		ui->showTab->findChild<QLayout*>("showLayout")->addWidget(show_control_widget_.data());
	}

	void MaestroControlWidget::edit_palettes(QString palette) {
		PaletteControlWidget palette_control(&palette_controller_, palette);
		palette_control.exec();
		initialize_palettes();
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool MaestroControlWidget::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			// Handle Canvas tab keys
			if (watched == ui->canvasScrollArea && section_control_widget_->get_active_section()->get_canvas() != nullptr) {
				if (key_event->key() == Qt::Key_Left) {
					on_canvasPlaybackBackToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Right) {
					on_canvasPlaybackNextToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Space) {
					ui->canvasPlaybackStartStopToolButton->setChecked(!ui->canvasPlaybackStartStopToolButton->isChecked());
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	/**
	 * Returns the index of the selected Canvas Palette color.
	 * @return Canvas Palette color index.
	 */
	uint8_t MaestroControlWidget::get_canvas_color_index() const {
		return canvas_color_index_;
	}

	/**
	 * Returns whether the Canvas paint tool button is currently active.
	 * @return True if the paintToolButton is active.
	 */
	bool MaestroControlWidget::get_canvas_painting_enabled() {
		return (ui->paintToolButton->isEnabled() && ui->paintToolButton->isChecked());
	}

	/**
	 * Returns the control widget's MaestroController.
	 * @return MaestroController.
	 */
	MaestroController* MaestroControlWidget::get_maestro_controller() {
		return maestro_controller_;
	}

	/**
	 * Build the MaestroControl UI.
	 */
	void MaestroControlWidget::initialize() {
		initialize_palettes();

		// Disable advanced controls until they're activated manually
		set_canvas_controls_enabled(0);

		// Initialize Canvas elements
		// Add drawing buttons to group
		canvas_shape_type_group_.addButton(ui->circleToolButton);
		canvas_shape_type_group_.addButton(ui->lineToolButton);
		canvas_shape_type_group_.addButton(ui->paintToolButton);
		canvas_shape_type_group_.addButton(ui->rectToolButton);
		canvas_shape_type_group_.addButton(ui->textToolButton);
		canvas_shape_type_group_.addButton(ui->triangleToolButton);

		// Set Canvas defaults
		ui->currentFrameSpinBox->setEnabled(false);

		// Add an event handler to the Canvas color picker cancel button
		connect(ui->canvasColorPickerCancelButton, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
	}

	/// Reinitializes Palettes from the Palette Dialog.
	void MaestroControlWidget::initialize_palettes() {
		QString canvas_palette = ui->canvasPaletteComboBox->currentText();

		// Repopulate color combo boxes
		ui->canvasPaletteComboBox->blockSignals(true);
		ui->canvasPaletteComboBox->clear();

		for (uint16_t i = 0; i < palette_controller_.get_palettes()->size(); i++) {
			ui->canvasPaletteComboBox->addItem(palette_controller_.get_palette(i)->name);
		}

		ui->canvasPaletteComboBox->setCurrentText(canvas_palette);
		ui->canvasPaletteComboBox->blockSignals(false);

		animation_control_widget_->refresh_palettes();
	}

	/**
	 * Loads a Cuefile into the Maestro.
	 * @param byte_array Byte array containing the Cuefile.
	 */
	void MaestroControlWidget::load_cuefile(QByteArray byte_array) {
		this->loading_cue_ = true;
		for (int i = 0; i < byte_array.size(); i++) {
			uint8_t byte = (uint8_t)byte_array.at(i);
			if (cue_controller_->read(byte)) {
				run_cue(cue_controller_->get_buffer(), true);
			}
		}
		this->loading_cue_ = false;
	}

	/// Opens the Palette Editor from the Canvas tab.
	void MaestroControlWidget::on_canvasEditPaletteButton_clicked() {
		QString palette_name = ui->canvasPaletteComboBox->currentText();

		PaletteControlWidget palette_control(&palette_controller_, palette_name);
		palette_control.exec();
		initialize_palettes();
		ui->canvasPaletteComboBox->setCurrentText(palette_name);
	}

	/**
	 * Activates the Canvas.
	 * @param checked If true, Canvas is enabled.
	 */
	void MaestroControlWidget::on_canvasEnableCheckBox_toggled(bool checked) {
		// Check to see if a Canvas already exists. If it does, warn the user that the current Canvas will be erased.
		if (!checked && section_control_widget_->get_active_section()->get_canvas() != nullptr) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				run_cue(
					section_handler->remove_canvas(
						section_control_widget_->get_section_index(),
						section_control_widget_->get_layer_index()
					)
				);
			}
			else {
				// Return to the previous state and exit.
				ui->canvasEnableCheckBox->blockSignals(true);
				ui->canvasEnableCheckBox->setChecked(true);
				ui->canvasEnableCheckBox->blockSignals(false);
				return;
			}
		}

		// Display/hide controls as necessary
		set_canvas_controls_enabled(checked);
		if (checked) {
			run_cue(
				section_handler->set_canvas(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index()
				)
			);

			// Default to the circle radio button so that the controls can be refreshed
			ui->circleToolButton->setChecked(true);
			on_circleToolButton_toggled(true);

			// Select a palette
			on_canvasPaletteComboBox_currentIndexChanged(0);
		}
		else {
			// Disable Canvas playback
			ui->canvasPlaybackStartStopToolButton->setChecked(false);
			on_canvasPlaybackStartStopToolButton_toggled(false);
		}
	}

	/**
	 * Changes the Canvas' palette.
	 * @param index New index.
	 */
	void MaestroControlWidget::on_canvasPaletteComboBox_currentIndexChanged(int index) {
		PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette(index);
		run_cue(
			canvas_handler->set_palette(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index(),
				&palette_wrapper->palette
			)
		);

		populate_palette_canvas_color_selection(palette_wrapper);
	}

	/**
	 * Moves the Canvas animation back a frame.
	 */
	void MaestroControlWidget::on_canvasPlaybackBackToolButton_clicked() {
		run_cue(
			canvas_handler->previous_frame(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index()
			)
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Advances the Canvas animation by a single frame.
	 */
	void MaestroControlWidget::on_canvasPlaybackNextToolButton_clicked() {
		run_cue(
			canvas_handler->next_frame(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index()
			)
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Toggles Canvas animation playback.
	 * @param checked If true, run the Canvas animation.
	 */
	void MaestroControlWidget::on_canvasPlaybackStartStopToolButton_toggled(bool checked) {
		if (section_control_widget_->get_active_section()->get_canvas() == nullptr) return;

		// Enables/disable the 'current frame' control
		ui->currentFrameSpinBox->setEnabled(checked);

		if (checked) {
			run_cue(
				canvas_handler->stop_frame_timer(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index()
				)
			);
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
			ui->currentFrameSpinBox->blockSignals(false);
		}
		else {
			run_cue(
				canvas_handler->start_frame_timer(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index()
				)
			);

			on_frameIntervalSpinBox_editingFinished();
		}
	}

	/**
	 * Selects a circle for drawing.
	 * @param checked If true, next shape will be a circle.
	 */
	void MaestroControlWidget::on_circleToolButton_toggled(bool checked) {
		set_circle_controls_enabled(checked);
	}

	/**
	 * Clears the current Canvas frame.
	 */
	void MaestroControlWidget::on_clearButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			run_cue(
				canvas_handler->clear(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index()
				)
			);
		}
	}

	/**
	 * Switches to the Canvas Frame index specified in currentFrameSpinBox.
	 */
	void MaestroControlWidget::on_currentFrameSpinBox_editingFinished() {
		int frame = ui->currentFrameSpinBox->value();

		// If the selected frame exceeds the number of frames, set to the number of frames.
		int num_frames = section_control_widget_->get_active_section()->get_canvas()->get_num_frames();
		if (frame >= num_frames) {
			frame = num_frames - 1;
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(frame);
			ui->currentFrameSpinBox->blockSignals(false);
		}

		run_cue(
			canvas_handler->set_current_frame_index(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index(),
				frame
			)
		);
	}

	/**
	 * Handles drawing onto the current Canvas frame.
	 */
	void MaestroControlWidget::on_drawButton_clicked() {
		QAbstractButton* checked_button = canvas_shape_type_group_.checkedButton();

		if (checked_button == ui->circleToolButton) {
			run_cue(
				canvas_handler->draw_circle(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->fillCheckBox->isChecked()
				)
			);
		}
		else if (checked_button == ui->lineToolButton) {
			run_cue(
				canvas_handler->draw_line(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->targetYSpinBox->value()
				)
			);
		}
		else if (checked_button == ui->paintToolButton) {
			run_cue(
				canvas_handler->draw_point(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value()
				)
			);
		}
		else if (checked_button == ui->rectToolButton) {
			run_cue(
				canvas_handler->draw_rect(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->targetYSpinBox->value(),
					ui->fillCheckBox->isChecked()
				)
			);
		}
		else if (checked_button == ui->textToolButton) {
			run_cue(
				canvas_handler->draw_text(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					(Font::Type)ui->fontComboBox->currentIndex(),
					ui->textLineEdit->text().toLatin1().data(),
					ui->textLineEdit->text().size()
				)
			);
		}
		else if (checked_button == ui->triangleToolButton) {
			run_cue(
				canvas_handler->draw_triangle(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index(),
					canvas_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->targetYSpinBox->value(),
					ui->target2XSpinBox->value(),
					ui->target2YSpinBox->value(),
					ui->fillCheckBox->isChecked()
				)
			);
		}
	}

	/**
	 * Sets the number of Canvas frames.
	 */
	void MaestroControlWidget::on_frameCountSpinBox_editingFinished() {
		int new_max = ui->frameCountSpinBox->value();
		if (new_max != section_control_widget_->get_active_section()->get_canvas()->get_num_frames()) {

			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				if (new_max < ui->currentFrameSpinBox->value()) {
					ui->currentFrameSpinBox->setValue(new_max);
				}
				ui->currentFrameSpinBox->setMaximum(new_max);
				run_cue(
					canvas_handler->set_num_frames(
						section_control_widget_->get_section_index(),
						section_control_widget_->get_layer_index(),
						new_max
					)
				);

				// Set the new maximum for the current_frame spinbox
				ui->currentFrameSpinBox->setMaximum(new_max);
			}
			else {
				// Reset frame count
				ui->frameCountSpinBox->blockSignals(true);
				ui->frameCountSpinBox->setValue(section_control_widget_->get_active_section()->get_canvas()->get_num_frames());
				ui->frameCountSpinBox->blockSignals(false);
			}
		}
	}

	/**
	 * Sets the interval between Canvas frames.
	 * @param value New interval.
	 */
	void MaestroControlWidget::on_frameIntervalSlider_valueChanged(int value) {
		ui->frameIntervalSpinBox->blockSignals(true);
		ui->frameIntervalSpinBox->setValue(value);
		ui->frameIntervalSpinBox->blockSignals(false);

		set_canvas_frame_interval();
	}

	/**
	 * Sets the interval between Canvas frames.
	 */
	void MaestroControlWidget::on_frameIntervalSpinBox_editingFinished() {
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSlider->setValue(ui->frameIntervalSpinBox->value());
		ui->frameIntervalSlider->blockSignals(false);

		set_canvas_frame_interval();
	}

	/**
	 * Selects a rectangle for the next shape.
	 * @param checked If true, the next shape will be a rectangle.
	 */
	void MaestroControlWidget::on_rectToolButton_toggled(bool checked) {
		set_rect_controls_enabled(checked);
	}

	/**
	 * Selects a line for the next shape.
	 * @param checked If true, the next shape will be a line.
	 */
	void MaestroControlWidget::on_lineToolButton_toggled(bool checked) {
		set_line_controls_enabled(checked);
	}

	/**
	 * Loads an image into the Canvas.
	 * This overwrites all Canvas frames.
	 */
	void MaestroControlWidget::on_loadImageButton_clicked() {
		QString filename = QFileDialog::getOpenFileName(this,
			QString("Open Image"),
			QString(),
			QString("Images (*.bmp *.gif *.jpg *.png)"));

		if (!filename.isEmpty()) {

			// Stop playback
			on_canvasPlaybackStartStopToolButton_toggled(true);

			// Clear the current Canvas
			run_cue(
				canvas_handler->clear(
					section_control_widget_->get_section_index(),
					section_control_widget_->get_layer_index()
				)
			);

			CanvasUtility::load_image(filename, section_control_widget_->get_active_section()->get_canvas(), this);

			Canvas* canvas = section_control_widget_->get_active_section()->get_canvas();

			// Update UI based on new canvas
			ui->frameCountSpinBox->blockSignals(true);
			ui->frameCountSpinBox->setValue(canvas->get_num_frames());
			ui->frameCountSpinBox->blockSignals(false);

			if (canvas->get_frame_timer() != nullptr) {
				ui->frameIntervalSpinBox->blockSignals(true);
				ui->frameIntervalSpinBox->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameIntervalSpinBox->blockSignals(false);
			}

			// Add Palette to list
			QString name = "Section " + QString::number(section_control_widget_->get_section_index()) +
						   " Layer " + QString::number(section_control_widget_->get_layer_index()) +
						   " Canvas";
			palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
			ui->canvasPaletteComboBox->blockSignals(true);
			ui->canvasPaletteComboBox->addItem(name);
			ui->canvasPaletteComboBox->blockSignals(false);
			ui->canvasPaletteComboBox->setCurrentText(name);

			// Start playback
			on_canvasPlaybackStartStopToolButton_toggled(false);
		}
	}

	void MaestroControlWidget::on_paintToolButton_toggled(bool checked) {
		set_paint_controls_enabled(checked);
	}

	/**
	 * Sets the active drawing color.
	 * This gets passed to the CanvasCueHandler on each draw.
	 */
	void MaestroControlWidget::on_canvas_color_clicked() {
		QPushButton* sender = (QPushButton*)QObject::sender();

		if (sender != ui->canvasColorPickerCancelButton) {
			canvas_color_index_ = sender->objectName().toInt();

			// Change the color of the Color button to reflect the selection
			PaletteController::PaletteWrapper* palette_wrapper = palette_controller_.get_palette(ui->canvasPaletteComboBox->currentIndex());
			Colors::RGB color = palette_wrapper->palette.get_colors()[canvas_color_index_];
			ui->selectColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));
		}
		else {
			canvas_color_index_ = 255;
			ui->selectColorButton->setStyleSheet(QString("background-color: transparent;"));
		}

		run_cue(
			canvas_handler->set_drawing_color(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index(),
				canvas_color_index_
			)
		);
	}

	/**
	 * Selects text for the next Canvas shape.
	 * @param checked If true, the next shape will be text.
	 */
	void MaestroControlWidget::on_textToolButton_toggled(bool checked) {
		set_text_controls_enabled(checked);
	}

	/**
	 * Sets the next shape to triangle.
	 * @param checked If true, the next Canvas shape drawn will be a triangle.
	 */
	void MaestroControlWidget::on_triangleToolButton_toggled(bool checked) {
		set_triangle_controls_enabled(checked);
	}

	/**
	 * Rebuilds the Palette Canvas color selection scroll area.
	 */
	void MaestroControlWidget::populate_palette_canvas_color_selection(PaletteController::PaletteWrapper* palette_wrapper) {
		// Add color buttons to canvasColorPickerLayout. This functions identically to palette switching in the Palette Editor.
		// Delete existing color buttons
		QList<QPushButton*> buttons = ui->canvasColorPickerScrollArea->findChildren<QPushButton*>(QString(), Qt::FindChildOption::FindChildrenRecursively);
		for (QPushButton* button : buttons) {
			// Remove all but the Cancel button
			if (button != ui->canvasColorPickerCancelButton) {
				disconnect(button, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
				delete button;
			}
		}

		// Create new buttons and connect the color picker to the pushbutton event
		QLayout* layout = ui->canvasColorPickerScrollArea->findChild<QLayout*>("canvasColorPickerLayout");
		for (uint8_t color_index = 0; color_index < palette_wrapper->palette.get_num_colors(); color_index++) {
			Colors::RGB color = palette_wrapper->palette.get_colors()[color_index];
			QPushButton* button = new QPushButton();
			button->setVisible(true);
			button->setObjectName(QString::number(color_index));
			button->setToolTip(QString::number(color_index + 1));
			button->setMaximumWidth(40);
			button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));

			layout->addWidget(button);
			connect(button, &QPushButton::clicked, this, &MaestroControlWidget::on_canvas_color_clicked);
		}
	}

	/**
	 * Updates the UI to reflect a Section change.
	 */
	void MaestroControlWidget::refresh() {
		animation_control_widget_->refresh();
		section_control_widget_->refresh();
		// TODO: Add CanvasControlWidget here once implemented
	}

	/**
	 * Updates the UI to reflect the Maestro's current settings.
	 */
	void MaestroControlWidget::refresh_maestro_settings() {
		animation_control_widget_->refresh();
		section_control_widget_->refresh();
		show_control_widget_->refresh();

		// Recalculate the Maestro Cuefile
		if (device_control_widget_ != nullptr) {
			device_control_widget_->update_cuefile_size();
		}
	}

	/**
	 * Forwards the specified Cue to the drawing area and/or serial device.
	 * @param cue Cue to perform.
	 * @param serial_only If true, don't run the Cue on the local Maestro.
	 */
	void MaestroControlWidget::run_cue(uint8_t *cue, bool remote_only) {
		// TODO: Indicate that the Cue is unsaved

		show_control_widget_->add_event_to_history(cue);

		/*
		 * Only run the Cue if:
		 *	1) The Maestro isn't locked
		 *	2) The Maestro is locked, but the Cue is a Show Cue
		 */
		if (!show_control_widget_->get_maestro_locked() ||
			(show_control_widget_->get_maestro_locked() && cue[(uint8_t)CueController::Byte::PayloadByte] == (uint8_t)CueController::Handler::ShowCueHandler)) {
			if (!remote_only) {
				cue_controller_->run(cue);
			}

			// Send to serial device controller.
			if (device_control_widget_ != nullptr) {
				device_control_widget_->run_cue(cue, cue_controller_->get_cue_size(cue));
			}
		}
	}

	/**
	 * Changes the active Section and sets all GUI controls to the Section's settings.
	 * @param section New active Section.
	 * // FIXME: Remove after verifying SectionControlWidget::set_active_section()
	 */
	void MaestroControlWidget::set_active_section(Section* section) {
		if (section == nullptr) return;

		section_control_widget_->set_active_section(section);
		section_control_widget_->refresh();

		// Update Animation widget
		animation_control_widget_->refresh();

		// Update Canvas widgets
		ui->canvasEnableCheckBox->blockSignals(true);
		ui->frameCountSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->blockSignals(true);
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSpinBox->blockSignals(true);

		Canvas* canvas = section->get_canvas();
		if (canvas != nullptr) {
			ui->canvasEnableCheckBox->setChecked(true);
			ui->frameCountSpinBox->setValue(canvas->get_num_frames());
			ui->currentFrameSpinBox->setValue(canvas->get_current_frame_index());
			if (canvas->get_frame_timer() != nullptr) {
				ui->frameIntervalSlider->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameIntervalSpinBox->setValue(canvas->get_frame_timer()->get_interval());
			}
			set_canvas_controls_enabled(true);

			// Find the corresponding palette in the Palette Controller.
			if (canvas->get_palette() != nullptr) {
				int palette_index = palette_controller_.find(canvas->get_palette()->get_colors());
				if (palette_index >= 0) {
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->setCurrentIndex(palette_index);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
				else {
					QString name = "Section " + QString::number(section_control_widget_->get_section_index()) +
								   " Layer " + QString::number(section_control_widget_->get_layer_index()) +
								   " Canvas";
					palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
					ui->canvasPaletteComboBox->blockSignals(true);
					ui->canvasPaletteComboBox->addItem(name);
					ui->canvasPaletteComboBox->setCurrentText(name);
					ui->canvasPaletteComboBox->blockSignals(false);
				}
			}

			populate_palette_canvas_color_selection(palette_controller_.get_palette(ui->canvasPaletteComboBox->currentIndex()));
		}
		else {
			ui->canvasEnableCheckBox->setChecked(false);
			ui->frameCountSpinBox->setValue(1);
			ui->currentFrameSpinBox->setValue(0);
			ui->frameIntervalSlider->setValue(100);
			ui->frameIntervalSpinBox->setValue(100);
			set_canvas_controls_enabled(0);
		}

		ui->frameCountSpinBox->blockSignals(false);
		ui->currentFrameSpinBox->blockSignals(false);
		ui->frameIntervalSlider->blockSignals(false);
		ui->frameIntervalSpinBox->blockSignals(false);
		ui->canvasEnableCheckBox->blockSignals(false);
	}

	/**
	 * Enables Canvas controls.
	 * @param enabled If true, enable the controls.
	 */
	void MaestroControlWidget::set_canvas_controls_enabled(bool enabled) {
		ui->drawingToolsGroupBox->setEnabled(enabled);
		ui->animationToolsGroupBox->setEnabled(enabled);
		ui->loadImageButton->setEnabled(enabled);

		// Canvas-specific controls
		ui->canvasPaletteComboBox->setEnabled(enabled);
		ui->canvasPaletteLabel->setEnabled(enabled);
		ui->canvasEditPaletteButton->setEnabled(enabled);
		ui->canvasColorPickerScrollArea->setEnabled(enabled);
	}

	/**
	 * Sets the Canvas' origin to the specified coordinates.
	 */
	void MaestroControlWidget::set_canvas_origin(Point* coordinates) {
		ui->originXSpinBox->blockSignals(true);
		ui->originYSpinBox->blockSignals(true);
		ui->originXSpinBox->setValue(coordinates->x);
		ui->originYSpinBox->setValue(coordinates->y);
		ui->originXSpinBox->blockSignals(false);
		ui->originYSpinBox->blockSignals(false);
	}

	/**
	 * Enables Canvas circle controls.
	 * @param enabled If true, circle controls are enabled.
	 */
	void MaestroControlWidget::set_circle_controls_enabled(bool enabled) {
		if (enabled) {
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);

			ui->targetLabel->setText("Radius");
			ui->targetLabel->setEnabled(true);
			ui->targetXSpinBox->setEnabled(true);
		}
		else {
			ui->targetLabel->setText("Target");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
		ui->fillCheckBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas line controls.
	 * @param enabled If true, line controls are enabled.
	 */
	void MaestroControlWidget::set_line_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->fillCheckBox->setEnabled(false);
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas paint controls.
	 * @param enabled If true, paint controls are enabled.
	 */
	void MaestroControlWidget::set_paint_controls_enabled(bool enabled) {
		set_circle_controls_enabled(false);
		set_line_controls_enabled(false);
		set_rect_controls_enabled(false);
		set_text_controls_enabled(false);
		set_triangle_controls_enabled(false);

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
		ui->originLabel->setText("Cursor");
	}

	/**
	 * Enables Canvas rectangle controls.
	 * @param enabled If true, rectangle controls are enabled.
	 */
	void MaestroControlWidget::set_rect_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->targetLabel->setText("Size");
		}
		else {
			ui->targetLabel->setText("Target");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);

		ui->fillCheckBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas text controls.
	 * @param enabled If true, text controls are enabled.
	 */
	void MaestroControlWidget::set_text_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->fillCheckBox->setEnabled(false);
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->fontLabel->setEnabled(enabled);
		ui->fontComboBox->setEnabled(enabled);

		ui->textLabel->setEnabled(enabled);
		ui->textLineEdit->setEnabled(enabled);
	}

	/**
	 * Enables Canvas triangle controls.
	 * @param enabled If true, triangle controls are enabled.
	 */
	void MaestroControlWidget::set_triangle_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);

			ui->originLabel->setText("Point A");
			ui->targetLabel->setText("Point B");
			ui->target2Label->setText("Point C");
		}
		else {
			ui->originLabel->setText("Cursor");
			ui->targetLabel->setText("Target");
			ui->target2Label->setText("Target 2");
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);

		ui->targetLabel->setEnabled(enabled);
		ui->targetXSpinBox->setEnabled(enabled);
		ui->targetYSpinBox->setEnabled(enabled);

		ui->target2Label->setEnabled(enabled);
		ui->target2XSpinBox->setEnabled(enabled);
		ui->target2YSpinBox->setEnabled(enabled);

		ui->fillCheckBox->setEnabled(enabled);
	}
	// End Canvas-specific methods

	/**
	 * Sets the widget's target MaestroController.
	 * @param maestro_controller New MaestroController.
	 */
	void MaestroControlWidget::set_maestro_controller(MaestroController *maestro_controller) {
		this->maestro_controller_ = maestro_controller;

		// Get Maestro's Cue Handlers for convenience
		cue_controller_ = maestro_controller_->get_maestro()->get_cue_controller();
		animation_handler = static_cast<AnimationCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::AnimationCueHandler)
		);
		canvas_handler = static_cast<CanvasCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::CanvasCueHandler)
		);
		maestro_handler = static_cast<MaestroCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::MaestroCueHandler)
		);
		section_handler = static_cast<SectionCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::SectionCueHandler)
		);
		show_handler = static_cast<ShowCueHandler*>(
			cue_controller_->get_handler(CueController::Handler::ShowCueHandler)
		);

		// Initialize UI components and controllers
		section_control_widget_->set_active_section(maestro_controller_->get_maestro()->get_section(0));
		section_control_widget_->initialize();
		animation_control_widget_->initialize();
		refresh_maestro_settings();
		initialize();
	}

	/// Sets the interval between Canvas frames.
	void MaestroControlWidget::set_canvas_frame_interval() {
		run_cue(
			canvas_handler->set_frame_timer(
				section_control_widget_->get_section_index(),
				section_control_widget_->get_layer_index(),
				ui->frameIntervalSpinBox->value()
			)
		);
	}

	MaestroControlWidget::~MaestroControlWidget() {
		delete ui;
	}
}
