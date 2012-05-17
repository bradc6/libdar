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
// $Id: real_infinint.cpp,v 1.19.2.2 2007/07/22 16:35:00 edrusb Rel $
//
/*********************************************************************/

#include "../my_config.h"

extern "C"
{
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
} // end extern "C"

#include "real_infinint.hpp"
#include "erreurs.hpp"
#include "generic_file.hpp"
#include "tools.hpp"

namespace libdar
{

    infinint::endian infinint::used_endian = not_initialized;

    infinint::infinint(user_interaction & dialog, S_I *fd, generic_file *x)
    {
        if(fd != NULL && x != NULL)
            throw Erange("infinint::infinint(file, file)", gettext("Both arguments are not NULL, please choose one or the other, not both"));
        if(fd != NULL)
        {
            fichier f = fichier(dialog, dup(*fd));
            build_from_file(f);
        }
        else
            if(x != NULL)
                build_from_file(*x);
            else
                throw Erange("infinint::infinint(file, file)", gettext("Cannot read from file, both arguments are NULL"));
    }

    void infinint::dump(user_interaction & dialog, S_I fd) const
    {
        fichier f = fichier(dialog, dup(fd));
        dump(f);
    }

    void infinint::build_from_file(generic_file & x)
    {
        E_BEGIN;
        unsigned char a;
        bool fin = false;
        infinint skip = 0;
        storage::iterator it;
        S_I lu;
        int_tools_bitfield bf;

        while(!fin)
        {
            lu = x.read((char *)&a, 1);

            if(lu <= 0)
                throw Erange("infinint::build_from_file(generic_file)", gettext("Reached end of file before all data could be read"));

            if(a == 0)
                ++skip;
            else // end of size field
            {
                    // computing the size to read
                U_I pos = 0;

                int_tools_expand_byte(a, bf);
                for(S_I i = 0; i < 8; ++i)
                    pos += bf[i];
                if(pos != 1)
                    throw Erange("infinint::build_from_file(generic_file)", gettext("Badly formed infinint or not supported format")); // more than 1 bit is set to 1

                pos = 0;
                while(bf[pos] == 0)
                    ++pos;
                pos += 1; // bf starts at zero, but bit zero means 1 TG of length

                skip *= 8;
                skip += pos;
                skip *= TG;

                try
                {
                    field = new storage(x, skip);
                }
                catch(...)
                {
                    field = NULL;
                    throw;
                }

                if(field != NULL)
                {
                    it = field->begin();
                    fin = true;
                }
                else
                    throw Ememory("infinint::build_from_file(generic_file)");
            }
        }
        reduce(); // necessary to reduce due to TG storage
        E_END("infinint::build_from_file", "generic_file");
    }


    void infinint::dump(generic_file & x) const
    {
        E_BEGIN;
        infinint width;
        infinint pos;
        unsigned char last_width;
        infinint justification;
        U_32 tmp;

        if(! is_valid())
            throw SRC_BUG;

        width = field->size();    // this is the informational field size in byte
            // TG is the width in TG, thus the number of bit that must have
            // the preamble
        euclide(width, TG, width, justification);
        if(justification != 0)
                // in case we need to add some bytes to have a width multiple of TG
            ++width;  // we need then one more group to have a width multiple of TG

        euclide(width, 8, width, pos);
        if(pos == 0)
        {
            --width; // division is exact, only last bit of the preambule is set
            last_width = 0x80 >> 7;
                // as we add the last byte separately width gets shorter by 1 byte
        }
        else // division non exact, the last_width (last byte), make the rounding
        {
            U_16 pos_s = 0;
            pos.unstack(pos_s);
            last_width = 0x80 >> (pos_s - 1);
        }

            // now we write the preamble except the last byte. All theses are zeros.

        tmp = 0;
        unsigned char u = 0x00;
        width.unstack(tmp);
        do
        {
            while(tmp-- > 0)
                if(x.write((char *)(&u), 1) < 1)
                    throw Erange("infinint::dump(generic_file)", gettext("Cannot write data to file"));
            tmp = 0;
            width.unstack(tmp);
        }
        while(tmp > 0);

            // now we write the last byte of the preambule, which as only one bit set

        if(x.write((char *)&last_width, 1) < 1)
            throw Erange("infinint::dump(generic_file)", gettext("Cannot write data to file"));

            // we need now to write some justification byte to have an informational field multiple of TG

        if(justification != 0)
        {
            U_16 tmp = 0;
            justification.unstack(tmp);
            tmp = TG - tmp;
            while(tmp-- > 0)
                if(x.write((char *)(&u), 1) < 1)
                    throw Erange("infinint::dump(generic_file)", gettext("Cannot write data to file"));
        }

            // now we continue dumping the informational bytes :
        field->dump(x);

        E_END("infinint::dump", "generic_file");
    }

