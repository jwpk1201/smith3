//
// SMITH3 - generates spin-free multireference electron correlation programs.
// Filename: active_gen.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the SMITH3 package.
//
// The SMITH3 package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The SMITH3 package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the SMITH3 package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include "constants.h"
#include "active.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;


// need to do here both what generate_get_block and a generate_sort_indices usually does   
// ok to do this here b/c these blocks need special attention because of deltas
string RDM::generate(string indent, const string tlab, const list<shared_ptr<Index> >& loop) const {
  stringstream tt;
  indent += "  ";
  stringstream ls;
  stringstream rs;
  
  
  // first let's do the generate_get_block for an rdm..
  tt << indent << "std::vector<size_t> i0hash = vec("; 
  int cntr=0;
  for (auto iter = index_.rbegin(); iter != index_.rend(); ++iter,++cntr) {
    if (iter != index_.rbegin()) tt << ", ";
    tt << (*iter)->str_gen() << "->key()";
  }   
  tt << ");" << endl; 
  tt << indent << "std::unique_ptr<double[]> data = rdm" << rank() << "->get_block(i0hash);" << endl;

  // now do the sort 
  vector<string> close;
  string itag = "i";
  string iindent = "  ";
  // start sort loops
  int cntl=0;
  for (auto i = loop.begin(); i != loop.end(); ++i,++cntl) {
    // string itag += string((*iter)->num());  // new label this would simplify things for next line
    tt << indent << "for (int " << itag << (*i)->num() << " =0; " << itag << (*i)->num() << " != " << (*i)->str_gen() <<             "->size(); ++"<< itag << (*i)->num() << ") {" << endl;
    close.push_back(indent + "}");
    indent += iindent;
  }
  // somehow check if we have a delta, if so we need to process and get the indices
  // and when don't have need to make an exception for data
/*
  for (auto d = delta_.begin() ; d != delta_.end(); ++d) {  
    std::cout << (*d)->print() << endl;
  }    
*/

  // make odata part of summation for target
  int cnt=0;
  int cntp=0;
  ls  << "odata[";
  for (auto ri = loop.rbegin(); ri != loop.rend(); ++ri,++cnt) {
    if (cnt != cntl-1 && cnt != cntl-2) {
      ls << itag << (*ri)->num() << "+" << (*ri)->str_gen() << "->size()*(";
      ++cntp;
    } else if (cnt == cntl-2 ){
      ls << itag << (*ri)->num() << "+" << (*ri)->str_gen() << "->size()*";
    } else {
      ls << itag << (*ri)->num();
      for (int p=0; p!= cntp; ++p) ls << ")";
      ls << "]" << endl;
    }
  }
  // make data part of summation
  {
  int cnt=0;
  int cntp=0;
  // mkm not sure why this factor isn't looking like a double, ie I get 1 and not 1.0
  rs << "(" << factor() << ") * data[";
  for (auto riter = index_.rbegin(); riter != index_.rend(); ++riter,++cnt) {
    if (cnt != cntr-1 && cnt!= cntr-2) {
      rs << itag << (*riter)->num() << "+" << (*riter)->str_gen() << "->size()*(";
      ++cntp;
    } else if ( cnt == cntr -2){
      rs << itag << (*riter)->num() << "+" << (*riter)->str_gen() << "->size()*";
    } else {
      rs << itag << (*riter)->num();
      for (int p=0; p!= cntp; ++p) rs << ")";
      rs << "]";
    }
  }
  }
  // add the odata and data summations with prefactor

  tt << indent << ls.str()  << indent <<" += " << rs.str() << ";" << endl;

  // close loops
  for (auto iter = close.rbegin(); iter != close.rend(); ++iter)
        tt << *iter << endl;
  return tt.str();
}



