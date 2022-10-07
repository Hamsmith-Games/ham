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

#ifndef HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP
#define HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP 1

#include <QMainWindow>
#include <QAbstractItemModel>
#include <QWizard>
#include <QDir>

#include "window.hpp"
#include "project.hpp"

namespace ham::engine::editor{
	class project_wizard: public QWizard{
		Q_OBJECT

		private:
			class initial_page: public QWizardPage{
				public:
					explicit initial_page(QWidget *parent = nullptr);
					~initial_page();

					bool validatePage() override;
			};

			class info_page: public QWizardPage{
				public:
					explicit info_page(QWidget *parent = nullptr);
					~info_page();

					bool validatePage() override;
			};

			class template_model: public QAbstractItemModel{
				public:
					enum template_role{
						NameRole = Qt::UserRole,
						DirRole,
						AuthorRole,
						DescriptionRole,
					};

					explicit template_model(QObject *parent = nullptr)
						: QAbstractItemModel(parent){}

					QModelIndex parent(const QModelIndex&) const override{
						return QModelIndex();
					}

					QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override{
						if(column != 0 || row >= m_data.size() || parent != QModelIndex()){
							return QModelIndex();
						}

						return createIndex(row, 0, m_data[row]);
					}

					int rowCount(const QModelIndex &parent = QModelIndex()) const override{
						if(parent != QModelIndex()){
							return 0;
						}

						return m_data.size();
					}

					int columnCount(const QModelIndex &parent = QModelIndex()) const override{
						if(parent != QModelIndex()){
							return 0;
						}

						return 1;
					}

					QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
						if(index.column() != 0 || index.row() >= m_data.size()){
							return QVariant();
						}

						switch(role){
							case Qt::DisplayRole:
							case NameRole:
								return QVariant(m_data[index.row()]->name());

							case DirRole: return QVariant(m_data[index.row()]->dir().path());

							case AuthorRole: return QVariant(m_data[index.row()]->author());
							case DescriptionRole: return QVariant(m_data[index.row()]->description());

							default: return QVariant();
						}

						return QVariant();
					}

					bool set_templates(QList<project_template*> new_tmpls){
						if(new_tmpls != m_data){
							m_data = std::move(new_tmpls);
							if(!m_data.isEmpty()){
								Q_EMIT dataChanged(createIndex(0, 0, m_data.first()), createIndex(m_data.size()-1, 0, m_data.last()));
							}
						}

						return true;
					}

				private:
					QList<project_template*> m_data;
			};

			class template_page: public QWizardPage{
				public:
					explicit template_page(QWidget *parent = nullptr);
					~template_page();

					bool validatePage() override;

					project_template *current_template() const noexcept{ return m_cur_tmpl; }

				private:
					project_template *m_cur_tmpl = nullptr;
			};

		public:
			explicit project_wizard(QWidget *parent = nullptr);
			~project_wizard();
	};

	class welcome_window: public window{
		Q_OBJECT

		public:
			explicit welcome_window(QWidget *parent = nullptr);
			~welcome_window();

		protected:
			void new_proj_pressed();
			void open_proj_pressed();

		private:
			QWidget *createWelcomeSplash();

			QWidget *m_inner;
			QWidget *m_splash;
	};
}

#endif // !HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP
