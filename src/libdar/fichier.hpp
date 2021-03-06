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

    /// \file fichier.hpp
    /// \brief class fichier definition. This is a full implementation of a generic_file applied to a plain file
    /// \ingroup Private

#ifndef FICHIER_HPP
#define FICHIER_HPP


#include "../my_config.h"

extern "C"
{
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
} // end extern "C"

#include "integers.hpp"
#include "thread_cancellation.hpp"
#include "label.hpp"
#include "crc.hpp"
#include "user_interaction.hpp"
#include "mem_ui.hpp"

#include <string>

namespace libdar
{

	/// \addtogroup Private
	/// @{

    class fichier : public generic_file, public thread_cancellation
    {
    public :
	    // constructors
        fichier(user_interaction & dialog, S_I fd);
        fichier(user_interaction & dialog, const char *name, gf_mode m, U_I mode, bool furtive_mode);
        fichier(user_interaction & dialog, const std::string & chemin, gf_mode m, U_I mode, bool furtive_mode);
	fichier(const std::string & chemin, bool furtive_mode = false); // builds a read-only object
	fichier(const fichier & ref) : generic_file(ref) { copy_from(ref); };

	    // assignment operator
	const fichier & operator = (const fichier & ref) { detruit(); copy_from(ref); return *this; };

	    // destructor
	~fichier() { detruit(); };


	    /// set the ownership of the file
	virtual void change_ownership(const std::string & user, const std::string & group);

	    /// change the permission of the file
	virtual void change_permission(U_I perm);

	    /// return the size of the file
        infinint get_size() const;

            // inherited from generic_file
        bool skip(const infinint & pos);
        bool skip_to_eof();
        bool skip_relative(S_I x);
        infinint get_position();

#ifdef LIBDAR_SPECIAL_ALLOC
        USE_SPECIAL_ALLOC(fichier);
#endif
    protected :
        U_I inherited_read(char *a, U_I size);
        void inherited_write(const char *a, U_I size);
	void inherited_sync_write() {};
	void inherited_terminate() {};

    private :
        S_I filedesc;
	user_interaction *x_dialog;

        void open(const char *name, gf_mode m,  U_I perm, bool furtive_mode);
	void copy_from(const fichier & ref);
	void detruit() { close(filedesc); filedesc = -1; if(x_dialog != NULL) { delete x_dialog; x_dialog = NULL; } };
	void init_dialog(user_interaction &dialog);
    };

	/// @}

} // end of namespace

#endif
