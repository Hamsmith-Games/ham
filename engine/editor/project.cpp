/*
 * Ham World Engine Editor
 * Copyright (C) 2022  Hamsmith Ltd.
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

	const auto json_path = dir.path() + "/ham-engine-template.json";
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

bool editor::project_template::createInDir(
	const QDir &proj_dir,
	const QString &proj_name,
	const QString &proj_display_name,
	const QString &proj_author,
	const QString &proj_desc
) const
{
	/*
	int (*start)(void *closure);
	int (*put)(void *closure, const char *name, int escape, FILE *file);
	int (*enter)(void *closure, const char *name);
	int (*next)(void *closure);
	int (*leave)(void *closure);
	int (*partial)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	int (*emit)(void *closure, const char *buffer, size_t size, int escape, FILE *file);
	int (*get)(void *closure, const char *name, struct mustach_sbuf *sbuf);
	void (*stop)(void *closure, int status);
	*/

	// enter next leave

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

	const auto tmpl_files = m_dir.entryList(QDir::Files);
	for(auto &&file_name : tmpl_files){
		if(file_name == "ham-engine-template.json") continue;

		const auto tmpl_file_path = QString("%1/%2").arg(m_dir.path(), file_name);

		if(!file_name.endsWith(".in")){
			if(!QFile::copy(tmpl_file_path, QString("%1/%2").arg(proj_dir.path(), file_name))){
				qWarning() << "Failed to copy template file" << file_name << "to" << QString("%1/%2").arg(proj_dir.path(), file_name);
			}

			continue;
		}

		QFile tmpl_file(tmpl_file_path);
		if(!tmpl_file.open(QFile::ReadOnly)){
			qWarning() << "Failed to open template file" << file_name;
			continue;
		}

		const auto tmpl_file_data = tmpl_file.readAll();

		char *mustach_out = nullptr;
		size_t mustach_out_size = 0;

		const int mustach_res = mustach_mem(tmpl_file_data.constData(), qMax(tmpl_file_data.size()-1, 0), &mustach_fns, &vars, 0, &mustach_out, &mustach_out_size);
		if(mustach_res != 0){
			qWarning() << "Error in mustach_mem(" << file_name << ")";
			continue;
		}

		const auto out_file_path = QString("%1/%2").arg(proj_dir.path(), file_name.first(file_name.size()-3)); // filename.size()-3 == without .in ext

		QFile out_file(out_file_path);
		if(!out_file.open(QFile::WriteOnly)){
			qWarning() << "Error opening file for writing" << out_file_path;
			continue;
		}

		if(out_file.write(mustach_out, mustach_out_size) != mustach_out_size){
			qWarning() << "Error writing file" << out_file_path;
			continue;
		}
	}

	return true;
}
