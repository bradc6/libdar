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
// $Id: tools.cpp,v 1.21.2.2 2004/01/15 14:06:28 edrusb Rel $
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

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#ifndef WIFSTOPPED
#define WIFSTOPPED(status)    (((status) & 0xff) == 0x7f)
#endif
#ifndef WIFSIGNALED
# define WIFSIGNALED(status)  (!WIFSTOPPED(status) && !WIFEXITED(status))
#endif
#ifndef WTERMSIG
#define WTERMSIG(status)      ((status) & 0x7f)
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_PWD_H
#include <pwd.h>
#endif

#if HAVE_GRP_H
#include <grp.h>
#endif
} // end extern "C"

#include <iostream>
#include "tools.hpp"
#include "erreurs.hpp"
#include "deci.hpp"
#include "user_interaction.hpp"
#include "integers.hpp"
#include "mask.hpp"
#include "etage.hpp"

using namespace std;

namespace libdar
{

    static void runson(char *argv[]);
    static void deadson(S_I sig);
    static bool is_a_slice_available(const string & base, const string & extension);
    static string retreive_basename(const string & base, const string & extension);

    char *tools_str2charptr(string x)
    {
        U_I size = x.size();
        char *ret = new char[size+1];

        if(ret == NULL)
            throw Ememory("tools_str2charptr");
        for(register unsigned int i = 0; i < size; i++)
            ret[i] = x[i];
        ret[size] = '\0';

        return ret;
    }

    void tools_write_string(generic_file & f, const string & s)
    {
        tools_write_string_all(f, s);
        f.write("", 1); // adding a '\0' at the end;
    }

    void tools_read_string(generic_file & f, string & s)
    {
        char a[2] = { 0, 0 };
        S_I lu;

        s = "";
        do
        {
            lu = f.read(a, 1);
            if(lu == 1  && a[0] != '\0')
                s += a;
        }
        while(lu == 1 && a[0] != '\0');

        if(lu != 1 || a[0] != '\0')
            throw Erange("tools_read_string", "not a zero terminated string in file");
    }

    void tools_write_string_all(generic_file & f, const string & s)
    {
        char *tmp = tools_str2charptr(s);

        if(tmp == NULL)
            throw Ememory("tools_write_string_all");
        try
        {
            U_I len = s.size();
            U_I wrote = 0;

            while(wrote < len)
                wrote += f.write(tmp+wrote, len-wrote);
        }
        catch(...)
        {
            delete tmp;
        }
        delete tmp;
    }

    void tools_read_string_size(generic_file & f, string & s, infinint taille)
    {
        U_16 small_read = 0;
        size_t max_read = 0;
        S_I lu = 0;
        const U_I buf_size = 10240;
        char buffer[buf_size];

        s = "";
        do
        {
            if(small_read > 0)
            {
                max_read = small_read > buf_size ? buf_size : small_read;
                lu = f.read(buffer, max_read);
                small_read -= lu;
                s += string((char *)buffer, (char *)buffer+lu);
            }
            taille.unstack(small_read);
        }
        while(small_read > 0);
    }

    infinint tools_get_filesize(const path &p)
    {
        struct stat buf;
        char *name = tools_str2charptr(p.display());

        if(name == NULL)
            throw Ememory("tools_get_filesize");

        try
        {
            if(lstat(name, &buf) < 0)
                throw Erange("tools_get_filesize", strerror(errno));
        }
        catch(...)
        {
            delete name;
        }

        delete name;
        return (U_32)buf.st_size;
    }

