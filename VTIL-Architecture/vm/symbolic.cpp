// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of mosquitto nor the names of its   
//    contributors may be used to endorse or promote products derived from   
//    this software without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#include "symbolic.hpp"

namespace vtil
{
	// Dumps the current state of the virtual machine.
	//
	void symbolic_vm::dump_state() const
	{
		using namespace vtil::logger;
		for ( auto& [reg, exp] : register_state )
		{
			log<CON_BLU>( "%-16s     :=", reg.to_string() );
			log<CON_GRN>( "%s\n", exp.to_string() );
		}

		for ( auto& [ptr, exp] : memory_state )
		{
			log<CON_YLW>( "[%-16s]\n", ptr.to_string() );
			log<CON_GRN>( "%s\n", exp.to_string() );
		}
	}

	// Reads from the register.
	//
	symbolic::expression symbolic_vm::read_register( const register_desc& desc )
	{
		register_desc full = { desc.flags, desc.local_id, 64, 0 };

		auto it = register_state.find( full );
		if ( it == register_state.end() )
			return symbolic::make_register_ex( desc );
		else
			return ( it->second >> desc.bit_offset ).resize( desc.bit_count );
	}

	// Writes to the register.
	//
	void symbolic_vm::write_register( const register_desc& desc, symbolic::expression value )
	{
		if ( desc.bit_count == 64 && desc.bit_offset == 0 )
		{
			register_state.erase( desc );
			register_state.emplace( desc, std::move( value ) );
		}
		else
		{
			register_desc full = { desc.flags, desc.local_id, 64, 0 };
			auto& exp = register_state[ full ];
			if ( !exp ) exp = { full, 64 };
			exp = ( exp & ~desc.get_mask() ) | ( value.resize( desc.bit_count ).resize( 64 ) << desc.bit_offset );
		}
	}

	// Reads the given number of bytes from the memory.
	//
	symbolic::expression symbolic_vm::read_memory( const symbolic::expression& pointer, size_t byte_count )
	{
		bitcnt_t bcnt = byte_count * 8;
		return memory_state.read( pointer, bcnt ).value_or( { symbolic::make_undefined_ex( bcnt ) } );
	}

	// Writes the given expression to the memory.
	//
	void symbolic_vm::write_memory( const symbolic::expression& pointer, symbolic::expression value )
	{
		memory_state.write( pointer, value.resize( ( value.size() + 7 ) & ~7 ) );
	}
};