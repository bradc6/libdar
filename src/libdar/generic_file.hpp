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

    /// \file generic_file.hpp
    /// \brief class generic_file is defined here as well as class fichier
    /// \ingroup Private
    ///
    /// the generic_file interface is widely used in libdar
    /// it defines the standard way of transmitting data between different
    /// part of the library
    /// - compression engine
    /// - encryption engine
    /// - exchange through pipes
    /// - slicing
    /// - dry run operations
    /// - file access
    /// .


///////////////////////////////////////////////////////////////////////
// IMPORTANT : THIS FILE MUST ALWAYS BE INCLUDE AFTER infinint.hpp   //
//             (and infinint.hpp must be included too, always)       //
///////////////////////////////////////////////////////////////////////
#include "infinint.hpp"
///////////////////////////////////////////////////////////////////////



#ifndef GENERIC_FILE_HPP
#define GENERIC_FILE_HPP


#include "../my_config.h"

extern "C"
{
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
} // end extern "C"

#include "path.hpp"
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

	/// generic_file openning modes
    enum gf_mode
    {
	gf_read_only,  ///< read only access
	gf_write_only, ///< write only access
	gf_read_write  ///< read and write access
    };


    extern gf_mode generic_file_get_mode(S_I fd);
    extern const char * generic_file_get_name(gf_mode mode);

	/// this is the interface class from which all other data transfer classes inherit

	/// it provides mainly read and write operations,
	/// skip operations and few other functions.
	/// \note
	/// the read and write method are similar to the read and write system calls
	/// except that they never return negative values, but throw exception instead.
	/// returning zero means end of generic_file. The call is blocked if
	/// no data is available for reading.
	/// write returns the number of bytes written, and never make partial writtings.
	/// Thus, it is blocked until all bytes are written or occures an exception
	/// inconsequences the returned value is always the value of the argument
	/// "size".
    class generic_file
    {
    public :
	    /// main constructor
        generic_file(gf_mode m) { rw = m; terminated = false; enable_crc(false); checksum = NULL; };

	    /// copy constructor
	generic_file(const generic_file &ref) { copy_from(ref); };


	    /// virtual destructor, this let inherited destructor to be called even from a generic_file pointer to an inherited class

	    /// destructor-like call, except that it is allowed to throw exceptions
	void terminate() const;

	virtual ~generic_file() { destroy(); };

	    /// assignment operator
	const generic_file & operator = (const generic_file & ref) { destroy(); copy_from(ref); return *this; };

	    /// retreive the openning mode for this object
        gf_mode get_mode() const { return rw; };

	    /// read data from the generic_file

	    /// \param[in, out] a is where to put the data to read
	    /// \param[in] size is how much data to read
	    /// \return the exact number of byte read.
	    /// \note read as much as requested data, unless EOF is met (only EOF can lead to reading less than requested data)
	    /// \note EOF is met if read() returns less than size
        U_I read(char *a, U_I size);

	    /// write data to the generic_file

	    /// \note throws a exception if not all data could be written as expected
        void write(const char *a, U_I size);

	    /// write a string to the generic_file

	    /// \note throws a exception if not all data could be written as expected
        void write(const std::string & arg);

	    /// skip back one char, read on char and skip back one char
        S_I read_back(char &a);

	    /// read one char
	S_I read_forward(char &a) { if(terminated) throw SRC_BUG; return read(&a, 1); };

	    /// skip at the absolute position

	    /// \param[in] pos the offset in byte where next read/write operation must start
	    /// \return true if operation was successfull and false if the requested position is not valid (after end of file)
	    /// \note if requested position is not valid the reading/writing cursor must be set to the closest valid position
        virtual bool skip(const infinint & pos) = 0;

	    /// skip to the end of file
        virtual bool skip_to_eof() = 0;

	    /// skip relatively to the current position
        virtual bool skip_relative(S_I x) = 0;

	    /// get the current read/write position
        virtual infinint get_position() = 0;

	    /// copy all data from current position to the object in argument
        virtual void copy_to(generic_file &ref);

	    /// copy all data from the current position to the object in argument and computes a CRC value of the transmitted data

	    /// \param[in] ref defines where to copy the data to
	    /// \param[in] crc_size tell the width of the crc to compute on the copied data
	    /// \param[out] value points to a newly allocated crc object containing the crc value
	    /// \note value has to be deleted by the caller when no more needed
        virtual void copy_to(generic_file &ref, const infinint & crc_size, crc * & value);

	    /// small copy (up to 4GB) with CRC calculation
        U_32 copy_to(generic_file &ref, U_32 size); // returns the number of byte effectively copied

	    /// copy the given amount to the object in argument
        infinint copy_to(generic_file &ref, infinint size); // returns the number of byte effectively copied

	    /// compares the contents with the object in argument

	    /// \param[in] f is the file to compare the current object with
	    /// \param[in] crc_size is the width of the CRC to use for calculation
	    /// \param[out] value is the computed checksum, its value can be used for additional
	    /// testing if this method returns false (no difference between files). The given checksum
	    /// has to be set to the expected width by the caller.
	    /// \return true if arg differ from "this"
	    /// \note value has to be deleted by the caller when no more needed
        bool diff(generic_file & f, const infinint & crc_size, crc * & value);

	    /// compare the contents with the object in argument, also providing the offset of the first difference met
	    /// \param[in] f is the file to compare the current object with
	    /// \param[in] crc_size is the width of the CRC to use for calculation
	    /// \param[out] value is the computed checksum, its value can be used for additional
	    /// \param[out] err_offset in case of difference, holds the offset of the first difference met
	    /// testing if this method returns false (no difference between files). The given checksum
	    /// has to be set to the expected width by the caller.
	    /// \return true if arg differ from "this", else false is returned and err_offset is set
	    /// \note value has to be deleted by the caller when no more needed
	bool diff(generic_file & f, const infinint & crc_size, crc * & value, infinint & err_offset);

            /// reset CRC on read or writen data

	    /// \param[in] width is the width to use for the CRC
        void reset_crc(const infinint & width);

	    /// to known whether CRC calculation is activated or not
	bool crc_status() const { return active_read == &generic_file::read_crc; };

	    /// get CRC of the transfered date since last reset

	    /// \return a newly allocated crc object, that the caller has the responsibility to delete
	    /// \note does also ends checksum calculation, which if needed again
	    /// have to be re-enabled calling reset_crc() method
        crc *get_crc();

	    /// write any pending data
	void sync_write();

    protected :
        void set_mode(gf_mode x) { rw = x; };

	    /// implementation of read() operation

	    /// \param[in,out] a where to put the data to read
	    /// \param[in] size says how much data to read
	    /// \return the exact amount of data read and put into 'a'
	    /// \note read as much byte as requested, up to end of file
	    /// stays blocked if not enough data is available and EOF not
	    /// yet met. May return less data than requested only if EOF as been reached.
	    /// in other worlds, EOF is reached when returned data is stricly less than the requested data
	    /// Any problem shall be reported by throwing an exception.
        virtual U_I inherited_read(char *a, U_I size) = 0;

	    /// implementation of the write() operation

	    /// \param[in] a what data to write
	    /// \param[in] size amount of data to write
	    /// \note must either write all data or report an error by throwing an exception
        virtual void inherited_write(const char *a, U_I size) = 0;


	    /// write down any pending data

	    /// \note this method is called after read/write mode
	    /// checking from sync_write() public method;
	virtual void inherited_sync_write() = 0;


	    /// destructor-like call, except that it is allowed to throw exceptions

	    /// \note this method must never be called directly but using terminate() instead,
	    /// generic_file class manages it to never be called more than once
	virtual void inherited_terminate() = 0;


	    /// is some specific call (skip() & Co.) need to be forbidden when the object
	    /// has been terminated, one can use this call to check the terminated status
	bool is_terminated() const { return terminated; };

    private :
        gf_mode rw;
        crc *checksum;
	bool terminated;
        U_I (generic_file::* active_read)(char *a, U_I size);
        void (generic_file::* active_write)(const char *a, U_I size);

        void enable_crc(bool mode);

        U_I read_crc(char *a, U_I size);
        void write_crc(const char *a, U_I size);
	void destroy();
	void copy_from(const generic_file & ref);
    };

#define CONTEXT_INIT "init"
#define CONTEXT_OP   "operation"
#define CONTEXT_LAST_SLICE  "last_slice"

	/// the contextual class adds the information of phases in the generic_file

	/// several phases are defined like for example
	/// - INIT phase
	/// - OPERATIONAL phase
	/// - LAST SLICE phase
	/// .
	/// these are used to help the command launched between slices to
	/// decide the action to do depending on the context when reading an archive
	/// (first slice / last slice read, ...)
	/// the context must also be transfered to dar_slave through the pair of tuyau objects
	///
	/// this class also support some additional informations common to all 'level1' layer of
	/// archive, such as:
	/// - the data_name information
	///

    class label;

    class contextual
    {
    public :
	contextual() { status = ""; };
	virtual ~contextual() {};

	virtual void set_info_status(const std::string & s) { status = s; };
        virtual std::string get_info_status() const { return status; };
	virtual bool is_an_old_start_end_archive() const = 0;

	virtual const label & get_data_name() const = 0;

    private:
	std::string status;
    };

	/// @}

} // end of namespace

#endif