    infinint tools_get_extended_size(string s)
    {
        U_I len = s.size();
        infinint factor = 1;

        if(len < 1)
            return false;
        switch(s[len-1])
        {
        case 'K':
        case 'k': // kilobyte
            factor = 1024;
            break;
        case 'M': // megabyte
            factor = infinint(1024).power((U_I)2);
            break;
        case 'G': // gigabyte
            factor = infinint(1024).power((U_I)3);
            break;
        case 'T': // terabyte
            factor = infinint(1024).power((U_I)4);
            break;
        case 'P': // petabyte
            factor = infinint(1024).power((U_I)5);
            break;
        case 'E': // exabyte
            factor = infinint(1024).power((U_I)6);
            break;
        case 'Z': // zettabyte
            factor = infinint(1024).power((U_I)7);
            break;
        case 'Y':  // yottabyte
            factor = infinint(1024).power((U_I)8);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;
        default :
            throw Erange("command_line get_extended_size", string("unknown sufix in string : ")+s);
        }

        if(factor != 1)
            s = string(s.begin(), s.end()-1);

        deci tmp = s;
        factor *= tmp.computer();

        return factor;
    }

    char *tools_extract_basename(const char *command_name)
    {
        path commande = command_name;
        string tmp = commande.basename();
        char *name = tools_str2charptr(tmp);

        return name;
    }


    static void dummy_call(char *x)
    {
        static char id[]="$Id: tools.cpp,v 1.21.2.2 2004/01/15 14:06:28 edrusb Rel $";
        dummy_call(id);
    }

    void tools_split_path_basename(const char *all, path * &chemin, string & base)
    {
        chemin = new path(all);
        if(chemin == NULL)
            throw Ememory("tools_split_path_basename");

        try
        {
            if(chemin->degre() > 1)
            {
                if(!chemin->pop(base))
                    throw SRC_BUG; // a path of degree > 1 should be able to pop()
            }
            else
            {
                base = chemin->basename();
                delete chemin;
                chemin = new path(".");
                if(chemin == NULL)
                    throw Ememory("tools_split_path_basename");
            }
        }
        catch(...)
        {
            if(chemin != NULL)
                delete chemin;
            throw;
        }
    }

    void tools_split_path_basename(const string & all, string & chemin, string & base)
    {
        path *tmp = NULL;
        char *ptr = tools_str2charptr(all);

        try
        {
            tools_split_path_basename(ptr, tmp, base);
            if(tmp == NULL)
                throw SRC_BUG;
            chemin = tmp->display();
            delete tmp;
        }
        catch(...)
        {
            delete ptr;
            throw;
        }
        delete ptr;
    }

    void tools_open_pipes(const string &input, const string & output, tuyau *&in, tuyau *&out)
    {
        in = out = NULL;
        try
        {
            if(input != "")
                in = new tuyau(input, gf_read_only);
            else
                in = new tuyau(0, gf_read_only); // stdin by default
            if(in == NULL)
                throw Ememory("tools_open_pipes");

            if(output != "")
                out = new tuyau(output, gf_write_only);
            else
                out = new tuyau(1, gf_write_only); // stdout by default
            if(out == NULL)
                throw Ememory("tools_open_pipes");

        }
        catch(...)
        {
            if(in != NULL)
                delete in;
            if(out != NULL)
                delete out;
            throw;
        }
    }

    void tools_blocking_read(S_I fd, bool mode)
    {
        S_I flags = fcntl(fd, F_GETFL, 0);
        if(!mode)
            flags |= O_NDELAY;
        else
            flags &= ~O_NDELAY;
        fcntl(fd, F_SETFL, flags);
    }

    string tools_name_of_uid(U_16 uid)
    {
        struct passwd *pwd = getpwuid(uid);

        if(pwd == NULL) // uid not associated with a name
        {
            infinint tmp = uid;
            deci d = tmp;
            return d.human();
        }
        else
            return pwd->pw_name;
    }

    string tools_name_of_gid(U_16 gid)
    {
        struct group *gr = getgrgid(gid);

        if(gr == NULL) // uid not associated with a name
        {
            infinint tmp = gid;
            deci d = tmp;
            return d.human();
        }
        else
            return gr->gr_name;
    }

    string tools_int2str(S_I x)
    {
        infinint tmp = x >= 0 ? x : -x;
        deci d = tmp;

        return (x >= 0 ? string("") : string("-")) + d.human();
    }

