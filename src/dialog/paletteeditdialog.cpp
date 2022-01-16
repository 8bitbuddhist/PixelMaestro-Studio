/*
 * PaletteDialog - Dialog for creating and editing Palettes.
 */

#include <QColorDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include "paletteeditdialog.h"
#include "ui_paletteeditdialog.h"
#include "widget/palettecontrolwidget.h"

// NOTE: Consider allowing editing a range of colors instead of having to basically recreate the entire Palette all at once.
namespace PixelMaestroStudio {
	/**
	 * Constructor.
	 * Initializes the form and pre-populates fields when editing a Palette.
	 * @param parent Parent widget.
	 * @param target_palette Palette to edit (if applicable).
	 */
	PaletteEditDialog::PaletteEditDialog(PaletteControlWidget* parent, PaletteController::PaletteWrapper* target_palette_wrapper) : QDialog(parent), ui(new Ui::PaletteEditDialog) {

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		ui->setupUi(this);

		// If a valid Palette was passed in, pre-populate fields
		this->target_palette_ = target_palette_wrapper;
		if (target_palette_wrapper != nullptr) {
			ui->nameLineEdit->setText(target_palette_wrapper->name);
			ui->numColorsSpinBox->setValue(target_palette_wrapper->palette.get_num_colors());
			ui->typeComboBox->setCurrentIndex((uint8_t)target_palette_wrapper->type);
			ui->reverseCheckBox->setChecked(target_palette_wrapper->mirror);
			ui->startSpinBox->setValue(target_palette_wrapper->start);
			ui->lengthSpinBox->setValue(target_palette_wrapper->length);

			this->base_color_ = target_palette_wrapper->base_color;
			parent->set_button_color(ui->baseColorButton, base_color_.r, base_color_.g, base_color_.b);

			this->target_color_ = target_palette_wrapper->target_color;
			parent->set_button_color(ui->targetColorButton, target_color_.r, target_color_.g, target_color_.b);

			on_typeComboBox_currentIndexChanged(ui->typeComboBox->currentIndex());
		}
		else {
			// Hide advanced controls by default
			on_typeComboBox_currentIndexChanged(0);
		}
	}

	/**
	 * Saves changes and closes the form.
	 */
	void PaletteEditDialog::accept() {
		// Don't allow Palettes without names.
		if (ui->nameLineEdit->text().size() <= 0) {
			ui->nameLabel->setStyleSheet(QString("color: red;"));
			QMessageBox::warning(this, "Empty Name", "Palette name cannot be blank.");
			return;
		}

		// Don't allow duplicate name Palettes, unless we're updating an existing Palette
		PaletteController::PaletteWrapper* duplicate = dynamic_cast<PaletteControlWidget*>(parentWidget())->get_palette_controller().get_palette(ui->nameLineEdit->text());
		if ((target_palette_ == nullptr && duplicate != nullptr) ||
			(target_palette_ != nullptr && duplicate != nullptr && duplicate != target_palette_)) {
			ui->nameLabel->setStyleSheet(QString("color: red;"));
			QMessageBox::warning(this, "Duplicate Name", "A Palette with this name already exists.");
			return;
		}

		if (ui->nameLineEdit->text().size() > 0) {
			uint8_t num_colors = ui->numColorsSpinBox->value();
			Colors::RGB colors[num_colors];

			// Handle generation method
			switch ((PaletteController::PaletteType)ui->typeComboBox->currentIndex()) {
				case PaletteController::PaletteType::Blank:
					{
						for (uint8_t i = 0; i < num_colors; i++) {
							colors[i] = {0, 0, 0};
						}
					}
					break;
				case PaletteController::PaletteType::Comet:
					Colors::generate_comet(&colors[0], num_colors, base_color_, target_color_, ui->startSpinBox->value(), ui->lengthSpinBox->value());
					break;
				case PaletteController::PaletteType::Scaling:
					Colors::generate_scaling_color_array(&colors[0], base_color_, target_color_, num_colors, (bool)ui->reverseCheckBox->isChecked());
					break;
				case PaletteController::PaletteType::Random:
					QRandomGenerator* random = QRandomGenerator::global();
					for (uint8_t i = 0; i < num_colors; i++) {
						colors[i].r = random->generate() % 256;
						colors[i].g = random->generate() % 256;
						colors[i].b = random->generate() % 256;
					}
					break;
			}

			// If the target Palette already exists update it. Otherwise, create a new one.
			if (target_palette_ != nullptr) {
				target_palette_->name = ui->nameLineEdit->text();

				// Check to see if we need to update the Palette's color scheme
				if (colors_changed_) {
					target_palette_->palette.set_colors(&colors[0], num_colors);
					target_palette_->type = (PaletteController::PaletteType)ui->typeComboBox->currentIndex();
					target_palette_->base_color = base_color_;
					target_palette_->target_color = target_color_;
					target_palette_->mirror = ui->reverseCheckBox->isChecked();
					target_palette_->start = ui->startSpinBox->value();
					target_palette_->length = ui->lengthSpinBox->value();

					colors_changed_ = false;
				}
			}
			else {
				// Add the new Palette
				PaletteControlWidget* parent = dynamic_cast<PaletteControlWidget*>(parentWidget());
				parent->get_palette_controller().add_palette(ui->nameLineEdit->text(), &colors[0], num_colors, (PaletteController::PaletteType)ui->typeComboBox->currentIndex(), base_color_, target_color_, ui->reverseCheckBox->isChecked(), ui->startSpinBox->value(), ui->lengthSpinBox->value());
			}

			QDialog::accept();
		}
	}

