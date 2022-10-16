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

#include "ham/json.h"
#include "ham/engine/config.h"

extern "C" {
#define template template_ // eww
#include "mustach.h"
#undef template
}

#define QT_NO_KEYWORDS 1
#include "project.hpp"

#include <QDebug>
#include <QHash>
#include <QStandardPaths>

namespace editor = ham::engine::editor;

//
// Utility functions
//

QString editor::default_project_path(){
	const auto docs_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	return QString("%1/Ham Projects").arg(docs_path);
}

//
// Open project instance
//

editor::project::project(const QDir &dir, QObject *parent)
	: QObject(parent)
	, m_dir(dir)
{
	if(!dir.exists()){
		qWarning() << "Project directory does not exist" << dir;
		throw project_dir_error{};
	}

	const auto json_path = dir.path() + "/ham-engine-project.json";
	if(!QFileInfo::exists(json_path)){
		qWarning() << "Project JSON does not exist" << json_path;
		throw project_dir_error{};
	}

	QFile json_file(json_path);
	if(!json_file.open(QFile::ReadOnly)){
		qWarning() << "Failed to open template JSON" << json_path;
		throw project_dir_error{};
	}

	const auto file_bytes = json_file.readAll();

	const auto json_doc = ham::json_document(ham::str8(file_bytes.data(), file_bytes.size()-1));
	if(!json_doc){
		qWarning() << "Failed to open template JSON" << json_path;
		throw project_json_error{};
	}

	const auto json_root = json_doc.root();

	const auto proj_root = json_root["project"];
	if(!proj_root){
		qWarning() << "Invalid project JSON file" << json_path << "no \"project\" object";
		throw project_json_error{};
	}

	const auto proj_name_json = proj_root["name"];
	if(!proj_name_json){
		qWarning() << "Invalid project JSON file" << json_path << "no \"project.name\" key";
		throw project_json_error{};
	}

	const auto proj_display_json = proj_root["display-name"];
	if(!proj_display_json){
		qWarning() << "Invalid project JSON file" << json_path << "no \"project.display-name\" key";
		throw project_json_error{};
	}

	const auto proj_author_json = proj_root["author"];
	if(!proj_author_json){
		qWarning() << "Invalid project JSON file" << json_path << "no \"project.author\" key";
		throw project_json_error{};
	}

	const auto proj_desc_json = proj_root["desc"];
	if(!proj_desc_json){
		qWarning() << "Invalid template JSON file" << json_path << "no \"project.desc\" key";
		throw project_json_error{};
	}

	const auto proj_name    = proj_name_json.get_str();
	const auto proj_display = proj_display_json.get_str();
	const auto proj_author  = proj_author_json.get_str();
	const auto proj_desc    = proj_desc_json.get_str();

	m_name = QString::fromUtf8(proj_name.ptr());
	m_display_name = QString::fromUtf8(proj_display.ptr());
	m_author = QString::fromUtf8(proj_author.ptr());
	m_desc = QString::fromUtf8(proj_desc.ptr());
}

editor::project::~project(){}

//
// Project templates
//

editor::project_template::project_template(const QDir &dir, QObject *parent)
	: QObject(parent)
	, m_dir(dir)
{
	if(!dir.exists()){
		qWarning() << "Project template directory does not exist" << dir;
		throw project_dir_error{};
	}

	const auto json_path = dir.path() + "/" HAM_ENGINE_EDITOR_TEMPLATE_JSON_PATH;
	if(!QFileInfo::exists(json_path)){
		qWarning() << "Project template JSON does not exist" << json_path;
		throw project_dir_error{};
	}

	QFile json_file(json_path);
	if(!json_file.open(QFile::ReadOnly)){
		qWarning() << "Failed to open template JSON" << json_path;
		throw project_dir_error{};
	}

	const auto file_bytes = json_file.readAll();

	const auto json_doc = ham::json_document(ham::str8(file_bytes.data(), file_bytes.size()-1));
	if(!json_doc){
		qWarning() << "Failed to open template JSON" << json_path;
		throw project_json_error{};
	}

	const auto json_root = json_doc.root();

	const auto tmpl_name_json = json_root["name"];
	if(!tmpl_name_json){
		qWarning() << "Invalid template file" << json_path << "no \"name\" key";
		throw project_json_error{};
	}

	const auto tmpl_author_json = json_root["author"];
	if(!tmpl_author_json){
		qWarning() << "Invalid template file" << json_path << "no \"author\" key";
		throw project_json_error{};
	}

	const auto tmpl_desc_json = json_root["desc"];
	if(!tmpl_desc_json){
		qWarning() << "Invalid template file" << json_path << "no \"desc\" key";
		throw project_json_error{};
	}

	const auto tmpl_name   = tmpl_name_json.get_str();
	const auto tmpl_author = tmpl_author_json.get_str();
	const auto tmpl_desc   = tmpl_desc_json.get_str();

	m_name = QString::fromUtf8(tmpl_name.ptr());
	m_author = QString::fromUtf8(tmpl_author.ptr());
	m_desc = QString::fromUtf8(tmpl_desc.ptr());
}

editor::project_template::~project_template(){}