    infinint & infinint::operator += (const infinint & arg)
    {
        E_BEGIN;
        if(! is_valid() || ! arg.is_valid())
            throw SRC_BUG;

            // enlarge field to be able to receive the result of the operation
        make_at_least_as_wider_as(arg);

            // now processing the operation

        storage::iterator it_a = arg.field->rbegin();
        storage::iterator it_res = field->rbegin();
        U_I retenue = 0, somme;

        while(it_res != field->rend() && (it_a != arg.field->rend() || retenue != 0))
        {
            somme = *it_res;
            if(it_a != arg.field->rend())
	    {
                somme += *it_a;
		--it_a;
	    }
            somme += retenue;

            retenue = somme >> 8;
            somme &= 0xFF;

            *it_res = somme;
	    --it_res;
        }

        if(retenue != 0)
        {
            field->insert_null_bytes_at_iterator(field->begin(), 1);
            (*field)[0] = retenue;
        }

            // reduce(); // not necessary here, as the resulting filed is
            // not smaller than the one of the two infinint in presence

        return *this;
        E_END("infinint::operator +=", "");
    }

    infinint & infinint::operator -= (const infinint & arg)
    {
        E_BEGIN;
        if(! is_valid() || ! arg.is_valid())
            throw SRC_BUG;

        if(*this < arg)
            throw Erange("infinint::operator", gettext("Subtracting a infinint greater than the first, infinint cannot be negative"));

            // now processing the operation

        storage::iterator it_a = arg.field->rbegin();
        storage::iterator it_res = field->rbegin();
        U_I retenue = 0;
        S_I somme;
        U_I tmp;

        while(it_res != field->rend() && (it_a != arg.field->rend() || retenue != 0))
        {
            somme = *it_res;
            if(it_a != arg.field->rend())
	    {
                somme -= *it_a;
		--it_a;
	    }
            somme -= retenue;

            if(somme < 0)
            {
                somme = -somme;

                tmp = somme & 0xFF;
                retenue = somme >> 8;
                if(tmp != 0)
                {
                    somme = 0x100 - tmp;
                    ++retenue;
                }
                else
                    somme = 0;
            }
            else
                retenue = 0;

            *it_res = somme;
	    --it_res;
        }

        reduce(); // it is necessary here !

        return *this;
        E_END("infinint::operator -=", "");
    }

    infinint & infinint::operator *= (unsigned char arg)
    {
        E_BEGIN;
        if(!is_valid())
            throw SRC_BUG;

        storage::iterator it = field->rbegin();
        U_I produit, retenue = 0;

        while(it != field->rend())
        {
            produit = (*it) * arg + retenue;
            retenue = 0;

            retenue = produit >> 8;
            produit = produit & 0xFF;

            *it = produit;
	    --it;
        }

        if(retenue != 0)
        {
            field->insert_null_bytes_at_iterator(field->begin(), 1);
            (*field)[0] = retenue;
        }

        if(arg == 0)
            reduce(); // only necessary in that case

        return *this;
        E_END("infinint::operator *=", "unsigned char");
    }

    infinint & infinint::operator *= (const infinint & arg)
    {
        E_BEGIN;
        infinint ret = 0;

        if(!is_valid() || !arg.is_valid())
            throw SRC_BUG;

        storage::iterator it_t = field->begin();

        while(it_t != field->end())
        {
            ret <<= 8; // shift by one byte;
            ret += arg * (*it_t);
	    ++it_t;
        }

            //  ret.reduce();  // not necessary because all operations
            // done on ret, provide reduced infinint
        *this = ret;

        return *this; // copy constructor
        E_END("infinint::operator *=", "infinint");
    }