	/**
	 * Opens a color picker for the base color.
	 */
	void PaletteEditDialog::on_baseColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Base Color");
		base_color_ = {(uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue()};
		PaletteControlWidget* parent = dynamic_cast<PaletteControlWidget*>(parentWidget());
		parent->set_button_color(ui->baseColorButton, base_color_.r, base_color_.g, base_color_.b);

		if (target_palette_ != nullptr && (base_color_ != target_palette_->target_color)) {
			this->colors_changed_ = true;
		}
	}

	void PaletteEditDialog::on_numColorsSpinBox_valueChanged(int arg1) {
		if (target_palette_ != nullptr && (arg1 != target_palette_->palette.get_num_colors())) {
			this->colors_changed_ = true;
		}
	}

	void PaletteEditDialog::on_reverseCheckBox_stateChanged(int arg1) {
		if (target_palette_ != nullptr && (arg1 != target_palette_->mirror)) {
			this->colors_changed_ = true;
		}
	}

	/**
	 * Opens a color picker for the target color.
	 */
	void PaletteEditDialog::on_targetColorButton_clicked() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Target Color");
		target_color_ = {(uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue()};
		PaletteControlWidget* parent = dynamic_cast<PaletteControlWidget*>(parentWidget());
		parent->set_button_color(ui->targetColorButton, target_color_.r, target_color_.g, target_color_.b);

		if (target_palette_ != nullptr && (target_color_ != target_palette_->target_color)) {
			this->colors_changed_ = true;
		}
	}

	/**
	 * Adds or removes certain fields based on the type of Palette selected.
	 * @param index Palette type.
	 */
	void PaletteEditDialog::on_typeComboBox_currentIndexChanged(int index) {
		// SetEnabled doesn't visually change widgets, so we use setVisible instead
		bool color_pickers = (index == (int)PaletteController::PaletteType::Comet || index == (int)PaletteController::PaletteType::Scaling);

		ui->baseColorLabel->setVisible(color_pickers);
		ui->baseColorButton->setVisible(color_pickers);
		ui->targetColorLabel->setVisible(color_pickers);
		ui->targetColorButton->setVisible(color_pickers);
		ui->reverseCheckBox->setVisible(index == (int)PaletteController::PaletteType::Scaling);
		ui->lengthLabel->setVisible(index == (int)PaletteController::PaletteType::Comet);
		ui->lengthSpinBox->setVisible(index == (int)PaletteController::PaletteType::Comet);
		ui->startLabel->setVisible(index == (int)PaletteController::PaletteType::Comet);
		ui->startSpinBox->setVisible(index == (int)PaletteController::PaletteType::Comet);

		if (target_palette_ != nullptr && (index != (int)target_palette_->type)) {
			this->colors_changed_ = true;
		}
	}

	PaletteEditDialog::~PaletteEditDialog() {
		delete ui;
	}
}
