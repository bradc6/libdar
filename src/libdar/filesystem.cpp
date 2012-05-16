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
// $Id: filesystem.cpp,v 1.17.2.5 2004/07/13 22:37:33 edrusb Rel $
//
/*********************************************************************/

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

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_TIME_H
#include <time.h>
#endif

#if HAVE_UTIME_H
#include <utime.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_TYPE_H
#include <sys/types.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if STDC_HEADERS
#include <ctype.h>
#endif

#ifdef LIBDAR_NODUMP_FEATURE
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if LIBDAR_NODUMP_FEATURE == linux
#include <linux/ext2_fs.h>
#else
#if LIBDAR_NODUMP_FEATURE == ext2fs
#include <ext2fs/ext2_fs.h>
#else
#error "unknown location of ext2_fs.h include file"
#endif
#endif
#endif

#if MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#else
#if MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#endif
} // end extern "C"


#include <map>

#include "tools.hpp"
#include "erreurs.hpp"
#include "filesystem.hpp"
#include "user_interaction.hpp"
#include "catalogue.hpp"
#include "ea_filesystem.hpp"
#include "cygwin_adapt.hpp"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

using namespace std;

namespace libdar
{

    static void supprime(const path & ref);
    static void make_owner_perm(const inode & ref, const path & ou, bool dir_perm, bool ignore_ownership);
    static void make_date(const string & chemin, infinint access, infinint modif);
    static void attach_ea(const string &chemin, inode *ino, bool ea_root_mode, bool ea_user_mode);
    static void set_back_dir_dates(const string & chem, const infinint & last_acc, const infinint & last_mod);
    static bool is_nodump_flag_set(const path & chem, const string & filename);
    static path *get_root_with_symlink(const path & root, bool info_details);

///////////////////////////////////////////////////////////////////
///////////////// filesystem_hard_link_read methods ///////////////
///////////////////////////////////////////////////////////////////

    nomme *filesystem_hard_link_read::make_read_entree(path & lieu, const string & name, bool see_hard_link, bool ea_root_mode, bool ea_user_mode)
    {
        char *ptr_name = name != "" ?
            tools_str2charptr((lieu + path(name)).display())
            : tools_str2charptr(lieu.display());
        nomme *ref = NULL;

        try
        {
            struct stat buf;

            if(lstat(ptr_name, &buf) < 0)
            {
                switch(errno)
                {
                case EACCES:
                    user_interaction_warning(string("error reading inode of ") + ptr_name + " : " + strerror(errno));
                    break;
                case ENOENT:
                    break;
                default:
                    throw Erange("filesystem_hard_link_read::make_read_entree", string("Cannot read inode for ") + ptr_name + " : " + strerror(errno));
                }

                    // the function returns NULL (meaning file does not exists)
            }
            else
            {
                if(S_ISLNK(buf.st_mode))
                {
		    string pointed = tools_readlink(ptr_name);

		    ref = new lien(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
				   buf.st_atime,
				   buf.st_mtime,
				   name,
				   pointed);
                }
                else if(S_ISREG(buf.st_mode))
                {
                    if(buf.st_nlink == 1 || ! see_hard_link) // file without hard link
                        ref = new file(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                       buf.st_atime,
                                       buf.st_mtime,
                                       name,
                                       lieu,
                                       buf.st_size);
                    else // file with hard link(s)
                    {
                        map <ino_t, couple>::iterator it = corres_read.find(buf.st_ino);

                        if(it == corres_read.end()) // inode not seen yet, first link on it
                        {
                            file_etiquette *ptr = new file_etiquette(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                                                     buf.st_atime,
                                                                     buf.st_mtime,
                                                                     name,
                                                                     lieu,
                                                                     buf.st_size);
                            ref = ptr;
                            if(ref != NULL)
                            {
                                couple tmp;
                                tmp.count = buf.st_nlink - 1;
                                tmp.obj = ptr;
                                corres_read[buf.st_ino] = tmp;
                            }
                        }
                        else  // inode already seen previously
                        {
                            ref = new hard_link(name, it->second.obj);

                            if(ref != NULL)
                            {
                                it->second.count--;
                                if(it->second.count == 0)
                                    corres_read.erase(it);
                            }
                        }
                    }
                }
                else if(S_ISDIR(buf.st_mode))
                {
                    ref = new directory(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                        buf.st_atime,
                                        buf.st_mtime,
                                        name);
                }
                else if(S_ISCHR(buf.st_mode))
                    ref = new chardev(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                      buf.st_atime,
                                      buf.st_mtime,
                                      name,
                                      major(buf.st_rdev),
                                      minor(buf.st_rdev)); // makedev(major, minor)
                else if(S_ISBLK(buf.st_mode))
                    ref = new blockdev(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                       buf.st_atime,
                                       buf.st_mtime,
                                       name,
                                       major(buf.st_rdev),
                                       minor(buf.st_rdev)); // makedev(major, minor)
                else if(S_ISFIFO(buf.st_mode))
                    ref = new tube(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                   buf.st_atime,
                                   buf.st_mtime,
                                   name);
                else if(S_ISSOCK(buf.st_mode))
                    ref = new prise(buf.st_uid, buf.st_gid, buf.st_mode & 07777,
                                    buf.st_atime,
                                    buf.st_mtime,
                                    name);
                else
                    throw Erange("filesystem_hard_link_read::make_read_entree", string("Unknown file type ! file is ") + string(ptr_name));

                if(ref == NULL)
                    throw Ememory("filesystem_hard_link_read::make_read_entree");

                inode *ino = dynamic_cast<inode *>(ref);
                if(ino != NULL)
                {
                    try
                    {
                        attach_ea(ptr_name, ino, ea_root_mode, ea_user_mode);
                        if(ino->ea_get_saved_status() != inode::ea_none)
                            ino->set_last_change(buf.st_ctime);
                    }
                    catch(Ebug & e)
                    {
                        throw;
                    }
                    catch(Euser_abort & e)
                    {
                        throw;
                    }
                    catch(Ememory & e)
                    {
                        throw;
                    }
                    catch(Egeneric & ex)
                    {
                        user_interaction_warning(string("Error reading EA for ")+ptr_name+ " : " + ex.get_message());
                            // no throw !
                            // we must be able to continue without EA
                    }
                }
            }
        }
        catch(...)
        {
            delete ptr_name;
            throw;
        }
        delete ptr_name;

        return ref;

    }