    infinint & infinint::operator &= (const infinint & arg)
    {
	E_BEGIN;
	if(! is_valid() || ! arg.is_valid())
	    throw SRC_BUG;

	make_at_least_as_wider_as(arg);

	storage::iterator it_a = arg.field->rbegin();
	storage::iterator it_res = field->rbegin();

	while(it_res != field->rend() && it_a != arg.field->rend())
	    *it_res-- &= *it_a--;
	while(it_res != field->rend())	// set upper bits to zero when arg is smaller than *this
	    *it_res-- = 0;

	reduce();

	return *this;
	E_END("infinint::operator &=", "infinint");
    }

    infinint & infinint::operator |= (const infinint & arg)
    {
	E_BEGIN;
	if(! is_valid() || ! arg.is_valid())
	    throw SRC_BUG;

	make_at_least_as_wider_as(arg);

	storage::iterator it_a = arg.field->rbegin();
	storage::iterator it_res = field->rbegin();

	while(it_res != field->rend() && it_a != arg.field->rend())
	    *it_res-- |= *it_a--;

	reduce();

	return *this;
	E_END("infinint::operator |=", "infinint");
    }

    infinint & infinint::operator ^= (const infinint & arg)
    {
	E_BEGIN;
	if(! is_valid() || ! arg.is_valid())
	    throw SRC_BUG;

	make_at_least_as_wider_as(arg);

	storage::iterator it_a = arg.field->rbegin();
	storage::iterator it_res = field->rbegin();

	while(it_res != field->rend() && it_a != arg.field->rend())
	    *it_res-- ^= *it_a--;

	reduce();

	return *this;
	E_END("infinint::operator ^=", "infinint");
    }


    infinint & infinint::operator >>= (U_32 bit)
    {
        E_BEGIN;
        if(! is_valid())
            throw SRC_BUG;

        U_32 byte = bit/8;
        storage::iterator it = field->rbegin() - byte + 1;
        int_tools_bitfield bf;
        unsigned char mask, r1 = 0, r2 = 0;
        U_I shift_retenue;

        bit = bit % 8;
        shift_retenue = 8 - bit;
        if(byte >= field->size())
            *this = 0;
        else
        {
                // shift right by "byte" bytes
            field->remove_bytes_at_iterator(it, byte);

                // shift right by "bit" bits
            if(bit != 0)
            {
                for(register U_I i = 0; i < 8; ++i)
                    bf[i] = i < shift_retenue ? 0 : 1;
                int_tools_contract_byte(bf, mask);

                it = field->begin();
                while(it != field->end())
                {
                    r1 = *it & mask;
                    r1 <<= shift_retenue;
                    *it >>= bit;
                    *it |= r2;
                    r2 = r1;
                    ++it;
                }
                reduce(); // necessary here
            }
        }

        return *this;
        E_END("infinint::operator >>=", "U_32");
    }

    infinint & infinint::operator >>= (infinint bit)
    {
        E_BEGIN;
        if(! is_valid() || ! bit.is_valid())
            throw SRC_BUG;

        U_32 delta_bit = 0;
        bit.unstack(delta_bit);

        do
        {
            *this >>= delta_bit;
            delta_bit = 0;
            bit.unstack(delta_bit);
        }
        while(delta_bit > 0);

        return *this;
        E_END("infinint::operator >>=", "infinint");
    }

    infinint & infinint::operator <<= (U_32 bit)
    {
        E_BEGIN;
        if(! is_valid())
            throw SRC_BUG;

        U_32 byte = bit/8;
        storage::iterator it = field->end();

        if(*this == 0)
            return *this;

        bit %= 8;     // bit gives now the remaining translation after the "byte" translation

        if(bit != 0)
            ++byte;        // to prevent the MSB to be lost (in "out of space" ;-) )

            // this is the "byte" translation
        field->insert_null_bytes_at_iterator(it, byte);

        if(bit != 0)
        {
	    U_I shift_retenue, r1 = 0, r2 = 0;
	    int_tools_bitfield bf;
	    unsigned char mask;

                // and now the bit translation
		// we have shift left one byte in place of 'bit' bits, so we shift right
		// shift_retenue bits:
            shift_retenue = 8 - bit;
            it = field->begin();

                // the mask for selecting the retenue
            for(register U_I i = 0; i < 8; ++i)
                bf[i] = i < bit ? 0 : 1;
            int_tools_contract_byte(bf, mask);

            while(it != field->end())
            {
                r1 = (*it) & mask;
                r1 <<= bit;
                *it >>= shift_retenue;
                *it |= r2;
                r2 = r1;
                ++it;
            }
            reduce();
        }

        return *this;
        E_END("infinint::operator <<=", "U_32");
    }

