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

#ifndef HAM_ENGINE_EDITOR_UTIL_HPP
#define HAM_ENGINE_EDITOR_UTIL_HPP 1

#include <QString>
#include <QDir>
#include <QVersionNumber>

#include "ham/typedefs.h"

namespace ham::engine::editor{
	ham_used
	static inline QString to_QString(str8 str){
		return QString::fromUtf8(str.data(), str.len());
	}

	ham_used
	static inline QDir to_QDir(str8 str){
		return QDir(QString::fromUtf8(str.data(), str.size()));
	}

	ham_used
	static inline QVersionNumber to_QVersionNumber(ham_version ver){
		return { ver.major, ver.minor, ver.patch };
	}

	ham_used
	static inline str8 to_str8(const QByteArray &utf8){
		return { utf8.data(), (usize)utf8.size() };
	}

	ham_used
	static inline str8 to_str8(QUtf8StringView view){
		return { view.data(), (usize)view.size() };
	}

	namespace detail{
		template<typename T>
		struct from_str8_helper;

		template<>
		struct from_str8_helper<QString>{
			QString operator()(str8 str) const{
				return QString::fromUtf8(str.data(), str.size());
			}
		};

		template<>
		struct from_str8_helper<QByteArray>{
			QByteArray operator()(str8 str) const{
				return { str.data(), (qsizetype)str.size() };
			}
		};

		template<>
		struct from_str8_helper<QAnyStringView>{
			QAnyStringView operator()(str8 str) const{
				return {str.data(), (qsizetype)str.size()};
			}
		};

		template<>
		struct from_str8_helper<QUtf8StringView>{
			QUtf8StringView operator()(str8 str) const{
				return {str.data(), (qsizetype)str.size()};
			}
		};

		template<>
		struct from_str8_helper<QDir>{
			QDir operator()(str8 str) const{
				return QDir(QString::fromUtf8(str.data(), str.size()));
			}
		};
	}

	template<typename T>
	static inline auto from_str8(str8 str){
		static constexpr detail::from_str8_helper<T> helper;
		return helper(str);
	}
}

#endif // !HAM_ENGINE_EDITOR_UTIL_HPP
