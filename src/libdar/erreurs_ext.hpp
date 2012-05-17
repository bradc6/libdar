/*********************************************************************/
// dar - disk archive - a backup/restoration program
// Copyright (C) 2002-2052 Denis Corbin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// to contact the author : http://dar.linux.free.fr/email.html
/*********************************************************************/
// $Id: erreurs_ext.hpp,v 1.4 2011/01/09 17:25:58 edrusb Rel $
//
/*********************************************************************/

    /// \file erreurs_ext.hpp
    /// \brief contains some additional exception class thrown by libdar
    /// \ingroup Private

#ifndef ERREURS_EXT_HPP
#define ERREURS_EXT_HPP

#include "erreurs.hpp"
#include "infinint.hpp"

namespace libdar
{

	/// \addtogroup Private
	/// @{


	/// Ethread_cancel with infinint attribute

    class Ethread_cancel_with_attr : public Ethread_cancel
    {
    public :
	Ethread_cancel_with_attr(bool now, U_64 x_flag, const infinint & attr) : Ethread_cancel(now, x_flag), x_attr(attr) {};

	const infinint get_attr() const { return x_attr; };

    private :
	infinint x_attr;
    };

	/// @}

} // end of namespace

#endif