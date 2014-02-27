/*
 * Copyright 2011 Steven Gribble
 *
 *  This file is part of the UW CSE 333 course project sequence
 *  (333proj).
 *
 *  333proj is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  333proj is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with 333proj.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <algorithm>
#include <list>
#include "./QueryProcessor.h"

extern "C" {
  #include "./libhw1/CSE333.h"
}

namespace hw3 {

QueryProcessor::QueryProcessor(list<string> indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::iterator idx_iterator = indexlist_.begin();
  for (HWSize_t i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = new DocTableReader(fir.GetDocTableReader());
    itr_array_[i] = new IndexTableReader(fir.GetIndexTableReader());
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != NULL);
  Verify333(itr_array_ != NULL);
  for (HWSize_t i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);
  vector<QueryProcessor::QueryResult> finalresult;

  // MISSING:
  // final result is vector of files that match query?
 // for (HWSize_t ind = 0; ind < arraylen_; ind++) {
  
  DocIDTableReader *initialSet = itr_array_[0]->LookupWord(query[0]);
  if (initialSet == NULL) {
    return finalresult;
  }
  list<docid_element_header> initialdocs = initialSet->GetDocIDList();

  size_t numwords = query.size();
  for (size_t w = 1; w < numwords; w++) {
    for (HWSize_t ind = 0; ind < arraylen_; ind++) {
      DocIDTableReader *ditr = itr_array_[ind]->LookupWord(query[w]);
      if (ditr == NULL) { //word not found in that index
        continue;
      }
      list<docid_element_header> tempdocs = ditr->GetDocIDList();
      //iterate through initial docs, increment num_pos if its in there
      std::list<docid_element_header>::iterator iter;
      for (iter = initialdocs.begin(); iter != initialdocs.end();) {  
        bool isInTemp = false;
        for (docid_element_header temp : tempdocs) {
          if (temp.docid == iter->docid) {
            isInTemp = true;
            iter->num_positions += temp.num_positions;
            break;
          }
        }
        if(!isInTemp) {
          //delete that docid_element_header
          initialdocs.erase(iter);
        } else {
          iter++;
        }
      }
    }
  }
  // now initialdocs is list of docid_element_header{docid, num_positions}
  // of docs with all query words
  std::list<docid_element_header>::iterator finaliter;
  for (finaliter = initialdocs.begin(); finaliter != initialdocs.end(); finaliter++) {
    HWSize_t currank = finaliter->num_positions;
    // get doc name
    // make query result with rank and docname
    //QueryResult qr = new QueryResult{

  }

  // Sort the final results.
  std::sort(finalresult.begin(), finalresult.end());
  return finalresult;
}

}  // namespace hw3