    void filesystem_hard_link_read::forget_etiquette(file_etiquette *obj)
    {
        map<ino_t, couple>::iterator it = corres_read.begin();

        while(it != corres_read.end() && it->second.obj != obj)
            it++;

        if(it != corres_read.end())
            corres_read.erase(it);
    }


///////////////////////////////////////////////////////////////////
///////////////// filesystem_backup methods ///////////////////////
///////////////////////////////////////////////////////////////////


    filesystem_backup::filesystem_backup(const path &root, bool x_info_details, bool x_save_root_ea, bool x_save_user_ea, bool check_no_dump_flag)
    {
	fs_root = get_root_with_symlink(root, x_info_details);
	if(fs_root == NULL)
	    throw Ememory("filesystem_backup::filesystem_backup");
	info_details = x_info_details;
	save_root_ea = x_save_root_ea;
	save_user_ea = x_save_user_ea;
	no_dump_check = check_no_dump_flag;
	current_dir = NULL;
        reset_read();
    }

    void filesystem_backup::detruire()
    {
        if(fs_root != NULL)
        {
            delete fs_root;
            fs_root = NULL;
        }
        if(current_dir != NULL)
            delete current_dir;
    }

    void filesystem_backup::copy_from(const filesystem_backup & ref)
    {
        if(ref.fs_root != NULL)
            fs_root = new path(*ref.fs_root);
        else
            fs_root = NULL;

        if(ref.current_dir != NULL)
            current_dir = new path(*ref.current_dir);
        else
            current_dir = NULL;
        info_details = ref.info_details;
        save_root_ea = ref.save_root_ea;
        save_user_ea = ref.save_user_ea;
        no_dump_check = ref.no_dump_check;
        filename_pile = ref.filename_pile;
        pile = ref.pile;
    }

    void filesystem_backup::reset_read()
    {
        char *tmp;

        corres_reset();
        file_etiquette::reset_etiquette_counter();
        if(current_dir != NULL)
            delete current_dir;
        current_dir = new path(*fs_root);
        filename_pile.clear();
        if(current_dir == NULL)
            throw Ememory("filesystem_backup::reset_read");
        pile.clear();

        tmp = tools_str2charptr(current_dir->display());
        try
        {
            entree *ref = make_read_entree(*current_dir, "", true, save_root_ea, save_user_ea);
            directory *ref_dir = dynamic_cast<directory *>(ref);
            try
            {
                pile.push_back(etage(tmp));
                if(ref_dir != NULL)
                {
                    filename_struct rfst;

                    rfst.last_acc = pile.back().last_acc = ref_dir->get_last_access();
                    rfst.last_mod = pile.back().last_mod = ref_dir->get_last_modif();
                    filename_pile.push_back(rfst);
                }
                else
                    if(ref == NULL)
                        throw Erange("filesystem_backup::reset_read", string("Non existant file: ") + tmp);
                    else
                        throw Erange("filesystem_backup::reset_read", string("File must be a directory: ")+ tmp);
            }
            catch(...)
            {
                if(ref != NULL)
                    delete ref;
                throw;
            }
            if(ref != NULL)
                delete ref;
        }
        catch(...)
        {
            delete tmp;
            throw;
        }
        delete tmp;
    }


    bool filesystem_backup::read(entree * & ref)
    {
        bool once_again;
        ref = NULL;

        if(current_dir == NULL)
            throw SRC_BUG; // constructor not called or badly implemented.

        do
        {
            once_again = false;

            if(pile.size() == 0)
                return false; // end of filesystem reading
            else // at least one directory to read
            {
                etage & inner = pile.back();
                string name;

                if(!inner.read(name))
                {
                    string tmp;

                    set_back_dir_dates(current_dir->display(), inner.last_acc, inner.last_mod);
                    pile.pop_back();
                    if(pile.size() > 0)
                    {
                        if(! current_dir->pop(tmp))
                            throw SRC_BUG;
                        ref = new eod();
                    }
                    else
                        return false; // end of filesystem
                }
                else // could read a filename in directory
                {
                    try
                    {
                            // checking the EXT2 nodump flag (if set ignoring the file)

                        if(!no_dump_check || !is_nodump_flag_set(*current_dir, name))
                        {
                            ref = make_read_entree(*current_dir, name, true, save_root_ea, save_user_ea);
                            directory *ref_dir = dynamic_cast<directory *>(ref);

                            if(ref_dir != NULL)
                            {
                                char *ptr_name;

                                *current_dir += name;
                                ptr_name = tools_str2charptr(current_dir->display());

                                try
                                {
                                    pile.push_back(etage(ptr_name));
                                    pile.back().last_acc = ref_dir->get_last_access();
                                    pile.back().last_mod = ref_dir->get_last_modif();
                                }
                                catch(Erange & e)
                                {
                                    string tmp;

                                    user_interaction_warning(string("error openning ") + ptr_name + " : " + e.get_message());
                                    try
                                    {
                                        pile.push_back(etage());
                                        pile.back().last_acc = 0;
                                        pile.back().last_mod = 0;
                                    }
                                    catch(Erange & e)
                                    {
                                        delete ref;
                                        once_again = true;
                                        ref = NULL;
                                        if(! current_dir->pop(tmp))
                                            throw SRC_BUG;
                                    }
                                }
                                catch(Egeneric & e)
                                {
                                    delete ptr_name;
                                    delete ref;
                                    throw;
                                }

                                delete ptr_name;
                            }


                            if(ref == NULL)
                                once_again = true;
                                // the file has been removed between the time
                                // the directory has been openned, and the time
                                // we try to read it, so we ignore it.
                        }
                        else // EXT2 nodump flag is set, and we must not consider such file for backup
                        {
                            if(info_details)
                                user_interaction_warning(string("ignoring file with NODUMP flag set: ") + (*current_dir + name).display());
                            once_again = true;
                        }
                    }
                    catch(Erange & e)
                    {
                        user_interaction_warning("error reading directory contents : " + e.get_message() + " . Ignoring file or directory");
                        once_again = true;
                    }
                }
            }
        }
        while(once_again);

        if(ref == NULL)
            throw Ememory("filesystem_backup::read");
        else
            return true;
    }