    infinint & infinint::operator <<= (infinint bit)
    {
        E_BEGIN;
        U_32 delta_bit = 0;
        bit.unstack(delta_bit);

        do
        {
            *this <<= delta_bit;
            delta_bit = 0;
            bit.unstack(delta_bit);
        }
        while(delta_bit > 0);

        return *this;
        E_END("infinint::operator <<=", "infinint");
    }

    unsigned char infinint::operator [] (const infinint & position) const
    {
	return (*field)[field->size() - (position + 1)];
    }

    S_I infinint::difference(const infinint & b) const
    {
        E_BEGIN;
        storage::iterator ita;
        storage::iterator itb;
        const infinint & a = *this;

        if(! a.is_valid() || ! b.is_valid())
            throw SRC_BUG;

        if(*a.field < *b.field) // field is shorter for this than for ref and object are always "reduced"
            return -1;
        else
            if(*a.field > *b.field)
                return +1;
            else // *a.field == *b.field
            {
                ita = a.field->begin();
                itb = b.field->begin();

                while(ita != a.field->end() && itb != b.field->end() && *ita == *itb)
                {
                    ++ita;
                    ++itb;
                }

                if(ita == a.field->end() && itb == b.field->end())
                    return 0;

                if(itb == b.field->end())
                    return +1; // b can't be greater than a, at most it can be equal to it

                if(ita == a.field->end())
                    return -1;  // because itb != b.field->end();

                return *ita - *itb;
            }
        E_END("infinint::difference", "");
    }


    bool infinint::is_valid() const
    {
        E_BEGIN;
        return field != NULL;
        E_END("infinint::is_valid", "");
    }

    void infinint::reduce()
    {
        E_BEGIN;
        static const U_I max_a_time = ~ (U_I)(0); // this is the argument type of remove_bytes_at_iterator

        U_I count = 0;
        storage::iterator it = field->begin();

        do
        {
            while(it != field->end() && (*it) == 0 && count < max_a_time)
            {
                ++it;
                ++count;
            }

            if(it == field->end()) // all zeros
            {
                if(count == 0) // empty storage;
                    field->insert_null_bytes_at_iterator(field->begin(), 1); // field width is at least one byte
                else
                    if(count > 1)
                        field->remove_bytes_at_iterator(field->begin(), count - 1);

                    // it == field->end()  is still true
            }
            else
            {
                if(count > 0)
                    field->remove_bytes_at_iterator(field->begin(), count);
                count = 0;
                it = field->begin();
            }
        }
        while(it != field->end() && (*it) == 0);
        E_END("infinint::reduce", "");
    }

    void infinint::copy_from(const infinint & ref)
    {
        E_BEGIN;
        if(ref.is_valid())
        {
            field = new storage(*(ref.field));
            if(field == NULL)
                throw Ememory("infinint::copy_from");
        }
        else
            throw SRC_BUG;
        E_END("infinint::copy_from", "");
    }

    void infinint::detruit()
    {
        E_BEGIN;
        if(field != NULL)
        {
            delete field;
            field = NULL;
        }
        E_END("infinint::detruit", "");
    }

    void infinint::make_at_least_as_wider_as(const infinint & ref)
    {
        E_BEGIN;
        if(! is_valid() || ! ref.is_valid())
            throw SRC_BUG;

        field->insert_as_much_as_necessary_const_byte_to_be_as_wider_as(*ref.field, field->begin(), 0x00);
        E_END("infinint::make_at_least_as_wider_as", "");
    }


