#ifndef SECTIONCONTROLWIDGET_H
#define SECTIONCONTROLWIDGET_H

#include <QWidget>
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class SectionControlWidget;
}

namespace PixelMaestroStudio {
	class SectionControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit SectionControlWidget(QWidget *parent = nullptr);
			~SectionControlWidget();

			void refresh();
			Section* get_active_section();
			uint8_t get_layer_index();
			uint8_t get_layer_index(Section* section);
			uint8_t get_section_index();
			uint8_t get_section_index(Section* section);
			void initialize();
			void set_active_section(Section* section);

		private slots:
			void on_activeSectionComboBox_currentIndexChanged(int index);

			void on_gridSizeXSpinBox_editingFinished();

			void on_gridSizeYSpinBox_editingFinished();

			void on_scrollXSpinBox_editingFinished();

			void on_activeLayerComboBox_currentIndexChanged(int index);

			void on_mixModeComboBox_currentIndexChanged(int index);

			void on_scrollYSpinBox_editingFinished();

			void on_alphaSpinBox_editingFinished();

			void on_offsetXSpinBox_editingFinished();

			void on_offsetYSpinBox_editingFinished();

			void on_layerSpinBox_editingFinished();

		private:
			/// The Section currently being controlled.
			Section* active_section_ = nullptr;

			/// The parent controller widget.
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			Ui::SectionControlWidget *ui;

			uint8_t get_num_layers(Section* section);
			void populate_layer_combobox();
			void set_offset();
			void set_scroll();
			void set_section_size();
			void set_layer_controls_enabled(bool enabled);
	};
}

#endif // SECTIONCONTROLWIDGET_H
