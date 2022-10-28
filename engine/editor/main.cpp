/*
 * Ham World Engine Editor
 * Copyright (C) 2022 Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/engine/config.h"

#include "welcome_wizard.hpp"
#include "main_window.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QStyle>
#include <QSurface>
#include <QStyleFactory>
#include <QVulkanInstance>
#include <QStandardPaths>
#include <QMessageBox>

namespace editor = ham::engine::editor;

int main(int argc, char *argv[]){
	QApplication::setApplicationDisplayName("Ham World Editor");
	QApplication::setApplicationName("ham-engine-editor");
	QApplication::setApplicationVersion(HAM_ENGINE_VERSION_STR);
	QApplication::setOrganizationName("Hamsmith");
	QApplication::setOrganizationDomain("https://hamsmith.dev/");

//	QSurfaceFormat surfaceFmt;
//	surfaceFmt.setRenderableType(QSurfaceFormat::OpenGL);
//	surfaceFmt.setProfile(QSurfaceFormat::CoreProfile);
//	surfaceFmt.setVersion(4, 6);
//#ifdef HAM_DEBUG
//	surfaceFmt.setOption(QSurfaceFormat::FormatOption::DebugContext);
//#endif
//	surfaceFmt.setRedBufferSize(8);
//	surfaceFmt.setBlueBufferSize(8);
//	surfaceFmt.setGreenBufferSize(8);
//	surfaceFmt.setAlphaBufferSize(8);
//	surfaceFmt.setDepthBufferSize(24);
//	surfaceFmt.setStencilBufferSize(8);
//	surfaceFmt.setSwapBehavior(QSurfaceFormat::TripleBuffer);
//	surfaceFmt.setSwapInterval(1);

//	QSurfaceFormat::setDefaultFormat(surfaceFmt);

	QApplication app(argc, argv);

	QCommandLineParser cmd_parser;
	cmd_parser.setApplicationDescription("The Ham world engine editor.");
	cmd_parser.addHelpOption();
	cmd_parser.addVersionOption();
	cmd_parser.addPositionalArgument("app-dir", QApplication::translate("main", "Game directory"));

	cmd_parser.process(app);

	// setup default project directory (especially important for android)
	const auto documents_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	if(documents_dir.isEmpty()){
		QMessageBox::critical(nullptr, "Fatal Error", "Could not get default document directory");
		return 1;
	}

	const auto default_proj_dir = QString("%1/Ham Projects").arg(documents_dir);
	if(!QDir(default_proj_dir).exists() && !QDir().mkdir(default_proj_dir)){
		QMessageBox::critical(nullptr, "Fatal Error", "Could not create default project directory");
		return 1;
	}

	int font_id = QFontDatabase::addApplicationFont(":/fonts/PressStart2P-Regular.ttf");
	const auto font_fam = QFontDatabase::applicationFontFamilies(font_id).at(0);

	QFont app_font(font_fam);
	app_font.setPointSize(9);

	QStyle *app_style = QStyleFactory::create("fusion");

	// TODO: customize style

	QPalette darkPalette;
	darkPalette.setColor(QPalette::Window, QColor(49, 49, 49));
	darkPalette.setColor(QPalette::WindowText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127,127,127));
	darkPalette.setColor(QPalette::Base, QColor(42,42,42));
	darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
	darkPalette.setColor(QPalette::ToolTipBase, QColor(105, 105, 105));
	darkPalette.setColor(QPalette::ToolTipText, Qt::white);
	darkPalette.setColor(QPalette::Text, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127,127,127));
	darkPalette.setColor(QPalette::Dark, QColor(35,35,35));
	darkPalette.setColor(QPalette::Shadow, QColor(20,20,20));
	darkPalette.setColor(QPalette::Button, QColor(53,53,53));
	darkPalette.setColor(QPalette::ButtonText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127,127,127));
	//darkPalette.setColor(QPalette::BrightText, );
	darkPalette.setColor(QPalette::Link, QColor(255, 120, 0));
	darkPalette.setColor(QPalette::Highlight, QColor(127, 83, 63));
	darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80,80,80));
	darkPalette.setColor(QPalette::HighlightedText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127,127,127));

	QApplication::setStyle(app_style);
	QApplication::setPalette(darkPalette);
	QApplication::setFont(app_font);

	app.setStyleSheet(
		"QToolBar{"
			"border-radius: 0;"
		"}"
		"QPushButton{"
			"padding: 4px 4px 4px 4px;"
			"border-radius: 0;"
		"}"
		"QToolButton{"
			"border-radius: 0;"
		"}"
		"QComboBox{"
			"border-radius: 0;"
		"}"
		"QFontComboBox::down-arrow{"
			"border-radius: 0;"
		"}"
		"QPlainTextEdit{"
			"border-radius: 0;"
		"}"
		"QLineEdit{"
			"border-radius: 0;"
		"}"
		"QListView{"
			"border-radius: 0;"
		"}"
	);

//	QVulkanInstance vk_inst;
//	vk_inst.setLayers({ "VK_LAYER_KHRONOS_validation" });
//	if(!vk_inst.create()){
//		qFatal("Failed to create Vulkan instance: %d", vk_inst.errorCode());
//	}

	const QStringList args = cmd_parser.positionalArguments();
	if(args.empty()){
		editor::welcome_window welcome_window;
		welcome_window.show();

		return app.exec();
	}
	else{
		const auto proj = new editor::project(QDir(args.at(0)));

		editor::main_window window(proj);
		window.show();

		return app.exec();
	}
}
