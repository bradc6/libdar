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

    /// \file tronc.hpp
    /// \brief defines a limited segment over another generic_file.
    /// \ingroup Private
    ///
    /// This is used to read a part of a file as if it was a real file generating
    /// end of file behavior when reaching the given length.

#ifndef TRONC_HPP
#define TRONC_HPP

#include "../my_config.h"
#include "infinint.hpp"
#include "generic_file.hpp"

namespace libdar
{
	/// \addtogroup Private
	/// @{

	/// makes a segment of a generic_file appear like a real generic_file

    class tronc : public generic_file
    {
    public :
	    /// constructor

	    //! \param f is the file to take the segment from
	    //! \param offset is the position of the beginning of the segment
	    //! \param size is the size of the segment
	    //! \param own_f is true if this object has to owns and may destroy the 'f' object at tronc's destruction time
        tronc(generic_file *f, const infinint &offset, const infinint &size, bool own_f = false);
        tronc(generic_file *f, const infinint &offset, const infinint &size, gf_mode mode, bool own_f = false);

	    /// other constructor, the end of the segment is the end of the underlying generic_file
	    /// only data before offset is inaccessible
        tronc(generic_file *f, const infinint &offset, bool own_f = false);
        tronc(generic_file *f, const infinint &offset, gf_mode mode, bool own_f = false);

	    /// destructor
	~tronc()
	{
	    if(own_ref)
		delete ref;
	};

	    /// inherited from generic_file
        bool skip(const infinint & pos);
	    /// inherited from generic_file
        bool skip_to_eof();
	    /// inherited from generic_file
        bool skip_relative(S_I x);
	    /// inherited from generic_file
        infinint get_position() { return current; };

#ifdef LIBDAR_SPECIAL_ALLOC
        USE_SPECIAL_ALLOC(tronc);
#endif
    protected :
	    /// inherited from generic_file
        U_I inherited_read(char *a, U_I size);
	    /// inherited from generic_file
        void inherited_write(const char *a, U_I size);
	void inherited_sync_write() { ref->sync_write(); }
	void inherited_terminate() {if(own_ref) ref->terminate(); };

    private :
        infinint start;  //< offset in the global generic file to start at
	infinint sz;     //< length of the portion to consider
        generic_file *ref; //< global generic file of which to take a piece
        infinint current;  //< inside position of the next read or write
	bool own_ref;      //< whether we own ref (and must destroy it when no more needed)
	bool limited;      //< whether the sz argument is to be considered

	void set_back_current_position();
    };

	/// @}

} // end of namespace

#endif
