#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include "canvascontrolwidget.h"
#include "ui_canvascontrolwidget.h"
#include "utility/canvasutility.h"
#include "widget/palettecontrolwidget.h"

// TODO: Live video streaming/conversion
namespace PixelMaestroStudio {
	CanvasControlWidget::CanvasControlWidget(QWidget *parent) :	QWidget(parent), ui(new Ui::CanvasControlWidget) {
		this->maestro_control_widget_ = static_cast<MaestroControlWidget*>(parent);
		ui->setupUi(this);
		// Capture button key presses
		qApp->installEventFilter(this);

		initialize();
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool CanvasControlWidget::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			if (watched == ui->canvasScrollArea && maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas() != nullptr) {
				if (key_event->key() == Qt::Key_Left) {
					on_playbackPreviousToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Right) {
					on_playbackNextToolButton_clicked();
					return true;
				}
				else if (key_event->key() == Qt::Key_Space) {
					ui->playbackStartStopToolButton->setChecked(!ui->playbackStartStopToolButton->isChecked());
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	void CanvasControlWidget::initialize() {
		// Disable advanced controls until they're activated manually
		set_controls_enabled(0);

		// Initialize Canvas elements
		// Add drawing buttons to group
		canvas_shape_type_group_.addButton(ui->circleToolButton);
		canvas_shape_type_group_.addButton(ui->lineToolButton);
		canvas_shape_type_group_.addButton(ui->brushToolButton);
		canvas_shape_type_group_.addButton(ui->rectToolButton);
		canvas_shape_type_group_.addButton(ui->replaceToolButton);
		canvas_shape_type_group_.addButton(ui->textToolButton);
		canvas_shape_type_group_.addButton(ui->triangleToolButton);

		// Set Canvas defaults
		ui->currentFrameSpinBox->setEnabled(false);

		// Add an event handler to the Canvas color picker cancel button
		connect(ui->colorPickerCancelButton, SIGNAL(clicked(bool)), this, SLOT(on_canvas_color_clicked()));
	}

	/**
	 * Selects a circle for drawing.
	 * @param checked If true, next shape will be a circle.
	 */
	void CanvasControlWidget::on_circleToolButton_toggled(bool checked) {
		set_circle_controls_enabled(checked);
	}

	/**
	 * Returns the index of the selected Canvas Palette color.
	 * @return Canvas Palette color index.
	 */
	uint8_t CanvasControlWidget::get_selected_color_index() const {
		return selected_color_index_;
	}

	/**
	 * Returns whether the Canvas paint tool button is currently active.
	 * @return True if the brushToolButton is active.
	 */
	bool CanvasControlWidget::get_painting_enabled() const {
		return (ui->brushToolButton->isEnabled() && ui->brushToolButton->isChecked());
	}

	/**
	 * Selects the freehand brush for drawing.
	 * @param checked If true, user can draw freehand.
	 */
	void CanvasControlWidget::on_brushToolButton_toggled(bool checked) {
		set_brush_controls_enabled(checked);
	}

	/**
	 * Sets the active drawing color.
	 * This gets passed to the CanvasCueHandler on each draw.
	 */
	void CanvasControlWidget::on_canvas_color_clicked() {
		QPushButton* sender = (QPushButton*)QObject::sender();

		if (sender != ui->colorPickerCancelButton) {
			selected_color_index_ = sender->objectName().toInt();

			// Change the color of the Color button to reflect the selection
			PaletteController::PaletteWrapper* palette_wrapper = maestro_control_widget_->palette_controller_.get_palette(ui->paletteComboBox->currentIndex());
			Colors::RGB color = palette_wrapper->palette.get_colors()[selected_color_index_];
			ui->selectColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));
		}
		else {
			selected_color_index_ = 255;
			ui->selectColorButton->setStyleSheet(QString("background-color: transparent;"));
		}

		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->set_drawing_color(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index(),
				selected_color_index_
			)
		);
	}

