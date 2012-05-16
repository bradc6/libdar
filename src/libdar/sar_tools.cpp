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
// $Id: sar_tools.cpp,v 1.9.2.3 2004/04/20 09:27:02 edrusb Rel $
//
/*********************************************************************/
//

#include "../my_config.h"

extern "C"
{
#if STDC_HEADERS
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif
} // end extern "C"

#include "erreurs.hpp"
#include "user_interaction.hpp"
#include "sar.hpp"
#include "tools.hpp"
#include "tuyau.hpp"
#include "cygwin_adapt.hpp"

using namespace std;

namespace libdar
{

    static void dummy_call(char *x)
    {
        static char id[]="$Id: sar_tools.cpp,v 1.9.2.3 2004/04/20 09:27:02 edrusb Rel $";
        dummy_call(id);
    }

    generic_file *sar_tools_open_archive_fichier(const string &filename, bool allow_over, bool warn_over)
    {
        char *name = tools_str2charptr(filename);
        generic_file *ret = NULL;
        generic_file *tmp = NULL;

        try
        {
            S_I fd;

            if(!allow_over || warn_over)
            {
                struct stat buf;
                if(lstat(name, &buf) < 0)
                {
                    if(errno != ENOENT)
                        throw Erange("open_archive_fichier", string("Error retreiving inode information for ") + name + " : " + strerror(errno));
                }
                else
                {
                    if(!allow_over)
                        throw Erange("open_archive_fichier", filename + " already exists, and overwritten is forbidden, aborting");
                    if(warn_over)
                        user_interaction_pause(filename + " is about to be overwritten, continue ?");
                }
            }

            fd = ::open(name, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
            if(fd < 0)
                throw Erange("open_archive_fichier", string("Error openning file ") + name + " : " + strerror(errno));
            tmp = new fichier(fd);
            if(tmp == NULL)
                throw Ememory("open_archive_fichier");
            ret = new trivial_sar(tmp);
            if(ret == NULL)
                throw Ememory("open_archive_fichier");
        }
        catch(...)
        {
            delete name;
            if(ret != NULL)
                delete ret;
	    if(tmp != NULL)
		delete tmp;
            throw;
        }
        delete name;

        return ret;
    }

    generic_file *sar_tools_open_archive_tuyau(S_I fd, gf_mode mode)
    {
        generic_file *tmp = NULL;
        generic_file *ret = NULL;

        try
        {
            tmp = new tuyau(fd, mode);
            if(tmp == NULL)
                throw Ememory("sar_tools_open_archive_tuyau");
            ret = new trivial_sar(tmp);
            if(ret == NULL)
                throw Ememory("sar_tools_open_archive_tuyau");
        }
        catch(...)
        {
            if(ret != NULL)
                delete ret;
            else // tmp is not managed by ret, which does not exist
                if(tmp != NULL)
                    delete tmp;
            throw;
        }

        return ret;
    }

} // end of namespace