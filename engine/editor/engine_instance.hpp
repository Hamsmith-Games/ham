/*
 * Ham World Engine Editor
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_ENGINE_EDITOR_ENGINE_INSTANCE_HPP
#define HAM_ENGINE_EDITOR_ENGINE_INSTANCE_HPP 1

#include "ham/engine.h"

#include "util.hpp"

#include <QObject>
#include <QDir>
#include <QVersionNumber>
#include <QGuiApplication>

namespace ham::engine::editor{
	class engine_app: public QObject{
		Q_OBJECT

		Q_PROPERTY(ham_engine_app* handle READ handle CONSTANT)

		Q_PROPERTY(quint32 id READ id WRITE set_id NOTIFY id_changed)

		Q_PROPERTY(QDir dir READ dir WRITE set_dir NOTIFY dir_changed)

		Q_PROPERTY(QUtf8StringView name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(QUtf8StringView display_name READ display_name WRITE set_display_name NOTIFY display_name_changed)
		Q_PROPERTY(QUtf8StringView author READ author WRITE set_author NOTIFY author_changed)
		Q_PROPERTY(QUtf8StringView license READ license WRITE set_license NOTIFY license_changed)
		Q_PROPERTY(QUtf8StringView description READ description WRITE set_description NOTIFY description_changed)

		Q_PROPERTY(QVersionNumber version READ version WRITE set_version NOTIFY version_changed)

		Q_PROPERTY(ham_engine_app_init_fn init_fn READ init_fn WRITE set_init_fn NOTIFY init_fn_changed)
		Q_PROPERTY(ham_engine_app_fini_fn fini_fn READ fini_fn WRITE set_fini_fn NOTIFY fini_fn_changed)
		Q_PROPERTY(ham_engine_app_loop_fn loop_fn READ loop_fn WRITE set_loop_fn NOTIFY loop_fn_changed)

		Q_PROPERTY(void* user_data READ user_data WRITE set_user_data NOTIFY user_data_changed)

		public:
			explicit engine_app(const ham_engine_app *app, QObject *parent = nullptr)
				: engine_app(
					app->id,
					from_str8<QDir>(app->dir),
					from_str8<QUtf8StringView>(app->name),
					from_str8<QUtf8StringView>(app->display_name),
					from_str8<QUtf8StringView>(app->author),
					from_str8<QUtf8StringView>(app->license),
					from_str8<QUtf8StringView>(app->description),
					to_QVersionNumber(app->version),
					app->init, app->fini, app->loop,
					app->user,
					parent
				)
			{}

			engine_app(
				quint32 id_,
				const QDir &dir_,
				QUtf8StringView name_,
				QUtf8StringView display_name_,
				QUtf8StringView author_,
				QUtf8StringView license_,
				QUtf8StringView description_,
				QVersionNumber version_,
				ham_engine_app_init_fn init_fn,
				ham_engine_app_fini_fn fini_fn,
				ham_engine_app_loop_fn loop_fn,
				void *user,
				QObject *parent = nullptr
			)
				: QObject(parent)
				, m_dir(dir_.absolutePath().toUtf8())
				, m_name(name_.data(), name_.size())
				, m_display(display_name_.data(), display_name_.size())
				, m_author(author_.data(), author_.size())
				, m_license(license_.data(), license_.size())
				, m_desc(description_.data(), description_.size())
				, m_app{
					.id = id_,
					.dir = to_str8(m_dir),
					.name = to_str8(m_name),
					.display_name = to_str8(m_display),
					.author = to_str8(m_author),
					.license = to_str8(m_license),
					.description = to_str8(m_desc),
					.version = {(u16)version_.majorVersion(), (u16)version_.minorVersion(), (u16)version_.microVersion()},
					.init = init_fn,
					.fini = fini_fn,
					.loop = loop_fn,
					.user = user
				}
			{}

			static engine_app *from_gui_application(QObject *parent = nullptr){
				if(QCoreApplication::startingUp()){
					return nullptr;
				}

				const auto app_name = QGuiApplication::applicationName().toUtf8();
				const auto app_display = QGuiApplication::applicationDisplayName().toUtf8();
				const auto app_author = QGuiApplication::organizationName().toUtf8();

				return new engine_app(
					qHash(app_name), // id
					QGuiApplication::applicationDirPath(), // dir
					app_name,
					app_display,
					app_author,
					"UNSPECIFIED", // license
					"", // description
					QVersionNumber::fromString(QGuiApplication::applicationVersion()),
					[](auto...){ return true; },
					[](auto...){},
					[](auto...){},
					nullptr,
					parent
				);
			}

			ham_engine_app *handle() noexcept{ return &m_app; }
			const ham_engine_app *handle() const noexcept{ return &m_app; }

			quint32 id() const noexcept{ return m_app.id; }

			QDir dir() const{ return QDir(QString::fromUtf8(m_dir.constData(), m_dir.size())); }

			QUtf8StringView name() const{ return m_name; }
			QUtf8StringView display_name() const{ return m_display; }
			QUtf8StringView author() const{ return m_author; }
			QUtf8StringView license() const{ return m_license; }
			QUtf8StringView description() const{ return m_desc; }

			QVersionNumber version() const noexcept{ return to_QVersionNumber(m_app.version); }

			ham_engine_app_init_fn init_fn() const noexcept{ return m_app.init; }
			ham_engine_app_fini_fn fini_fn() const noexcept{ return m_app.fini; }
			ham_engine_app_loop_fn loop_fn() const noexcept{ return m_app.loop; }

			void *user_data() const noexcept{ return m_app.user; }

			void set_id(quint32 new_id){
				if(m_app.id != new_id){
					m_app.id = new_id;
					Q_EMIT id_changed(new_id);
				}
			}

			void set_dir(const QDir &new_dir){
				if(new_dir != dir()){
					const auto dir_str = new_dir.absolutePath();
					m_dir = dir_str.toUtf8();
					m_app.dir = to_str8(m_dir);
					Q_EMIT dir_changed(new_dir);
				}
			}

			void set_name(QUtf8StringView new_name){
				if(new_name != name()){
					m_name = QByteArray(new_name.data(), new_name.size());
					m_app.name = to_str8(m_name);
					Q_EMIT name_changed(m_name);
				}
			}

			void set_display_name(QUtf8StringView new_display){
				if(new_display != display_name()){
					m_display = QByteArray(new_display.data(), new_display.size());
					m_app.display_name = to_str8(m_display);
					Q_EMIT display_name_changed(m_display);
				}
			}

			void set_author(QUtf8StringView new_author){
				if(new_author != author()){
					m_author = QByteArray(new_author.data(), new_author.size());
					m_app.author = to_str8(m_author);
					Q_EMIT author_changed(m_author);
				}
			}

			void set_license(QUtf8StringView new_license){
				if(new_license != license()){
					m_license = QByteArray(new_license.data(), new_license.size());
					m_app.license = to_str8(m_license);
					Q_EMIT license_changed(m_license);
				}
			}

			void set_description(QUtf8StringView new_desc){
				if(new_desc != description()){
					m_desc = QByteArray(new_desc.data(), new_desc.size());
					m_app.description = to_str8(m_desc);
					Q_EMIT description_changed(m_desc);
				}
			}

			void set_version(QVersionNumber new_ver){
				if(new_ver != to_QVersionNumber(m_app.version)){
					m_app.version.major = (u16)new_ver.majorVersion();
					m_app.version.minor = (u16)new_ver.minorVersion();
					m_app.version.patch = (u16)new_ver.microVersion();

					Q_EMIT version_changed(new_ver);
				}
			}

			void set_init_fn(ham_engine_app_init_fn new_init){
				if(new_init != m_app.init){
					m_app.init = new_init;
					Q_EMIT init_fn_changed(new_init);
				}
			}

			void set_fini_fn(ham_engine_app_fini_fn new_fini){
				if(new_fini != m_app.fini){
					m_app.fini = new_fini;
					Q_EMIT fini_fn_changed(new_fini);
				}
			}

			void set_loop_fn(ham_engine_app_loop_fn new_loop){
				if(new_loop != m_app.loop){
					m_app.loop = new_loop;
					Q_EMIT loop_fn_changed(new_loop);
				}
			}

			void set_user_data(void *new_user){
				if(m_app.user != new_user){
					m_app.user = new_user;
					Q_EMIT user_data_changed(new_user);
				}
			}

		Q_SIGNALS:
			void id_changed(quint32);

			void dir_changed(const QDir&);

			void name_changed(QUtf8StringView);
			void display_name_changed(QUtf8StringView);
			void author_changed(QUtf8StringView);
			void license_changed(QUtf8StringView);
			void description_changed(QUtf8StringView);

			void version_changed(QVersionNumber);

			void init_fn_changed(ham_engine_app_init_fn);
			void fini_fn_changed(ham_engine_app_fini_fn);
			void loop_fn_changed(ham_engine_app_loop_fn);

			void user_data_changed(void*);

		private:
			QByteArray m_dir, m_name, m_display, m_author, m_license, m_desc;
			ham_engine_app m_app;
	};

	class engine_instance: public QObject{
		Q_OBJECT

		Q_PROPERTY(engine_app* app READ app CONSTANT)

		public:
			explicit engine_instance(engine_app *app_, typeset_view ts_ = nullptr, QObject *parent = nullptr)
				: QObject(parent)
				, m_app(app_)
			{
				Q_ASSERT(app_);

				if(!app_->parent()){
					app_->setParent(this);
				}

				m_engine = ham_engine_create(app_->handle(), ts_);
				if(!m_engine){
					throw std::runtime_error("Failed to create ham_engine instance");
				}
			}

			explicit engine_instance(QObject *parent = nullptr)
				: engine_instance(engine_app::from_gui_application(), nullptr, parent)
			{
				m_app->setParent(this);
			}

			~engine_instance(){
				ham_engine_destroy(m_engine);
			}

			ham_engine *handle() noexcept{ return m_engine; }
			const ham_engine *handle() const noexcept{ return m_engine; }

			typeset_view ts() const noexcept{ return ham_engine_ts(m_engine); }

			engine_app *app() const noexcept{ return m_app; }

			ham_engine_subsys *net_subsys() noexcept{ return nullptr; }
			ham_engine_subsys *video_subsys() noexcept{ return nullptr; }
			ham_engine_subsys *audio_subsys() noexcept{ return nullptr; }

//			void set_app(engine_app *new_app){
//				if(m_app != new_app){
//					m_app = new_app;
//					Q_EMIT app_changed(new_app);
//				}
//			}

			usize num_subsystems() const noexcept{ return ham_engine_num_subsystems(m_engine); }

			bool request_exit() const noexcept{ return ham_engine_request_exit(m_engine); }

			int exec(){ return ham_engine_exec(m_engine); }

		Q_SIGNALS:
//			void app_changed(engine_app*);

		private:
			engine_app *m_app;
			ham_engine *m_engine;
	};
}

#endif // !HAM_ENGINE_EDITOR_ENGINE_INSTANCE_HPP