	/**
	 * Clears the current Canvas frame.
	 */
	void CanvasControlWidget::on_clearButton_clicked() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->clear(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index()
				)
			);
		}

		// Reset the current frame and frame count for animated images
		ui->frameCountSpinBox->setValue(1);
		ui->currentFrameSpinBox->setValue(0);
		on_currentFrameSpinBox_editingFinished();
	}

	/**
	 * Switches to the specified frame index.
	 */
	void CanvasControlWidget::on_currentFrameSpinBox_editingFinished() {
		int frame = ui->currentFrameSpinBox->value();

		// If the selected frame exceeds the number of frames, set to the number of frames.
		int num_frames = maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_num_frames();
		if (frame >= num_frames) {
			frame = num_frames - 1;
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(frame);
			ui->currentFrameSpinBox->blockSignals(false);
		}

		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->set_current_frame_index(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index(),
				frame
			)
		);
	}

	/**
	 * Handles drawing onto the current Canvas frame.
	 */
	void CanvasControlWidget::on_drawButton_clicked() {
		QAbstractButton* checked_button = canvas_shape_type_group_.checkedButton();

		if (checked_button == ui->circleToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_circle(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->fillCheckBox->isChecked()
				)
			);
		}
		else if (checked_button == ui->lineToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_line(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->targetYSpinBox->value()
				)
			);
		}
		else if (checked_button == ui->brushToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_point(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value()
				)
			);
		}
		else if (checked_button == ui->rectToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_rect(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					ui->targetXSpinBox->value(),
					ui->targetYSpinBox->value(),
					ui->fillCheckBox->isChecked()
				)
			);
		}
		else if (checked_button == ui->replaceToolButton) {
			// Replace all instances of the selected color index in the current frame with the new color index.
			Section* active_section = maestro_control_widget_->section_control_widget_->get_active_section();
			Point* dimensions = active_section->get_dimensions();

			uint8_t* frame = active_section->get_canvas()->get_frame(active_section->get_canvas()->get_current_frame_index());
			uint32_t target_point = dimensions->get_inline_index(ui->originXSpinBox->value(), ui->originYSpinBox->value());
			uint8_t target_index = frame[target_point];

			for (uint16_t row = 0; row < dimensions->y; row++) {
				for (uint16_t column = 0; column < dimensions->x; column++) {
					uint32_t frame_index = dimensions->get_inline_index(column, row);

					if (frame[frame_index] == target_index) {
						frame[frame_index] = selected_color_index_;
					}
				}
			}

			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_frame(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					dimensions->x,
					dimensions->y,
					frame
				)
			);
		}
		else if (checked_button == ui->textToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_text(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
					ui->originXSpinBox->value(),
					ui->originYSpinBox->value(),
					(Font::Type)ui->fontComboBox->currentIndex(),
					ui->textLineEdit->text().toLatin1().data(),
					ui->textLineEdit->text().size()
				)
			);
		}
		else if (checked_button == ui->triangleToolButton) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->draw_triangle(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index(),
					selected_color_index_,
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

	/// Opens the Palette Editor from the Canvas tab.
	void CanvasControlWidget::on_editPaletteButton_clicked() {
		maestro_control_widget_->edit_palettes(ui->paletteComboBox->currentText());
	}

	/**
	 * Enables the Canvas.
	 * @param checked If true, Canvas is enabled.
	 */
	void CanvasControlWidget::on_enableCheckBox_toggled(bool checked) {
		// Check to see if a Canvas already exists. If it does, warn the user that the current Canvas will be erased.
		if (!checked && maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas() != nullptr) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				maestro_control_widget_->run_cue(
					maestro_control_widget_->section_handler->remove_canvas(
						maestro_control_widget_->section_control_widget_->get_section_index(),
						maestro_control_widget_->section_control_widget_->get_layer_index()
					)
				);
			}
			else {
				// Return to the previous state and exit.
				ui->enableCheckBox->blockSignals(true);
				ui->enableCheckBox->setChecked(true);
				ui->enableCheckBox->blockSignals(false);
				return;
			}
		}

		// Display/hide controls as necessary, and if the Canvas is animated, pause by default
		set_controls_enabled(checked);
		on_playbackStartStopToolButton_toggled(checked);
		if (checked) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->section_handler->set_canvas(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index()
				)
			);

			// Default to the circle radio button so that the controls can be refreshed
			ui->circleToolButton->setChecked(true);
			on_circleToolButton_toggled(true);

			// Select a palette
			on_paletteComboBox_currentIndexChanged(0);
		}
	}

	/**
	 * Sets the number of Canvas frames.
	 */
	void CanvasControlWidget::on_frameCountSpinBox_editingFinished() {
		int new_max = ui->frameCountSpinBox->value();
		if (new_max != maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_num_frames()) {

			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Canvas", "This will clear the Canvas. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				if (new_max < ui->currentFrameSpinBox->value()) {
					ui->currentFrameSpinBox->setValue(new_max);
				}
				ui->currentFrameSpinBox->setMaximum(new_max);
				maestro_control_widget_->run_cue(
					maestro_control_widget_->canvas_handler->set_num_frames(
						maestro_control_widget_->section_control_widget_->get_section_index(),
						maestro_control_widget_->section_control_widget_->get_layer_index(),
						new_max
					)
				);

				// Set the new maximum for the current_frame spinbox
				ui->currentFrameSpinBox->setMaximum(new_max);
			}
			else {
				// Reset frame count
				ui->frameCountSpinBox->blockSignals(true);
				ui->frameCountSpinBox->setValue(maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_num_frames());
				ui->frameCountSpinBox->blockSignals(false);
			}
		}
	}

	/**
	 * Sets the interval between Canvas frames.
	 * @param value New interval.
	 */
	void CanvasControlWidget::on_frameIntervalSlider_valueChanged(int value) {
		ui->frameIntervalSpinBox->blockSignals(true);
		ui->frameIntervalSpinBox->setValue(value);
		ui->frameIntervalSpinBox->blockSignals(false);

		set_frame_interval();
	}

	/**
	 * Sets the interval (in ms) between Canvas frames.
	 */
	void CanvasControlWidget::on_frameIntervalSpinBox_editingFinished() {
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSlider->setValue(ui->frameIntervalSpinBox->value());
		ui->frameIntervalSlider->blockSignals(false);

		set_frame_interval();
	}

	/**
	 * Selects a line for the next shape.
	 * @param checked If true, the next shape will be a line.
	 */
	void CanvasControlWidget::on_lineToolButton_toggled(bool checked) {
		set_line_controls_enabled(checked);
	}

	/**
	 * Loads an image into the Canvas.
	 * This overwrites all Canvas frames.
	 */
	void CanvasControlWidget::on_loadImageButton_clicked() {
		QString filename = QFileDialog::getOpenFileName(this,
			QString("Open Image"),
			QString(),
			QString("Images (*.bmp *.gif *.jpg *.png)"));

		if (!filename.isEmpty()) {

			// Stop playback
			ui->playbackStartStopToolButton->toggled(true);

			// Clear the current Canvas
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->clear(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index()
				)
			);

			CanvasUtility::load_image(filename, maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas(), maestro_control_widget_);

			Canvas* canvas = maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas();

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
			QString name = "Section " + QString::number(maestro_control_widget_->section_control_widget_->get_section_index()) +
						   " Layer " + QString::number(maestro_control_widget_->section_control_widget_->get_layer_index()) +
						   " Canvas";
			maestro_control_widget_->palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
			ui->paletteComboBox->blockSignals(true);
			ui->paletteComboBox->addItem(name);
			ui->paletteComboBox->blockSignals(false);
			ui->paletteComboBox->setCurrentText(name);

			// Start playback
			ui->playbackStartStopToolButton->toggled(false);
		}
	}

	/**
	 * Advances the Canvas by a single frame.
	 */
	void CanvasControlWidget::on_playbackNextToolButton_clicked() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->next_frame(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index()
			)
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Moves the Canvas animation back a frame.
	 */
	void CanvasControlWidget::on_playbackPreviousToolButton_clicked() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->previous_frame(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index()
			)
		);

		ui->currentFrameSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->setValue(maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
		ui->currentFrameSpinBox->blockSignals(false);
	}

	/**
	 * Toggles Canvas animation playback.
	 * @param checked If true, pause the Canvas animation.
	 */
	void CanvasControlWidget::on_playbackStartStopToolButton_toggled(bool checked) {
		if (maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas() == nullptr) return;

		// Enable the current frame box and frame step buttons while the animation is paused
		ui->currentFrameSpinBox->setEnabled(checked);
		ui->playbackNextToolButton->setEnabled(checked);
		ui->playbackPreviousToolButton->setEnabled(checked);

		if (checked) {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->stop_frame_timer(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index()
				)
			);
			ui->currentFrameSpinBox->blockSignals(true);
			ui->currentFrameSpinBox->setValue(maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas()->get_current_frame_index());
			ui->currentFrameSpinBox->blockSignals(false);
		}
		else {
			maestro_control_widget_->run_cue(
				maestro_control_widget_->canvas_handler->start_frame_timer(
					maestro_control_widget_->section_control_widget_->get_section_index(),
					maestro_control_widget_->section_control_widget_->get_layer_index()
				)
			);

			on_frameIntervalSpinBox_editingFinished();
		}
	}

	/**
	 * Changes the Canvas' palette.
	 * @param index New index.
	 */
	void CanvasControlWidget::on_paletteComboBox_currentIndexChanged(int index) {
		PaletteController::PaletteWrapper* palette_wrapper = maestro_control_widget_->palette_controller_.get_palette(index);
		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->set_palette(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index(),
				&palette_wrapper->palette
			)
		);

		populate_palette_canvas_color_selection(palette_wrapper);
	}

	/**
	 * Selects a rectangle for the next shape.
	 * @param checked If true, the next shape will be a rectangle.
	 */
	void CanvasControlWidget::on_rectToolButton_toggled(bool checked) {
		set_rect_controls_enabled(checked);
	}

	void CanvasControlWidget::on_replaceToolButton_toggled(bool checked) {
		set_replace_controls_enabled(checked);
	}

	/**
	 * Selects text for the next Canvas shape.
	 * @param checked If true, the next shape will be text.
	 */
	void CanvasControlWidget::on_textToolButton_toggled(bool checked) {
		set_text_controls_enabled(checked);
	}

	/**
	 * Sets the next shape to triangle.
	 * @param checked If true, the next Canvas shape drawn will be a triangle.
	 */
	void CanvasControlWidget::on_triangleToolButton_toggled(bool checked) {
		set_triangle_controls_enabled(checked);
	}

	/**
	 * Rebuilds the Palette Canvas color selection scroll area.
	 */
	void CanvasControlWidget::populate_palette_canvas_color_selection(PaletteController::PaletteWrapper* palette_wrapper) {
		/*
		 * Add color buttons to canvasColorPickerLayout. This functions identically to palette switching in the Palette Editor.
		 * Start by deleting existing color buttons.
		 */
		QList<QPushButton*> buttons = ui->colorPickerScrollArea->findChildren<QPushButton*>(QString(), Qt::FindChildOption::FindChildrenRecursively);
		for (QPushButton* button : buttons) {
			// Remove all but the Cancel button
			if (button != ui->colorPickerCancelButton) {
				disconnect(button, SIGNAL(clicked(bool)), this, SLOT(on_canvas_color_clicked()));
				delete button;
			}
		}

		// Create new buttons and connect the color picker to the pushbutton event
		QLayout* layout = ui->colorPickerScrollArea->findChild<QLayout*>("colorPickerLayout");
		for (uint8_t color_index = 0; color_index < palette_wrapper->palette.get_num_colors(); color_index++) {
			Colors::RGB color = palette_wrapper->palette.get_colors()[color_index];
			QPushButton* button = new QPushButton();
			button->setVisible(true);
			button->setObjectName(QString::number(color_index));
			button->setToolTip(QString::number(color_index + 1));
			button->setMaximumWidth(40);
			button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.r).arg(color.g).arg(color.b));

			layout->addWidget(button);
			connect(button, SIGNAL(clicked(bool)), this, SLOT(on_canvas_color_clicked()));
		}
	}

	/**
	 * Updates the UI.
	 */
	void CanvasControlWidget::refresh() {
		ui->enableCheckBox->blockSignals(true);
		ui->frameCountSpinBox->blockSignals(true);
		ui->currentFrameSpinBox->blockSignals(true);
		ui->frameIntervalSlider->blockSignals(true);
		ui->frameIntervalSpinBox->blockSignals(true);

		Canvas* canvas = maestro_control_widget_->section_control_widget_->get_active_section()->get_canvas();
		if (canvas != nullptr) {
			ui->enableCheckBox->setChecked(true);
			ui->frameCountSpinBox->setValue(canvas->get_num_frames());
			ui->currentFrameSpinBox->setValue(canvas->get_current_frame_index());
			if (canvas->get_frame_timer() != nullptr) {
				ui->frameIntervalSlider->setValue(canvas->get_frame_timer()->get_interval());
				ui->frameIntervalSpinBox->setValue(canvas->get_frame_timer()->get_interval());
			}
			set_controls_enabled(true);

			// Find the corresponding palette in the Palette Controller.
			if (canvas->get_palette() != nullptr) {
				int palette_index = maestro_control_widget_->palette_controller_.find(canvas->get_palette()->get_colors());
				if (palette_index >= 0) {
					ui->paletteComboBox->blockSignals(true);
					ui->paletteComboBox->setCurrentIndex(palette_index);
					ui->paletteComboBox->blockSignals(false);
				}
				else {
					QString name = "Section " + QString::number(maestro_control_widget_->section_control_widget_->get_section_index()) +
								   " Layer " + QString::number(maestro_control_widget_->section_control_widget_->get_layer_index()) +
								   " Canvas";
					maestro_control_widget_->palette_controller_.add_palette(name, canvas->get_palette()->get_colors(), canvas->get_palette()->get_num_colors(), PaletteController::PaletteType::Random, Colors::RGB(0, 0, 0), Colors::RGB(0, 0, 0), false);
					ui->paletteComboBox->blockSignals(true);
					ui->paletteComboBox->addItem(name);
					ui->paletteComboBox->setCurrentText(name);
					ui->paletteComboBox->blockSignals(false);
				}
			}

			populate_palette_canvas_color_selection(maestro_control_widget_->palette_controller_.get_palette(ui->paletteComboBox->currentIndex()));
		}
		else {
			ui->enableCheckBox->setChecked(false);
			ui->frameCountSpinBox->setValue(1);
			ui->currentFrameSpinBox->setValue(0);
			ui->frameIntervalSlider->setValue(100);
			ui->frameIntervalSpinBox->setValue(100);
			set_controls_enabled(0);
		}

		ui->frameCountSpinBox->blockSignals(false);
		ui->currentFrameSpinBox->blockSignals(false);
		ui->frameIntervalSlider->blockSignals(false);
		ui->frameIntervalSpinBox->blockSignals(false);
		ui->enableCheckBox->blockSignals(false);
	}

	/**
	 * Updates the Palettes visible in the Palette drop-down.
	 */
	void CanvasControlWidget::refresh_palettes() {
		QString palette = ui->paletteComboBox->currentText();

		ui->paletteComboBox->blockSignals(true);
		ui->paletteComboBox->clear();

		for (uint16_t i = 0; i < maestro_control_widget_->palette_controller_.get_palettes()->size(); i++) {
			ui->paletteComboBox->addItem(maestro_control_widget_->palette_controller_.get_palette(i)->name);
		}

		ui->paletteComboBox->setCurrentText(palette);
		ui->paletteComboBox->blockSignals(false);
	}

	/**
	 * Enables Canvas brush controls.
	 * @param enabled If true, brush controls are enabled.
	 */
	void CanvasControlWidget::set_brush_controls_enabled(bool enabled) {
		set_circle_controls_enabled(false);
		set_line_controls_enabled(false);
		set_rect_controls_enabled(false);
		set_replace_controls_enabled(false);
		set_text_controls_enabled(false);
		set_triangle_controls_enabled(false);

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
		ui->originLabel->setText("Cursor");
	}

	/**
	 * Enables controls.
	 * @param enabled If true, enable Canvas controls.
	 */
	void CanvasControlWidget::set_controls_enabled(bool enabled) {
		ui->drawingToolsGroupBox->setEnabled(enabled);
		ui->animationToolsGroupBox->setEnabled(enabled);
		ui->loadImageButton->setEnabled(enabled);
		ui->paletteComboBox->setEnabled(enabled);
		ui->paletteLabel->setEnabled(enabled);
		ui->editPaletteButton->setEnabled(enabled);
		ui->colorPickerScrollArea->setEnabled(enabled);
	}

	/**
	 * Sets the Canvas' origin to the specified coordinates.
	 * @param coordinates New coordinates.
	 */
	void CanvasControlWidget::set_canvas_origin(Point* coordinates) {
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
	void CanvasControlWidget::set_circle_controls_enabled(bool enabled) {
		if (enabled) {
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_replace_controls_enabled(false);
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

	/// Sets the interval between Canvas frames.
	void CanvasControlWidget::set_frame_interval() {
		maestro_control_widget_->run_cue(
			maestro_control_widget_->canvas_handler->set_frame_timer(
				maestro_control_widget_->section_control_widget_->get_section_index(),
				maestro_control_widget_->section_control_widget_->get_layer_index(),
				ui->frameIntervalSpinBox->value()
			)
		);
	}

	/**
	 * Enables Canvas line controls.
	 * @param enabled If true, line controls are enabled.
	 */
	void CanvasControlWidget::set_line_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_replace_controls_enabled(false);
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
	 * Enables Canvas rectangle controls.
	 * @param enabled If true, rectangle controls are enabled.
	 */
	void CanvasControlWidget::set_rect_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_replace_controls_enabled(false);
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

	void CanvasControlWidget::set_replace_controls_enabled(bool enabled) {
		if (enabled) {
			set_brush_controls_enabled(false);
			set_circle_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_text_controls_enabled(false);
			set_triangle_controls_enabled(false);
			ui->fillCheckBox->setEnabled(false);
		}

		ui->originLabel->setEnabled(enabled);
		ui->originXSpinBox->setEnabled(enabled);
		ui->originYSpinBox->setEnabled(enabled);
	}

	/**
	 * Enables Canvas text controls.
	 * @param enabled If true, text controls are enabled.
	 */
	void CanvasControlWidget::set_text_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_replace_controls_enabled(false);
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
	void CanvasControlWidget::set_triangle_controls_enabled(bool enabled) {
		if (enabled) {
			set_circle_controls_enabled(false);
			set_line_controls_enabled(false);
			set_rect_controls_enabled(false);
			set_replace_controls_enabled(false);
			set_replace_controls_enabled(false);
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

	CanvasControlWidget::~CanvasControlWidget() {
		delete ui;
	}
}