    static void dummy_call(char *x)
    {
        static char id[]="$Id: real_infinint.cpp,v 1.19.2.2 2007/07/22 16:35:00 edrusb Rel $";
        dummy_call(id);
    }

    void infinint::setup_endian()
    {
        E_BEGIN;
        U_16 u = 1;
        unsigned char *ptr = (unsigned char *)(&u);

        if(ptr[0] == 1)
            used_endian = big_endian;
        else
            used_endian = little_endian;
        E_END("infinint::setup_endian", "");
    }

///////////////////////////////////////////////////////////////////////
///////////////// friends and not friends of infinint /////////////////
///////////////////////////////////////////////////////////////////////

    infinint operator + (const infinint & a, const infinint & b)
    {
        E_BEGIN;
        infinint ret = a;
        ret += b;

        return ret;
        E_END("operator +", "infinint");
    }

    infinint operator - (const infinint & a, const infinint & b)
    {
        E_BEGIN;
        infinint ret = a;
        ret -= b;

        return ret;
        E_END("operator -", "infinint");
    }

    infinint operator * (const infinint & a, const infinint & b)
    {
        E_BEGIN;
        infinint ret = a;
        ret *= b;

        return ret;
        E_END("operator *", "infinint");
    }

    infinint operator * (const infinint &a, const unsigned char b)
    {
        E_BEGIN;
        infinint ret = a;
        ret *= b;

        return ret;
        E_END("operator *", "infinint, unsigned char");
    }

    infinint operator * (const unsigned char a, const infinint &b)
    {
        E_BEGIN;
        infinint ret = b;
        ret *= a;

        return ret;
        E_END("operator *", "unsigned char, infinint");
    }

    infinint operator / (const infinint & a, const infinint & b)
    {
        E_BEGIN;
        infinint q, r;
        euclide(a, b, q, r);

        return q;
        E_END("operator / ", "infinint");
    }

    infinint operator % (const infinint & a, const infinint & b)
    {
        E_BEGIN;
        infinint q, r;
        euclide(a, b, q, r);

        return r;
        E_END("operator %", "infinint");
    }


    infinint operator & (const infinint & a, const infinint & bit)
    {
	E_BEGIN;
	infinint ret = a;
	ret &= bit;
	return ret;
	E_END("operator &", "infinint");
    }

    infinint operator | (const infinint & a, const infinint & bit)
    {
	E_BEGIN;
	infinint ret = a;
	ret |= bit;
	return ret;
	E_END("operator |", "infinint");
    }

    infinint operator ^ (const infinint & a, const infinint & bit)
    {
	E_BEGIN;
	infinint ret = a;
	ret ^= bit;
	return ret;
	E_END("operator ^", "infinint");
    }

    infinint operator >> (const infinint & a, U_32 bit)
    {
        E_BEGIN;
        infinint ret = a;
        ret >>= bit;
        return ret;
        E_END("operator >>", "infinint, U_32");
    }

    infinint operator >> (const infinint & a, const infinint & bit)
    {
        E_BEGIN;
        infinint ret = a;
        ret >>= bit;
        return ret;
        E_END("operator >>", "infinint");
    }

    infinint operator << (const infinint & a, U_32 bit)
    {
        E_BEGIN;
        infinint ret = a;
        ret <<= bit;
        return ret;
        E_END("operator <<", "infinint, U_32");
    }

    infinint operator << (const infinint & a, const infinint & bit)
    {
        E_BEGIN;
        infinint ret = a;
        ret <<= bit;
        return ret;
        E_END("operator <<", "infinint");
    }

    void euclide(infinint a, const infinint &b, infinint &q, infinint &r)
    {
        E_BEGIN;
        if(b == 0)
            throw Einfinint("infinint.cpp : euclide", gettext("Division by zero")); // division by zero

        if(a < b)
        {
            q = 0;
            r = a;
            return;
        }

        r = b;
        while(*a.field >= *r.field)
            r <<= 8; // one byte

        q = 0;
        while(b < r)
        {
            r >>= 8; // one byte;
            q <<= 8; // one byte;
            while(r <= a)
            {
                a -= r;
                ++q;
            }
        }

        r = a;
        E_END("euclide", "infinint");
    }

} // end of namespace