    void filesystem_backup::skip_read_to_parent_dir()
    {
        string tmp;

        if(pile.size() == 0)
            throw SRC_BUG;
        else
        {
            set_back_dir_dates(current_dir->display(), pile.back().last_acc, pile.back().last_mod);
            pile.pop_back();
        }

        if(! current_dir->pop(tmp))
            throw SRC_BUG;
    }


///////////////////////////////////////////////////////////////////
////////////////// filesystem_diff methods  ///////////////////////
///////////////////////////////////////////////////////////////////


    filesystem_diff::filesystem_diff(const path &root, bool x_info_details, bool x_check_root_ea, bool x_check_user_ea)
    {
	fs_root = get_root_with_symlink(root, x_info_details);
	if(fs_root == NULL)
	    throw Ememory("filesystem_diff::filesystem_diff");
	info_details = x_info_details;
	check_root_ea = x_check_root_ea;
	check_user_ea = x_check_user_ea;
	current_dir = NULL;

        reset_read();
    }


    void filesystem_diff::reset_read()
    {
        char *tmp;

        corres_reset();
        file_etiquette::reset_etiquette_counter();
        if(current_dir != NULL)
            delete current_dir;
        current_dir = new path(*fs_root);
        filename_pile.clear();
        if(current_dir == NULL)
            throw Ememory("filesystem_diff::reset_read");
        tmp = tools_str2charptr(current_dir->display());
        try
        {
            entree *ref = make_read_entree(*current_dir, "", true, check_root_ea, check_user_ea);
            directory *ref_dir = dynamic_cast<directory *>(ref);
            try
            {
                if(ref_dir != NULL)
                {
                    filename_struct rfst;

                    rfst.last_acc = ref_dir->get_last_access();
                    rfst.last_mod = ref_dir->get_last_modif();
                    filename_pile.push_back(rfst);
                }
                else
                    if(ref == NULL)
                        throw Erange("filesystem_diff::reset_read", string("Non existant file: ") + tmp);
                    else
                        throw Erange("filesystem_diff::reset_read", string("File must be a directory: ") + tmp);
            }
            catch(...)
            {
                if(ref != NULL)
                    delete ref;
                throw;
            }
            if(ref != NULL)
                delete ref;
        }
        catch(...)
        {
            delete tmp;
            throw;
        }
        delete tmp;
    }

    bool filesystem_diff::read_filename(const string & name, nomme * &ref)
    {
        directory *ref_dir = NULL;
        if(current_dir == NULL)
            throw SRC_BUG;
        ref = make_read_entree(*current_dir, name, false, check_root_ea, check_user_ea);
        if(ref == NULL)
            return false; // no file of that name
        else
        {
            ref_dir = dynamic_cast<directory *>(ref);
            if(ref_dir != NULL)
            {
                filename_struct rfst;

                rfst.last_acc = ref_dir->get_last_access();
                rfst.last_mod = ref_dir->get_last_modif();
                filename_pile.push_back(rfst);
                *current_dir += ref_dir->get_name();
            }
            return true;
        }
    }

    void filesystem_diff::skip_read_filename_in_parent_dir()
    {
        if(filename_pile.size() > 0)
        {
            string tmp;
            set_back_dir_dates(current_dir->display(), filename_pile.back().last_acc, filename_pile.back().last_mod);
            filename_pile.pop_back();
            current_dir->pop(tmp);
        }
        else
            throw SRC_BUG;
    }

    void filesystem_diff::detruire()
    {
        if(fs_root != NULL)
            delete fs_root;
        if(current_dir != NULL)
            delete current_dir;
    }

    void filesystem_diff::copy_from(const filesystem_diff & ref)
    {
        if(ref.fs_root != NULL)
            fs_root = new path(*ref.fs_root);
        else
            fs_root = NULL;
        if(ref.current_dir != NULL)
            current_dir = new path(*ref.current_dir);
        else
            current_dir = NULL;
        info_details = ref.info_details;
        check_root_ea = ref.check_root_ea;
        check_user_ea = ref.check_user_ea;
        filename_pile = ref.filename_pile;
    }



///////////////////////////////////////////////////////////////////
////////////////// filesystem_hard_link_write methods  ////////////
///////////////////////////////////////////////////////////////////

