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
// $Id: test_filesystem.cpp,v 1.8.2.2 2004/04/20 09:27:02 edrusb Rel $
//
/*********************************************************************/

#include "../my_config.h"

extern "C"
{
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif
} // end extern "C"

#include <string.h>
#include <iostream>

#include "filesystem.hpp"
#include "user_interaction.hpp"
#include "test_memory.hpp"
#include "integers.hpp"
#include "shell_interaction.hpp"
#include "cygwin_adapt.hpp"

static void build();
static void test();
static void del();
static void re_test();

using namespace libdar;

static catalogue *cat;

int main()
{
    MEM_BEGIN;
    MEM_IN;
    shell_interaction_init(&cout, &cerr);
    catalogue_set_reading_version("03");
    cat = new catalogue();
    MEM_OUT;
    build();
    MEM_OUT;
    test();
    MEM_OUT;
    {
        MEM_IN;
        re_test();
        MEM_OUT;
        del();
        MEM_OUT;
    }
    delete cat;
    MEM_OUT;
    shell_interaction_close();
    MEM_OUT;
    MEM_END;
}

static void build()
{
    S_I fd;
    char phrase[] = "bonjour les amis il fait chaud il fait beau !";
    struct sockaddr_un name;
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, "arbo/sub/prise");

    mkdir("arbo", 0777);
    mknod("arbo/dev1", 0777 | S_IFCHR, makedev(20, 30));
    mkdir("arbo/sub", 0777);
    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(fd >= 0)
    {
        bind(fd, (const sockaddr *)&name, sizeof(name));
        close(fd);
    }
    mknod("arbo/sub/tube", 0777 | S_IFIFO, 0);
    fd = ::open("arbo/sub/fichier", O_WRONLY|O_CREAT|O_BINARY, 0777);
    if(fd >= 0)
    {
        write(fd, phrase, strlen(phrase));
        close(fd);
    }
    mknod("arbo/dev2", 0777 | S_IFBLK, makedev(20, 30));
    symlink("/yoyo/crotte", "arbo/lien");
}

static void del()
{
    unlink("arbo/sub/fichier");
    unlink("arbo/sub/tube");
    unlink("arbo/sub/prise");
    rmdir("arbo/sub");
    unlink("arbo/dev1");
    unlink("arbo/dev2");
    unlink("arbo/lien");
    rmdir("arbo");
}

static void test()
{
    entree *p;
    filesystem_backup fs = filesystem_backup(path("arbo"), true, true, true, false);

    while(fs.read(p))
    {
        file *f = dynamic_cast<file *>(p);
        cat->add(p);
        if(f != NULL)
        {
            generic_file *entree = f->get_data();

            try
            {
                crc val;

                fichier sortie = dup(1);
                entree->copy_to(sortie, val);
                f->set_crc(val);
            }
            catch(...)
            {
                delete entree;
                throw;
            }
            delete entree;
        }
    }
    cat->listing();
}

static void re_test()
{
    const entree *e;
    detruit det1 = detruit("lien", 'l' | 0x80);
    detruit det2 = detruit("dev1", 'd');
    filesystem_restore fs = filesystem_restore("algi", true, true, true, true, true, false, true, false);

    cat->reset_read();

    while(cat->read(e))
        fs.write(e);

    fs.reset_write();
    fs.write(&det1);
    fs.write(&det2);
}