    U_32 tools_str2int(const string & x)
    {
        deci d = x;
        infinint t = d.computer();
        U_32 ret = 0;

        t.unstack(ret);
        if(t != 0)
            throw Erange("tools_str2int", "cannot convert the string to integer, overflow");

        return ret;
    }

    string tools_addspacebefore(string s, U_I expected_size)
    {
        while(s.size() < expected_size)
            s = string(" ") + s;

        return s;
    }

    string tools_display_date(infinint date)
    {
        time_t pas = 0;
        string ret;

        date.unstack(pas);
        ret = ctime(&pas);

        return string(ret.begin(), ret.end() - 1); // -1 to remove the ending '\n'
    }

    void tools_system(const vector<string> & argvector)
    {
        if(argvector.size() == 0)
            return; // nothing to do
        else
        {
            char **argv = new char *[argvector.size()+1];

            if(argv == NULL)
                throw Ememory("tools_system");
            try
            {
                    // make an (S_I, char *[]) couple
                for(register U_I i = 0; i <= argvector.size(); i++)
                    argv[i] = NULL;

                try
                {
                    S_I status;
                    bool loop;

                    for(register U_I i = 0; i < argvector.size(); i++)
                        argv[i] = tools_str2charptr(argvector[i]);

                    do
                    {
                        deadson(0);
                        loop = false;
                        S_I pid = fork();

                        switch(pid)
                        {
                        case -1:
                            throw Erange("tools_system", string("Error while calling fork() to launch dar: ") + strerror(errno));
                        case 0: // fork has succeeded, we are the child process
                            runson(argv);
                                // function that never returns
                        default:
                            if(wait(&status) <= 0)
                                throw Erange("tools_system",
                                             string("Unexpected error while waiting for dar to terminate: ") + strerror(errno));
                            else // checking the way dar has exit
                                if(!WIFEXITED(status)) // not a normal ending
                                    if(WIFSIGNALED(status)) // exited because of a signal
                                    {
                                        try
                                        {
                                            user_interaction_pause(string("DAR terminated upon signal reception: ")
#if HAVE_DECL_SYS_SIGLIST
                                                                   + (WTERMSIG(status) < NSIG ? sys_siglist[WTERMSIG(status)] : tools_int2str(WTERMSIG(status)))
#else
                                                                   + tools_int2str(WTERMSIG(status))
#endif
                                                                   + " . Retry to launch dar as previously ?");
                                            loop = true;
                                        }
                                        catch(Euser_abort & e)
                                        {
                                            user_interaction_pause("Continue anyway ?");
                                        }
                                    }
                                    else // normal terminaison but exit code not zero
                                        user_interaction_pause(string("DAR sub-process has terminated with exit code ")
                                                               + tools_int2str(WEXITSTATUS(status))
                                                               + " Continue anyway ?");
                        }
                    }
                    while(loop);
                }
                catch(...)
                {
                    for(register U_I i = 0; i < argvector.size(); i++)
                        if(argv[i] != NULL)
                            delete argv[i];
                    throw;
                }
                for(register U_I i = 0; i < argvector.size(); i++)
                    if(argv[i] != NULL)
                        delete argv[i];
            }
            catch(...)
            {
                delete argv;
                throw;
            }
            delete argv;
        }
    }

    void tools_write_vector(generic_file & f, const vector<string> & x)
    {
        infinint tmp = x.size();
        vector<string>::iterator it = const_cast<vector<string> *>(&x)->begin();
        vector<string>::iterator fin = const_cast<vector<string> *>(&x)->end();

        tmp.dump(f);
        while(it != fin)
            tools_write_string(f, *it++);
    }

    void tools_read_vector(generic_file & f, vector<string> & x)
    {
        infinint tmp = infinint(NULL, &f);
        string elem;

        x.clear();
        while(tmp > 0)
        {
            tools_read_string(f, elem);
            x.push_back(elem);
            tmp--;
        }
    }