    bool filesystem_hard_link_write::ea_has_been_restored(const hard_link *h)
    {
        if(h == NULL)
            throw SRC_BUG;
        map<infinint, corres_ino_ea>::iterator it = corres_write.find(h->get_etiquette());
        if(it == corres_write.end())
            return false;
        else
            return it->second.ea_restored;
    }

    bool filesystem_hard_link_write::set_ea(const nomme *e, const ea_attributs & l, path spot,
                                            bool allow_overwrite, bool warn_overwrite,
					    bool set_root_ea, bool set_user_ea,
                                            bool info_details)
    {
        const etiquette *e_eti = dynamic_cast<const etiquette *>(e);
        const directory *e_dir = dynamic_cast<const directory *>(e);
        bool ret = false;
        bool exists;

        try
        {
            if(e == NULL)
                throw SRC_BUG;
                // buidling the path to file
            if(e_dir == NULL) // not a directory (directory get a chdir in them so write_current_dir is up to date)
                spot += e->get_name();

                // checking that we have not already restored the EA of this
                // inode throw a hard link
            if(e_eti != NULL)
            {
                map<infinint, corres_ino_ea>::iterator it;

                it = corres_write.find(e_eti->get_etiquette());
                if(it == corres_write.end())
                {
                        // inode never restored; (no data saved just EA)
                        // we must record it
                    corres_ino_ea tmp;
                    tmp.chemin = spot.display();
                    tmp.ea_restored = true;
                    corres_write[e_eti->get_etiquette()] = tmp;
                }
                else
                    if(it->second.ea_restored)
                        return false; // inode already restored
                    else
                        it->second.ea_restored = true;
            }

            string chemin = spot.display();

                // restoring the root EA
                //
            exists = ea_filesystem_is_present(chemin, ea_domain_root);
            if(!exists || allow_overwrite)
            {
                if(set_root_ea)
                {
                    if(exists && warn_overwrite)
                        user_interaction_pause(string("system EA for ")+chemin+ " are about to be overwriten, continue ? ");
                    ea_filesystem_clear_ea(chemin, ea_domain_root);
                    if(ea_filesystem_write_ea(chemin, l, true, false))
                    {
                        if(info_details)
                            user_interaction_warning(string("restoring system EA for ")+chemin);
                        ret = true;
                    }
                    else
                        if(exists && l.size() == 0)
                            ret = true; // EA have changed, (no more EA)
                }
            }
            else
                if(set_root_ea)
                    user_interaction_warning(string("system EA for ")+chemin+" will not be restored, (overwriting not allowed)");

                // restoring the user EA
                //
            exists = ea_filesystem_is_present(chemin, ea_domain_user);
            if(!exists || allow_overwrite)
            {
                if(set_user_ea)
                {
                    if(exists && warn_overwrite)
                        user_interaction_pause(string("user EA for ")+chemin+ " are about to be overwriten, continue ? ");
                    ea_filesystem_clear_ea(chemin, ea_domain_user);
                    if(ea_filesystem_write_ea(chemin, l, false, true))
                    {
                        if(info_details)
                            user_interaction_warning(string("restoring user EA for ")+chemin);
                        ret = true;
                    }
                    else
                        if(exists && l.size() == 0)
                            ret = true; // EA have changed, (no more EA)
                }
            }
            else
                if(set_user_ea)
                    user_interaction_warning(string("user EA for ")+chemin+" will not be restored, (overwriting not allowed)");
        }
        catch(Euser_abort & e)
        {
            ret = false;
        }

        return ret;
    }

    void filesystem_hard_link_write::write_hard_linked_target_if_not_set(const etiquette *ref, const string & chemin)
    {
        if(!known_etiquette(ref->get_etiquette()))
        {
            corres_ino_ea tmp;
            tmp.chemin = chemin;
            tmp.ea_restored = false; // if EA have to be restored next
            corres_write[ref->get_etiquette()] = tmp;
        }
    }

    bool filesystem_hard_link_write::known_etiquette(const infinint & eti)
    {
        return corres_write.find(eti) != corres_write.end();
    }

