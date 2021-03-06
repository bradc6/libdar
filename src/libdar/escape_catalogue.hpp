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

    /// \file escape_catalogue.hpp
    /// \brief class escape_catalogue definition. Used for sequential writing to archives, as well as several other inherited classes from catalogue.hpp
    /// \ingroup Private
    ///
    /// This class inherits from the class catalogue and implements
    /// the pre_add(...) method, which role is to add an escape sequence followed
    /// by an entry dump (usually used at the end of archive is the so called catalogue part
    /// of the archive. This sequence followed by entry dump is added
    /// before each file's data all along the archive.
    /// Other inherited classes, implement the escape specific part, used when performing
    /// sequential reading of the catalogue

#ifndef ESCAPE_CATALOGUE_HPP
#define ESCAPE_CATALOGUE_HPP

#include "../my_config.h"

#include "catalogue.hpp"
#include "escape.hpp"

namespace libdar
{

	/// \addtogroup Private
	/// @{

    class escape_catalogue : public catalogue
    {
    public:
        escape_catalogue(user_interaction & dialog,
			 const infinint & root_last_modif,
			 const label & data_name, escape *esc_ptr);
        escape_catalogue(user_interaction & dialog,        //< user interaction
			 const archive_version & reading_ver,  //< archive format
			 compression default_algo,         //< default compression algorithm
			 generic_file *data_loc,           //< at which layer to read data from
			 generic_file *ea_loc,             //< at which layer to read EA from
			 escape *esc_ptr,                  //< the escape layer of the stack
			 bool lax = false);                //< whether to use lax mode
        escape_catalogue(const escape_catalogue & ref) : catalogue(ref) { copy_from(ref); };
        const escape_catalogue & operator = (const escape_catalogue &ref);
	~escape_catalogue() { destroy(); };

	    // inherited from catalogue
	void pre_add(const entree *ref, compressor *compr) const;
	void pre_add_ea(const entree *ref, compressor *compr) const;
	void pre_add_crc(const entree *ref, compressor *compr) const;
	void pre_add_dirty(compressor *compr) const;
	void pre_add_ea_crc(const entree *ref, compressor *compr) const;
	void pre_add_waste_mark(compressor *compr) const;
	void pre_add_failed_mark(compressor *compr) const;
	escape *get_escape_layer() const { return esc; };

	void reset_read() const;
	void end_read() const;
	void skip_read_to_parent_dir() const;
	bool read(const entree * & ref) const;
	bool read_if_present(std::string *name, const nomme * & ref) const;
	void tail_catalogue_to_current_read();
	bool read_second_time_dir() const { return status == ec_detruits; };

    private:
	enum state
	{
	    ec_init,   //< state in which no one file has yet been searched in the archive
	    ec_marks,  //< state in which we find the next file using escape sequence marks
	    ec_eod,    //< state in which the archive is missing trailing EOD entries, due to user interruption, thus returning EOD in enough number to get back to the root directory
	    ec_detruits,  //< state in which which detruits objects are returned from the catalogue
	    ec_completed  //< state in which the escape_catalogue object is completed and has all information in memory as a normal catalogue
	};

	escape *esc;
	archive_version x_reading_ver;
	compression x_default_algo;
	generic_file *x_data_loc;
	generic_file *x_ea_loc;
	bool x_lax;
	std::map <infinint, etoile *> corres;
        state status;
	catalogue *cat_det; //< holds the final catalogue's detruit objects when no more file can be read from the archive
	infinint min_read_offset;   //< next offset in archive should be greater than that to identify a mark
	infinint depth;             //< directory depth of archive being read sequentially
	infinint wait_parent_depth; //< ignore any further entry while depth is less than wait_parent_depth. disabled is set to zero

	void set_esc(escape *esc_ptr) { if(esc_ptr != NULL) esc = esc_ptr; else throw SRC_BUG; };
	void copy_from(const escape_catalogue & ref);
	void destroy();
	void merge_cat_det();
	void reset_reading_process();
    };

	/// @}

} // end of namespace

#endif
