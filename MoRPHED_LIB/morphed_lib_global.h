/*
This file is part of MoRPHED

MoRPHED is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoRPHED is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

(c)2015 MoRPHED Development team
*/
#ifndef MORPHED_LIB_GLOBAL_H
#define MORPHED_LIB_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore>

#if defined(MORPHED_LIB_LIBRARY)
#  define MORPHED_LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MORPHED_LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MORPHED_LIB_GLOBAL_H