    void filesystem_hard_link_write::make_file(const nomme * ref, const path & ou, bool dir_perm, bool ignore_owner)
    {
        const directory *ref_dir = dynamic_cast<const directory *>(ref);
        const file *ref_fil = dynamic_cast<const file *>(ref);
        const lien *ref_lie = dynamic_cast<const lien *>(ref);
        const blockdev *ref_blo = dynamic_cast<const blockdev *>(ref);
        const chardev *ref_cha = dynamic_cast<const chardev *>(ref);
        const tube *ref_tub = dynamic_cast<const tube *>(ref);
        const prise *ref_pri = dynamic_cast<const prise *>(ref);
        const etiquette *ref_eti = dynamic_cast <const etiquette *>(ref);
        const inode *ref_ino = dynamic_cast <const inode *>(ref);

        if(ref_ino == NULL && ref_eti == NULL)
            throw SRC_BUG; // neither an inode nor a hard link

        char *name = tools_str2charptr((ou + ref->get_name()).display());

        try
        {
            S_I ret;

            do
            {
                if(ref_eti != NULL) // we potentially have to make a hard link
                {
                    bool create_file = false;
                    map<infinint, corres_ino_ea>::iterator it = corres_write.find(ref_eti->get_etiquette());
                    if(it == corres_write.end()) // first time, we have to create the inode
                    {
                        corres_ino_ea tmp;
                        tmp.chemin = string(name);
                        tmp.ea_restored = false;
                        corres_write[ref_eti->get_etiquette()] = tmp;
                        create_file = true;
                    }
                    else // the inode already exists, making hard link if possible
                    {
                        char *old = tools_str2charptr(it->second.chemin);
                        try
                        {
                            ret = link(old, name);
                            if(ret < 0)
                            {
                                switch(errno)
                                {
                                case EXDEV:
                                case EPERM:
                                // can't make hard link, trying to duplicate the inode
                                    user_interaction_warning(string("error creating hard link ") + name + " : " + strerror(errno) + "\n Trying to duplicate the inode");
                                    create_file = true;
                                    clear_corres(ref_eti->get_etiquette());
                                        // need to remove this entry to be able
                                        // to restore EA for other copies
                                    break;
                                case ENOENT:
                                    if(ref_eti->get_inode()->get_saved_status() == s_saved)
                                    {
                                        create_file = true;
                                        clear_corres(ref_eti->get_etiquette());
                                            // need to remove this entry to be able
                                            // to restore EA for other copies
                                        user_interaction_warning(string("error creating hard link : ") + name + " , the inode to link with [" + old + "] has disapeared, re-creating it");

                                    }
                                    else
                                    {
                                        create_file = false; // nothing to do;
                                        user_interaction_warning(string("error creating hard link : ") + name + " , the inode to link with [" + old + "] is not present, cannot restore this hard link");
                                    }
                                    break;
                                default :
                                // nothing to do (ret < 0 and create_file == false)
                                    break;
                                }
                            }
                            else
                                create_file = false;
                        }
                        catch(...)
                        {
                            delete old;
                            throw;
                        }
                        delete old;
                    }

                    if(create_file)
                    {
                        file remplacant = file(*ref_eti->get_inode());

                        remplacant.change_name(ref->get_name());
                        make_file(&remplacant, ou, dir_perm, ignore_owner); // recursive call but with a plain file as argument
                        ref_ino = NULL; // to avoid setting the owner & permission twice (previous line, and below)
                        ret = 0; // to exist from while loop
                    }
                    else // hard link made
                        ret = 0; // not necessary, but avoids a warning from compilator (ret might be used uninitialized)
                }
                else if(ref_dir != NULL)
                {
                    ret = mkdir(name, 0777);
                }
                else if(ref_fil != NULL)
                {
                    generic_file *ou;
                    infinint seek;

                    ret = ::open(name, O_WRONLY|O_CREAT|O_BINARY, 0777);
                    if(ret >= 0)
                    {
                        fichier dest = ret;
                        ou = ref_fil->get_data();
                        try
                        {
                            crc crc_dyn, crc_ori;
                            ou->skip(0);
                            ou->copy_to(dest, crc_dyn);
                            if(ref_fil->get_crc(crc_ori))  // CRC is not present in format "01"
                                if(!same_crc(crc_dyn, crc_ori))
                                    throw Erange("filesystem_hard_link_write::make_file", "bad CRC, data corruption occured");
                        }
                        catch(...)
                        {
                            delete ou;
                            throw;
                        }
                        delete ou;
                    }
                }
                else if(ref_lie != NULL)
                {
                    char *cible = tools_str2charptr(ref_lie->get_target());
                    ret = symlink(cible ,name);
                    delete cible;
                }
                else if(ref_blo != NULL)
                    ret = mknod(name, S_IFBLK | 0777, makedev(ref_blo->get_major(), ref_blo->get_minor()));
                else if(ref_cha != NULL)
                    ret = mknod(name, S_IFCHR | 0777, makedev(ref_cha->get_major(), ref_cha->get_minor()));
                else if(ref_tub != NULL)
                    ret = mknod(name, S_IFIFO | 0777, 0);
                else if(ref_pri != NULL)
                {
                    ret = socket(PF_UNIX, SOCK_STREAM, 0);
                    if(ret >= 0)
                    {
                        S_I sd = ret;
                        struct sockaddr_un addr;
                        addr.sun_family = AF_UNIX;

                        try
                        {
                            strncpy(addr.sun_path, name, UNIX_PATH_MAX - 1);
                            addr.sun_path[UNIX_PATH_MAX - 1] = '\0';
                            if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                                throw Erange("filesystem_hard_link_write::make_file (socket bind)", string("Error creating Unix socket file: ") + name + " : " + strerror(errno));
                        }
                        catch(...)
                        {
                            shutdown(sd, 2);
                            close(sd);
                            throw;
                        }
                        shutdown(sd, 2);
                        close(sd);
                    }
                }
                else
                    throw SRC_BUG; // unknown inode type

                if(ret < 0)
                    if(errno != ENOSPC)
                        throw Erange("filesystem_hard_link_write::make_file", string("Could not create inode: ") + name + " : " + strerror(errno));
                    else
                        user_interaction_pause(string("Cannot create inode: ") + strerror(errno) + " Ready to continue ?");
            }
            while(ret < 0 && errno == ENOSPC);

            if(ref_ino != NULL && ret >= 0)
                make_owner_perm(*ref_ino, ou, dir_perm, ignore_owner);
        }
        catch(...)
        {
            delete name;
            throw;
        }
        delete name;
    }

    void filesystem_hard_link_write::clear_corres(const infinint & ligne)
    {
        map<infinint, corres_ino_ea>::iterator it = corres_write.find(ligne);
        if(it != corres_write.end())
            corres_write.erase(it);
    }


///////////////////////////////////////////////////////////////////
////////////////// filesystem_restore methods  ////////////////////
///////////////////////////////////////////////////////////////////


