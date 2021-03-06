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

#include "../my_config.h"

extern "C"
{
#if HAVE_STDIO_H
#include <stdio.h>
#endif
} // end extern "C"

#include <string>

#include "scrambler.hpp"
#include "dar_suite.hpp"
#include "generic_file.hpp"
#include "integers.hpp"
#include "fichier.hpp"
#include "tools.hpp"

using namespace libdar;

S_I little_main(user_interaction & dialog, S_I argc, char * const argv[], const char **env);

int main(S_I argc, char * const argv[])
{
    return dar_suite_global(argc,
			    argv,
			    NULL,
			    "",
			    NULL,
			    &little_main);
}

S_I little_main(user_interaction & dialog, S_I argc, char * const argv[], const char **env)
{
    if(argc != 4)
    {
        printf("usage: %s <source> <destination_scrambled> <destination_clear>\n", argv[0]);
        return EXIT_SYNTAX;
    }

    fichier *src = new fichier(dialog, argv[1], gf_read_only, tools_octal2int("0777"), false);
    fichier *dst = new fichier(dialog, argv[2], gf_write_only, tools_octal2int("0777"), false);
    std::string pass = "bonjour";
    scrambler *scr = new scrambler(secu_string(pass.c_str(), pass.size()), *dst);

    src->copy_to(*scr);

    delete scr; scr = NULL;
    delete dst; dst = NULL;
    delete src; src = NULL;

    src = new fichier(dialog, argv[2], gf_read_only, tools_octal2int("0777"), false);
    scr = new scrambler(secu_string(pass.c_str(), pass.size()), *src);
    dst = new fichier(dialog, argv[3], gf_write_only, tools_octal2int("0777"), false);

    scr->copy_to(*dst);

    delete scr;
    delete src;
    delete dst;

    return EXIT_OK;
}
