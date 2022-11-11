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

#ifndef HAM_ENGINE_EDITOR_PROJECT_HPP
#define HAM_ENGINE_EDITOR_PROJECT_HPP 1

#include <QException>
#include <QObject>
#include <QDir>
#include <QVersionNumber>

#define HAM_ENGINE_EDITOR_TEMPLATE_JSON_PATH ".ham/engine-template.json"

namespace ham::engine::editor{
	QString default_project_path();

	class project_dir_error: public QException{
		public:
			void raise() const override{ throw *this; }
			project_dir_error *clone() const override{ return new project_dir_error; }
	};

	class project_json_error: public QException{
		public:
			void raise() const override{ throw *this; }
			project_json_error *clone() const override{ return new project_json_error; }
	};

	class project: public QObject{
		Q_OBJECT

		Q_PROPERTY(QDir dir READ dir NOTIFY dir_changed)
		Q_PROPERTY(quint32 id READ id WRITE set_id NOTIFY id_changed)
		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(QString display_name READ name WRITE set_display_name NOTIFY display_name_changed)
		Q_PROPERTY(QString author READ author WRITE set_author NOTIFY author_changed)
		Q_PROPERTY(QVersionNumber version READ version WRITE set_version NOTIFY version_changed)
		Q_PROPERTY(QString license READ license WRITE set_license NOTIFY license_changed)
		Q_PROPERTY(QString description READ description WRITE set_description NOTIFY description_changed)

		public:
			explicit project(const QDir &dir, QObject *parent = nullptr);
			~project();

			const QDir    &dir() const noexcept{ return m_dir; }
			const quint32 &id() const noexcept{ return m_id; }
			const QString &name() const noexcept{ return m_name; }
			const QString &display_name() const noexcept{ return m_display_name; }
			const QString &author() const noexcept{ return m_author; }
			const QString &license() const noexcept{ return m_license; }
			const QString &description() const noexcept{ return m_desc; }

			const QVersionNumber &version() const noexcept{ return m_ver; }

			void set_id(quint32 new_id) noexcept{
				if(new_id != m_id){
					m_id = new_id;
					Q_EMIT id_changed(new_id);
				}
			}

			void set_name(QString new_name) noexcept{
				if(new_name != m_name){
					m_name = std::move(new_name);
					Q_EMIT name_changed(m_name);
				}
			}

			void set_display_name(QString new_display_name) noexcept{
				if(new_display_name != m_display_name){
					m_display_name = std::move(new_display_name);
					Q_EMIT display_name_changed(m_display_name);
				}
			}

			void set_author(QString new_author) noexcept{
				if(new_author != m_author){
					m_author = std::move(new_author);
					Q_EMIT author_changed(m_author);
				}
			}

			void set_version(const QVersionNumber &new_ver) noexcept{
				if(m_ver != new_ver){
					m_ver = new_ver;
					Q_EMIT version_changed(m_ver);
				}
			}

			void set_license(QString new_license) noexcept{
				if(new_license != m_license){
					m_license = std::move(new_license);
					Q_EMIT license_changed(m_license);
				}
			}

			void set_description(QString new_desc) noexcept{
				if(new_desc != m_desc){
					m_desc = std::move(new_desc);
					Q_EMIT description_changed(m_desc);
				}
			}

		Q_SIGNALS:
			void dir_changed(const QDir&);
			void id_changed(quint32);
			void name_changed(const QString&);
			void display_name_changed(const QString&);
			void author_changed(const QString&);
			void version_changed(const QVersionNumber&);
			void license_changed(const QString&);
			void description_changed(const QString&);

		private:
			QDir m_dir;
			quint32 m_id;
			QVersionNumber m_ver;
			QString m_name, m_display_name, m_author, m_license, m_desc;
	};

	class project_template: public QObject{
		Q_OBJECT

		Q_PROPERTY(QDir dir READ dir CONSTANT)
		Q_PROPERTY(QString name READ name CONSTANT)
		Q_PROPERTY(QString author READ author CONSTANT)
		Q_PROPERTY(QString description READ description CONSTANT)

		public:
			explicit project_template(const QDir &dir, QObject *parent = nullptr);
			~project_template();

			const QDir    &dir() const noexcept{ return m_dir; }
			const QString &name() const noexcept{ return m_name; }
			const QString &author() const noexcept{ return m_author; }
			const QString &description() const noexcept{ return m_desc; }

			bool createInDir(
				const QDir &proj_dir,
				const QString &proj_name,
				const QString &proj_display_name,
				const QString &proj_author,
				const QString &proj_description
			) const;

		private:
			QDir m_dir;
			QString m_name, m_author, m_desc;
	};
}

#endif // !HAM_ENGINE_EDITOR_PROJECT_HPP