    filesystem_restore::filesystem_restore(const path &root, bool x_allow_overwrite,
                                           bool x_warn_overwrite, bool x_info_details,
                                           bool x_set_root_ea, bool x_set_user_ea, bool ignore_owner,
					   bool x_warn_remove_no_match, bool x_empty)
    {
	fs_root = get_root_with_symlink(root, x_info_details);
	if(fs_root == NULL)
	    throw Ememory("filesystem_write::filesystem_write");
	allow_overwrite = x_allow_overwrite;
	warn_overwrite = x_warn_overwrite;
	info_details = x_info_details;
	set_root_ea = x_set_root_ea;
	set_user_ea = x_set_user_ea;
	ignore_ownership = ignore_owner;
	warn_remove_no_match = x_warn_remove_no_match;
	current_dir = NULL;
	empty = x_empty;
	reset_write();
    }

    void filesystem_restore::reset_write()
    {
        filesystem_hard_link_write::corres_reset();
        filesystem_hard_link_read::corres_reset();
        stack_dir.clear();
        if(current_dir != NULL)
            delete current_dir;
        current_dir = new path(*fs_root);
        if(current_dir == NULL)
            throw Ememory("filesystem_write::reset_write");
    }

    bool filesystem_restore::write(const entree *x)
    {
        const eod *x_eod = dynamic_cast<const eod *>(x);
        const nomme *x_nom = dynamic_cast<const nomme *>(x);
	const directory *x_dir = dynamic_cast<const directory *>(x);

        bool ret = true;

        try
        {
            if(x_eod != NULL)
            {
                string tmp;
                current_dir->pop(tmp);
                if(stack_dir.size() > 0)
		{
		    if(!empty)
			make_owner_perm(stack_dir.back(), *current_dir, true, ignore_ownership);
		}
                else
                    throw SRC_BUG;
                stack_dir.pop_back();
            }
            else
                if(x_nom == NULL)
                    throw SRC_BUG; // neither "nomme" nor "eod"
                else // nomme
                {
                    path spot = *current_dir + x_nom->get_name();
                    const detruit *x_det = dynamic_cast<const detruit *>(x);
                    const inode *x_ino = dynamic_cast<const inode *>(x);
                    const etiquette *x_eti = dynamic_cast<const etiquette *>(x);

                    nomme *exists = make_read_entree(*current_dir, x_nom->get_name(), false, set_root_ea, set_user_ea);

                    try
                    {
                        if(x_ino == NULL && x_det == NULL && x_eti == NULL)
                            throw SRC_BUG; // should be either inode or detruit or hard link

                        if(x_det != NULL) // this is an object of class "detruit"
                        {
                            if(exists != NULL) // the file to destroy exists
                            {
                                if(!allow_overwrite)
                                    throw Erange("filesystem_write::write", spot.display() + " will not be remove from filesystem, overwriting not allowed");
                                if(warn_overwrite)
                                    user_interaction_pause(spot.display() + " is about to be removed from filesystem, continue ? ");

                                if(tolower(exists->signature()) == tolower(x_det->get_signature()))
				{
				    if(!empty)
					supprime(spot);
				}
                                else
                                {
				    if(warn_remove_no_match) // warning even if just allow_overwite is set (not espetially warn_overwrite)
					user_interaction_pause(spot.display() + " must be removed, but does not match expected type, remove it anyway ?");
				    if(!empty)
					supprime(spot);
                                }
                            }
                        }
                        else // hard_link or inode
                        {
                            if(exists == NULL) // nothing of this name in filesystem
			    {
				if(!empty)
				    make_file(x_nom, *current_dir, false, ignore_ownership);
			    }
                            else // an entry of this name exists in filesystem
                            {
                                const inode *exists_ino = dynamic_cast<inode *>(exists);

                                if((x_eti == NULL && x_ino == NULL) || exists_ino == NULL)
                                    throw SRC_BUG; // should be both of class inode (or hard_link for x)

                                if(allow_overwrite)
                                {
                                    if(warn_overwrite && x_dir == NULL)
                                        user_interaction_pause(spot.display() + " is about to be overwritten, OK ?");
                                    if(x_dir != NULL && x_ino->same_as(*exists_ino))
				    {
					if(!empty)
					    make_owner_perm(*x_ino, *current_dir, false, ignore_ownership);
				    }
                                    else
                                    {
                                        ea_attributs ea; // saving original EA of existing inode
                                        bool got_it = true;
                                        try
                                        {
                                            ea_filesystem_read_ea(spot.display(), ea, true, true);
                                        }
                                        catch(Egeneric & ex)
                                        {
                                            got_it = false;
                                            user_interaction_warning(string("existing EA for ") + spot.display() + " could not be read and preserved : " + ex.get_message());
                                        }
					if(!empty)
					{
					    supprime(spot); // this destroyes EA
					    make_file(x_nom, *current_dir, false, ignore_ownership);
					}
                                        try // if possible and available restoring original EA
                                        {
                                            if(got_it && !empty)
                                                (void)ea_filesystem_write_ea(spot.display(), ea, true, true);
                                                // we don't care about the return value, here
                                        }
                                        catch(Egeneric & e)
                                        {
                                            if(ea.size() >0)
                                                user_interaction_warning(string("existing EA for ")+ spot.display() + " could not be preserved : " + e.get_message());
                                        }
                                    }
                                }
                                else // overwriting not allowed
                                {
                                    if(x_dir != NULL && !x_ino->same_as(*exists_ino))
                                        throw Erange("filesystem_write::write", string("Directory ")+spot.display()+" cannot be restored: overwriting not allowed and a non-directory inode of that name exists");
                                    else
                                        if(info_details)
                                            user_interaction_warning(spot.display() + " has not been overwritten (action not allowed)");
                                    ret = false;
                                }
                            }

                            if(x_dir != NULL)
                            {
                                *current_dir += x_dir->get_name();
                                stack_dir.push_back(directory(*x_dir));
                            }
                        }
                    }
                    catch(...)
                    {
                        if(exists != NULL)
                            delete exists;
                        throw;
                    }
                    if(exists != NULL)
                        delete exists;
                }
        }
        catch(Euser_abort & e)
        {
	    if(x_dir != NULL) // we must keep directory memory even when the directory could not be written due to lack of disk space
	    {
		*current_dir += x_dir->get_name();
		stack_dir.push_back(directory(*x_dir));
	    }

            ret = false;
        }

        return ret;
    }