    string tools_concat_vector(const string & separator, const vector<string> & x)
    {
        string ret = separator;
        vector<string>::iterator it = const_cast<vector<string> *>(&x)->begin();
        vector<string>::iterator fin = const_cast<vector<string> *>(&x)->end();

        while(it != fin)
            ret += *it++ + separator;

        return ret;
    }

    vector<string> operator + (vector<string> a, vector<string> b)
    {
        vector<string>::iterator it = b.begin();

        while(it != b.end())
            a.push_back(*it++);

        return a;
    }


    static void deadson(S_I sig)
    {
        signal(SIGCHLD, &deadson);
    }

    static void runson(char *argv[])
    {
        if(execvp(argv[0], argv) < 0)
            user_interaction_warning(string("Error while calling execvp:") + strerror(errno));
        else
            user_interaction_warning("execvp failed but did not returned error code");
        exit(0);
    }

    const char *tools_get_from_env(const char **env, char *clef)
    {
        unsigned int index = 0;
        const char *ret = NULL;

        if(env == NULL || clef == NULL)
            return NULL;

        while(ret == NULL && env[index] != NULL)
        {
            unsigned int letter = 0;
            while(clef[letter] != '\0'
                  && env[index][letter] != '\0'
                  && env[index][letter] != '='
                  && clef[letter] == env[index][letter])
                letter++;
            if(clef[letter] == '\0' && env[index][letter] == '=')
                ret = env[index]+letter+1;
            else
                index++;
        }

        return ret;
    }


    bool tools_is_member(const string & val, const vector<string> & liste)
    {
        U_I index = 0;

        while(index < liste.size() && liste[index] != val)
            index++;

        return index <liste.size();
    }

    void tools_display_features(bool ea, bool largefile, bool nodump, bool special_alloc, U_I bits)
    {
        ui_printf("   Extended Attributes support: %s\n", (ea ? "YES" : "NO"));
        ui_printf("   Large files support (> 2GB): %s\n", (largefile ? "YES" : "NO"));
        ui_printf("   ext2fs NODUMP flag support : %s\n", (nodump ? "YES" : "NO"));
        ui_printf("   Special allocation scheme  : %s\n", (special_alloc ? "YES" : "NO"));
        if(bits == 0)
            ui_printf("   Integer size used          : unlimited\n");
        else
            ui_printf("   Integer size used          : %d bits\n", bits);
    }

    bool is_equal_with_hourshift(const infinint & hourshift, const infinint & date1, const infinint & date2)
    {
        infinint delta = date1 > date2 ? date1-date2 : date2-date1;
        infinint num, rest;

            // delta = num*3600 + rest
            // with 0 <= rest < 3600
            // (this euclidian division)
        euclide(delta, 3600, num, rest);

        if(rest != 0)
            return false;
        else // rest == 0
            return num <= hourshift;
    }

    bool tools_my_atoi(char *a, U_I & val)
    {
        infinint tmp;
        bool ret = true;

        try
        {
            tmp = deci(a).computer();
            val = 0;
            tmp.unstack(val);
        }
        catch(Edeci & e)
        {
            ret = false;
        }

        return ret;
    }

    void tools_check_basename(const path & loc, string & base, const string & extension)
    {
        regular_mask suspect = string(".\\.[1-9][0-9]*\\.")+extension;
        string old_path = (loc+base).display();

            // is basename is suspect ?
        if(!suspect.is_covered(base))
            return; // not a suspect basename

            // is there a slice available ?
        if(is_a_slice_available(old_path, extension))
            return; // yes, thus basename is not a mistake

            // removing the suspicious end (.<number>.extension)
            // and checking the avaibility of such a slice

        string new_base = retreive_basename(base, extension);
        string new_path = (loc+new_base).display();
        if(is_a_slice_available(new_path, extension))
        {
            try
            {
                user_interaction_pause(string("Warning, ") + base + " seems more to be a slice name than a base name. Do you want to replace it by " + new_base + " ?");
                base = new_base;
            }
            catch(Euser_abort & e)
            {
                user_interaction_warning(string("OK, keeping ") + base + " as basename");
            }
        }
    }