static bool ham_impl_write_project_template_path(const QDir &tmpl_dir, const QDir &proj_dir, const QHash<QString, QByteArray> &vars){
	static const mustach_itf mustach_fns = {
		.start = nullptr,
		.put = nullptr,
		.enter = [](void*, const char*) -> int{
			return 0;
		},
		.next = [](void*) -> int{
			return 0;
		},
		.leave = [](void*) -> int{
			return 0;
		},
		.partial = nullptr,
		.emit = nullptr,
		.get = [](void *closure_data, const char *name, mustach_sbuf *sbuf) -> int{
			const auto vars = reinterpret_cast<const QHash<QString, QByteArray>*>(closure_data);

			const auto var_res = vars->find(name);
			if(var_res == vars->end()){
				return 0;
			}

			sbuf->closure = nullptr;
			sbuf->freecb = nullptr;
			sbuf->releasecb = nullptr;

			if(var_res->isEmpty()){
				sbuf->value = nullptr;
				sbuf->length = 0;
			}
			else{
				sbuf->value = var_res->data();
				sbuf->length = qMax(var_res->length(), 0);
			}

			return 1;
		},
		.stop = nullptr,
	};

	const auto tmpl_dir_path = tmpl_dir.absolutePath();

	if(!tmpl_dir.exists()){
		const auto tmpl_dir_utf8 = tmpl_dir_path.toUtf8();
		qWarning() << "Project template directory does not exist" << tmpl_dir_path;
		return false;
	}

	const auto proj_dir_path = proj_dir.absolutePath();

	if(!proj_dir.exists() && !QDir().mkdir(proj_dir.absolutePath())){
		const auto proj_dir_utf8 = proj_dir_path.toUtf8();
		qWarning() << "Failed to create project directory" << proj_dir_path;
		return false;
	}

	const auto tmpl_files = tmpl_dir.entryList(QDir::Files);
	const auto tmpl_subdirs = tmpl_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	for(const auto &tmpl_file_name : tmpl_files){
		if(tmpl_file_name == "ham-engine-template.json") continue;

		const auto tmpl_file_path = QString("%1/%2").arg(tmpl_dir_path, tmpl_file_name);

		if(!tmpl_file_name.endsWith(".in")){
			const auto proj_file_path = QString("%1/%2").arg(proj_dir_path, tmpl_file_name);
			if(!QFile::copy(tmpl_file_path, proj_file_path)){
				const auto tmpl_file_utf8 = tmpl_file_path.toUtf8();
				const auto proj_file_utf8 = proj_file_path.toUtf8();

				qWarning() << "Failed to copy template file" << tmpl_file_path << "to" << proj_file_path;
				return false;
			}
		}

		QFile tmpl_file(tmpl_file_path);
		if(!tmpl_file.open(QFile::ReadOnly)){
			const auto tmpl_file_utf8 = tmpl_file_path.toUtf8();
			qWarning() << "Failed to open template file" << tmpl_file_path;
			continue;
		}

		const auto tmpl_file_data = tmpl_file.readAll();

		tmpl_file.close();

		char *mustach_out = nullptr;
		size_t mustach_out_size = 0;

		const int mustach_res = mustach_mem(tmpl_file_data.constData(), qMax(tmpl_file_data.size()-1, 0), &mustach_fns, (void*)&vars, 0, &mustach_out, &mustach_out_size);
		if(mustach_res != 0){
			qWarning() << "Error in mustach_mem(" << tmpl_file_path << ")";
			continue;
		}

		const auto proj_file_path = QString("%1/%2").arg(proj_dir.path(), tmpl_file_path.first(tmpl_file_path.size()-3)); // filename.size()-3 == without .in ext

		QFile out_file(proj_file_path);
		if(!out_file.open(QFile::WriteOnly)){
			qWarning() << "Error opening file for writing" << proj_file_path;
			continue;
		}

		if(out_file.write(mustach_out, mustach_out_size) != mustach_out_size){
			qWarning() << "Error writing file" << proj_file_path;
			continue;
		}

		out_file.close();
	}

	for(const auto &tmpl_subdir : tmpl_subdirs){
		if(tmpl_subdir.startsWith(".")) continue; // skip hidden directories

		const auto inner_tmpl_dir = QString("%1/%2").arg(tmpl_dir_path, tmpl_subdir);
		const auto inner_proj_dir = QString("%1/%2").arg(proj_dir_path, tmpl_subdir);

		if(!ham_impl_write_project_template_path(inner_tmpl_dir, inner_proj_dir, vars)){
			qWarning() << "Error copying template subdirectory" << inner_tmpl_dir;
			return false;
		}
	}

	return true;
}

bool editor::project_template::createInDir(
	const QDir &proj_dir,
	const QString &proj_name,
	const QString &proj_display_name,
	const QString &proj_author,
	const QString &proj_desc
) const
{
	QHash<QString, QByteArray> vars;

	vars["ham_engine_version"] = QByteArrayLiteral(HAM_ENGINE_VERSION_STR);
	vars["ham_engine_version_major"] = QByteArrayLiteral(HAM_STRINGIFY(HAM_ENGINE_VERSION_MAJOR));
	vars["ham_engine_version_minor"] = QByteArrayLiteral(HAM_STRINGIFY(HAM_ENGINE_VERSION_MINOR));
	vars["ham_engine_version_patch"] = QByteArrayLiteral(HAM_STRINGIFY(HAM_ENGINE_VERSION_PATCH));
	vars["ham_project_id"] = QByteArrayLiteral("480");
	vars["ham_project_name"] = proj_name.toUtf8();
	vars["ham_project_display_name"] = proj_display_name.toUtf8();
	vars["ham_project_author"] = proj_author.toUtf8();
	vars["ham_project_description"] = proj_desc.toUtf8();
	vars["ham_project_license"] = "UNLICENSED";
	vars["ham_project_version"] = QByteArrayLiteral("0.0.1");
	vars["ham_project_version_major"] = QByteArrayLiteral("0");
	vars["ham_project_version_minor"] = QByteArrayLiteral("0");
	vars["ham_project_version_patch"] = QByteArrayLiteral("1");

	return ham_impl_write_project_template_path(m_dir, proj_dir, vars);
}
