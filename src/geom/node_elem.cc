// $Id: node_elem.cc,v 1.1 2008/03/20 13:40:27 gdiso Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2007  Benjamin S. Kirk, John W. Peterson
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



// C++ includes

// Local includes
#include "node_elem.h"


// Forward declarations


void NodeElem::connectivity(const unsigned int,
			    const IOPackage,
			    std::vector<unsigned int>&) const
{
  genius_error();
}

#ifdef ENABLE_AMR

const float NodeElem::_embedding_matrix[1][1][1] =
{
  // embedding matrix for child 0
  {
    // 0 
    {1.0}, // 0
  }
};

#endif