    string tools_getcwd()
    {
	size_t length = 10240;
	char *buffer = NULL, *ret;
	string cwd;
	try
	{
	    do
	    {
		buffer = new char[length];
		if(buffer == NULL)
		    throw Ememory("tools_getcwd()");
		ret = getcwd(buffer, length-1); // length-1 to keep a place for ending '\0'
		if(ret == NULL) // could not get the CWD
		    if(errno == ERANGE) // buffer too small
		    {
			delete buffer;
			buffer = NULL;
			length *= 2;
		    }
		    else // other error
			throw Erange("tools_getcwd", string("Cannot get full path of current working directory: ") + strerror(errno));
	    }
	    while(ret == NULL);

	    buffer[length - 1] = '\0';
	    cwd = buffer;
	}
	catch(...)
	{
	    if(buffer != NULL)
		delete buffer;
	    throw;
	}
	if(buffer != NULL)
	    delete buffer;
	return cwd;
    }

    string tools_readlink(const char *root)
    {
	size_t length = 10240;
	char *buffer = NULL;
	S_I lu;
	string ret = "";

	if(root == NULL)
	    throw Erange("tools_readlink", "NULL argument given to tools_readlink");
	if(strcmp(root, "") == 0)
	    throw Erange("tools_readlink", "Empty string given as argument to tools_readlink");

	try
	{
	    do
	    {
		buffer = new char[length];
		if(buffer == NULL)
		    throw Ememory("tools_readlink");
		lu = readlink(root, buffer, length-1); // length-1 to have room to add '\0' at the end

		if(lu < 0) // error occured with readlink
		{
		    switch(errno)
		    {
		    case EINVAL: // not a symbolic link (thus we return the given argument)
			ret = root;
			break;
		    case ENAMETOOLONG: // too small buffer
			delete buffer;
			buffer = NULL;
			length *= 2;
			break;
		    default: // other error
			throw Erange("get_readlink", string("Cannot read file information for ")+ root + " : " + strerror(errno));
		    }
		}
		else // got the correct real path of symlink
		    if((U_I)(lu) < length)
		    {
			buffer[lu] = '\0';
			ret = buffer;
		    }
		    else // "lu" should not be greater than length: readlink system call error
		    {
			    // trying to workaround with a larger buffer
			delete buffer;
			buffer = NULL;
			length *= 2;
		    }
	    }
	    while(ret == "");
	}
	catch(...)
	{
	    if(buffer != NULL)
		delete buffer;
	    throw;
	}
	if(buffer != NULL)
	    delete buffer;
	return ret;
    }

    static bool is_a_slice_available(const string & base, const string & extension)
    {
        char *name = tools_str2charptr(base);
        path *chem = NULL;
        bool ret = false;

        try
        {
            char *char_chem = NULL;
            string rest;

            tools_split_path_basename(name, chem, rest);
            char_chem = tools_str2charptr(chem->display());

            try
            {
                etage contents = char_chem;
                regular_mask slice = rest + "\\.[1-9][0-9]*\\."+ extension;

                while(!ret && contents.read(rest))
                    ret = slice.is_covered(rest);
            }
            catch(Erange & e)
            {
                ret = false;
            }
            catch(...)
            {
                delete char_chem;
                throw;
            }
            delete char_chem;
        }
        catch(...)
        {
            delete name;
            if(chem != NULL)
                delete chem;
            throw;
        }
        delete name;
        if(chem != NULL)
            delete chem;

        return ret;
    }


    static string retreive_basename(const string & base, const string & extension)
    {
        string new_base = base;
        S_I index;

        if(new_base.size() < 2+1+extension.size())
            throw SRC_BUG; // must be a slice filename
        index = new_base.find_last_not_of(string(".")+extension);
        new_base = string(new_base.begin(), new_base.begin()+index);
        index = new_base.find_last_not_of("0123456789");
        new_base = string(new_base.begin(), new_base.begin()+index);

        return new_base;
    }


} // end of namespace