    nomme *filesystem_restore::get_before_write(const nomme *x)
    {
        return make_read_entree(*current_dir, x->get_name(), false, set_root_ea, set_user_ea);
    }

    void filesystem_restore::pseudo_write(const directory *dir)
    {
        if(dir == NULL)
            throw SRC_BUG;

        path spot = *current_dir + dir->get_name();
        nomme *exists = make_read_entree(*current_dir, dir->get_name(), false, set_root_ea, set_user_ea);

        try
        {
            if(exists == NULL)
	    {
		if(!empty)
		    make_file(dir, *current_dir, false, ignore_ownership);  // need to create the directory to be able to restore any file in it
	    }
            else
            {
                const directory *e_dir = dynamic_cast<const directory *>(exists);

                if(e_dir == NULL) // an inode of that name exists, but it is not a directory
                {
                    if(allow_overwrite)
                    {
                        if(warn_overwrite)
                            user_interaction_pause(spot.display() + " is about to be removed and replaced by a directory, OK ?");
			if(!empty)
			{
			    supprime(spot);
			    make_file(dir, *current_dir, false, ignore_ownership);
			}
                    }
                    else
                        throw Erange("filesystem_restore::pseudo_write",
                                     spot.display() +
                                     " could not be restored, because a file of that name exists and overwrite is not allowed");
                }
                else // just setting permission to allow creation of any sub-dir or sub_file
                {
                    char *name = tools_str2charptr(spot.display());
                    try
                    {
			if(!empty)
			    if(chmod(name, 0777) < 0)
				user_interaction_warning(string("Cannot restore permissions of ") + spot.display() + " : " + strerror(errno));
                    }
                    catch(...)
                    {
                        delete name;
                        throw;
                    }
                    delete name;
                }
            }
        }
        catch(...)
        {
            if(exists != NULL)
                delete exists;
            throw;
        }
        if(exists != NULL)
            delete exists;

        *current_dir += dir->get_name();
        stack_dir.push_back(directory(*dir));
    }

    void filesystem_restore::detruire()
    {
        if(fs_root != NULL)
            delete fs_root;
        if(current_dir != NULL)
            delete current_dir;
    }

    void filesystem_restore::copy_from(const filesystem_restore & ref)
    {
        if(ref.fs_root != NULL)
            fs_root = new path(*ref.fs_root);
        else
            fs_root = NULL;
        if(ref.current_dir != NULL)
            current_dir = new path(*ref.current_dir);
        else
            current_dir = NULL;
        info_details = ref.info_details;
        set_root_ea = ref.set_root_ea;
        set_user_ea = ref.set_user_ea;
        allow_overwrite = ref.allow_overwrite;
        warn_overwrite = ref.warn_overwrite;
        ignore_ownership = ref.ignore_ownership;
	warn_remove_no_match = ref.warn_remove_no_match;
        stack_dir = ref.stack_dir;
	empty = ref.empty;
    }



///////////////////////////////////////////////////////////////////
////////////////// static functions ///////////////////////////////
///////////////////////////////////////////////////////////////////

    static void supprime(const path & ref)
    {
        char *s = tools_str2charptr(ref.display());

        try
        {
            struct stat buf;
            if(lstat(s, &buf) < 0)
                throw Erange("filesystem supprime", string("Cannot get inode information about file to remove ") + s + " : " + strerror(errno));

            if(S_ISDIR(buf.st_mode))
            {
                etage fils = s;
                string tmp;

                while(fils.read(tmp))
                    supprime(ref+tmp);

                if(rmdir(s) < 0)
                    throw Erange("supprime (dir)", string("Cannot remove directory ") + s + " : " + strerror(errno));
            }
            else
                if(unlink(s) < 0)
                    throw Erange("supprime (file)", string("Cannot remove file ") + s + " : " + strerror(errno));
        }
        catch(...)
        {
            delete s;
            throw;
        }

        delete s;
    }

    static void dummy_call(char *x)
    {
        static char id[]="$Id: filesystem.cpp,v 1.17.2.5 2004/07/13 22:37:33 edrusb Rel $";
        dummy_call(id);
    }

