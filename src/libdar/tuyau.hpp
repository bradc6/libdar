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
// to contact the author : dar.linux@free.fr
/*********************************************************************/
// $Id: tuyau.hpp,v 1.6 2003/12/02 16:49:17 edrusb Rel $
//
/*********************************************************************/

#ifndef TUYAU_HPP
#define TUYAU_HPP

#pragma interface

#include "../my_config.h"
#include "generic_file.hpp"

namespace libdar
{

    class tuyau : public generic_file
    {
    public:
        tuyau(int fd); // fd is the filedescriptor of a pipe extremity
        tuyau(int fd, gf_mode mode); // forces the mode if possible
        tuyau(const std::string &filename, gf_mode mode);
        ~tuyau() { close(filedesc); };
    
            // inherited from generic_file
        bool skip(const infinint & pos);
        bool skip_to_eof();
        bool skip_relative(signed int x);
        infinint get_position() { return position; };

    protected:
        virtual int inherited_read(char *a, size_t size);
        virtual int inherited_write(char *a, size_t size);

    private:
        infinint position;
        int filedesc;
        std::string chemin; // named pipe open later

        void ouverture();
    };

} // end of namespace

#endif