    static void make_owner_perm(const inode & ref, const path & ou, bool dir_perm, bool ignore_ownership)
    {
        string chem = (ou + ref.get_name()).display();
        char *name = tools_str2charptr(chem);
        const lien *ref_lie = dynamic_cast<const lien *>(&ref);
        S_I permission;

        if(dynamic_cast<const directory *>(&ref) != NULL && !dir_perm)
            permission = 0777;
        else
            permission = ref.get_perm();

        try
        {
            if(!ignore_ownership)
                if(ref.get_saved_status() == s_saved)
		{
#if HAVE_LCHOWN
                    if(lchown(name, ref.get_uid(), ref.get_gid()) < 0)
                        user_interaction_warning(string(name) + string("Could not restore original file ownership: ") + strerror(errno));
#else
                    if(dynamic_cast<const lien *>(&ref) == NULL) // not a symbolic link
			if(chown(name, ref.get_uid(), ref.get_gid()) < 0)
			    user_interaction_warning(string(name) + string("Could not restore original file ownership: ") + strerror(errno));
			//
			// we don't/can't restore ownership for symbolic links (no system call to do that)
			//
#endif
		}
            try
            {
                if(ref_lie == NULL) // not restoring permission for symbolic links
                    if(chmod(name, permission) < 0)
			user_interaction_warning(string("Cannot restore permissions of ") + name + " : " + strerror(errno));
            }
            catch(Egeneric &e)
            {
                if(ref_lie == NULL)
                    throw;
                    // else (the inode is a symlink), we simply ignore this error
            }

            if(ref_lie == NULL) // not restoring atime & ctime for symbolic links
                make_date(chem, ref.get_last_access(), ref.get_last_modif());
        }
        catch(...)
        {
            delete name;
            throw;
        }
        delete name;
    }


    static void make_date(const string & chemin, infinint access, infinint modif)
    {
        struct utimbuf temps;
        time_t tmp = 0;
        char *filename;

        access.unstack(tmp);
        temps.actime = tmp;
        tmp = 0;
        modif.unstack(tmp);
        temps.modtime = tmp;

        filename = tools_str2charptr(chemin);
        try
        {
            if(utime(filename , &temps) < 0)
                Erange("make_date", string("Cannot set last access and last modification time: ") + strerror(errno));
        }
        catch(...)
        {
            delete filename;
            throw;
        }
        delete filename;
    }

    static void attach_ea(const string &chemin, inode *ino, bool ea_root_mode, bool ea_user_mode)
    {
        ea_attributs *eat = new ea_attributs();
        if(eat == NULL)
            throw Ememory("filesystem : attach_ea");
        try
        {
            if(ino == NULL)
                throw SRC_BUG;
            ea_filesystem_read_ea(chemin, *eat, ea_root_mode, ea_user_mode);
            if(eat->size() > 0)
            {
                ino->ea_set_saved_status(inode::ea_full);
                ino->ea_attach(eat);
                eat = NULL;
                    // allocated memory now managed by the inode object
            }
            else
                ino->ea_set_saved_status(inode::ea_none);
        }
        catch(...)
        {
            if(eat != NULL)
                delete eat;
            throw;
        }
        if(eat != NULL)
            delete eat;
    }

    static void set_back_dir_dates(const string & chem, const infinint & last_acc, const infinint & last_mod)
    {
        try
        {
            if(last_acc != 0 || last_mod != 0)
                make_date(chem, last_acc, last_mod);
                // else the directory could not be openned properly
                // and time could not be retrieved, so we don't try
                // to restore them
        }
        catch(Erange & e)
        {
                // cannot restore dates, ignoring
        }
    }

    static bool is_nodump_flag_set(const path & chem, const string & filename)
    {
#ifdef LIBDAR_NODUMP_FEATURE
        S_I fd, f = 0;
        char *ptr = tools_str2charptr((chem + filename).display());

        try
        {
            fd = ::open(ptr, O_RDONLY|O_BINARY|O_NONBLOCK);
            if(fd < 0)
		user_interaction_warning(string("Failed to open ") + filename +" while checking for nodump flag: " + strerror(errno));
	    else
	    {
		try
		{
		    if(ioctl(fd, EXT2_IOC_GETFLAGS, &f) < 0)
		    {
			user_interaction_warning(string("Cannot get ext2 attributes (and nodump flag value) for ") + filename + " : " + strerror(errno));
			f = 0;
		    }
		}
		catch(...)
		{
		    close(fd);
		    throw;
		}
		close(fd);
	    }
        }
        catch(...)
        {
            delete ptr;
            throw;
        }
        delete ptr;

        return (f & EXT2_NODUMP_FL) != 0;
#else
        return false;
#endif
    }

    static path *get_root_with_symlink(const path & root, bool info_details)
    {
	path *ret = NULL;
        char *ptr = tools_str2charptr(root.display());
	try
	{
	    struct stat buf;
            if(lstat(ptr, &buf) < 0) // stat not lstat, thus we eventually get the symlink pointed to inode
                throw Erange("filesystem:get_root_with_symlink", string("Cannot get inode information for ") + root.display() + " : " + strerror(errno));

	    if(S_ISDIR(buf.st_mode))
	    {
		ret = new path(root);
		if(ret == NULL)
		    throw  Ememory("get_root_with_symlink");
	    }
            else if(S_ISLNK(buf.st_mode))
	    {
		ret = new path(tools_readlink(ptr));
		if(ret == NULL)
		    throw Ememory("get_root_with_symlink");
		if(ret->is_relative())
		{
		    string tmp;
		    path base = root;

		    if(base.pop(tmp))
			*ret = base + *ret;
		    else
			if(!root.is_relative())
			    throw SRC_BUG;
			// symlink name is not "popable" and is absolute, it is thus the filesystem root '/'
			// and it is a symbolic link !!! How is it possible that "/" be a symlink ?
			// a symlink to where ???
		}
		if(info_details && ! (*ret == root) )
		    user_interaction_warning(string("Replacing in the -R option ") + root.display() + " by the directory pointed to by this symbolic link: " + ret->display());
	    }
	    else // not a directory given as argument
		throw Erange("filesystem:get_root_with_symlink", string("Given path ") + root.display() + " must be a directory (or symbolic link to an existing directory");
	}
	catch(...)
	{
	    delete ptr;
	    throw;
	}
	delete ptr;
	if(ret == NULL)
	    throw SRC_BUG; // exit without exception, but ret not allocated !

	return ret;
    }

} // end of namespace